using namespace std;

#include "TopSuggestDocsCollector.h"

namespace org::apache::lucene::search::suggest::document
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using CollectionTerminatedException =
    org::apache::lucene::search::CollectionTerminatedException;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
//    import static
//    org.apache.lucene.search.suggest.document.TopSuggestDocs.SuggestScoreDoc;

TopSuggestDocsCollector::TopSuggestDocsCollector(int num, bool skipDuplicates)
    : priorityQueue(make_shared<SuggestScoreDocPriorityQueue>(num)), num(num)
{
  if (num <= 0) {
    throw invalid_argument(L"'num' must be > 0");
  }
  if (skipDuplicates) {
    seenSurfaceForms = make_shared<CharArraySet>(num, false);
    pendingResults = deque<>();
  } else {
    seenSurfaceForms.reset();
    pendingResults.clear();
  }
}

bool TopSuggestDocsCollector::doSkipDuplicates()
{
  return seenSurfaceForms != nullptr;
}

int TopSuggestDocsCollector::getCountToCollect() { return num; }

void TopSuggestDocsCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
  if (seenSurfaceForms != nullptr) {
    seenSurfaceForms->clear();
    // NOTE: this also clears the priorityQueue:
    for (auto hit : priorityQueue->getResults()) {
      pendingResults.push_back(hit);
    }
  }
}

void TopSuggestDocsCollector::collect(int docID, shared_ptr<std::wstring> key,
                                      shared_ptr<std::wstring> context,
                                      float score) 
{
  shared_ptr<SuggestScoreDoc> current =
      make_shared<SuggestScoreDoc>(docBase + docID, key, context, score);
  if (current == priorityQueue->insertWithOverflow(current)) {
    // if the current SuggestScoreDoc has overflown from pq,
    // we can assume all of the successive collections from
    // this leaf will be overflown as well
    // TODO: reuse the overflow instance?
    throw make_shared<CollectionTerminatedException>();
  }
}

shared_ptr<TopSuggestDocs> TopSuggestDocsCollector::get() 
{

  std::deque<std::shared_ptr<SuggestScoreDoc>> suggestScoreDocs;

  if (seenSurfaceForms != nullptr) {
    // NOTE: this also clears the priorityQueue:
    for (auto hit : priorityQueue->getResults()) {
      pendingResults.push_back(hit);
    }

    // Deduplicate all hits: we already dedup'd efficiently within each segment
    // by truncating the FST top paths search, but across segments there may
    // still be dups:
    seenSurfaceForms->clear();

    // TODO: we could use a priority queue here to make cost O(N * log(num))
    // instead of O(N * log(N)), where N = O(num * numSegments), but typically
    // numSegments is smallish and num is smallish so this won't matter much in
    // practice:

    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(pendingResults, new
    // java.util.Comparator<SuggestScoreDoc>()
    sort(pendingResults.begin(), pendingResults.end(),
         make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

    deque<std::shared_ptr<SuggestScoreDoc>> hits =
        deque<std::shared_ptr<SuggestScoreDoc>>();

    for (auto hit : pendingResults) {
      if (seenSurfaceForms->contains(hit->key) == false) {
        seenSurfaceForms->add(hit->key);
        hits.push_back(hit);
        if (hits.size() == num) {
          break;
        }
      }
    }
    suggestScoreDocs =
        hits.toArray(std::deque<std::shared_ptr<SuggestScoreDoc>>(0));
  } else {
    suggestScoreDocs = priorityQueue->getResults();
  }

  if (suggestScoreDocs.size() > 0) {
    return make_shared<TopSuggestDocs>(
        suggestScoreDocs.size(), suggestScoreDocs, suggestScoreDocs[0]->score);
  } else {
    return TopSuggestDocs::EMPTY;
  }
}

TopSuggestDocsCollector::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<TopSuggestDocsCollector> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TopSuggestDocsCollector::ComparatorAnonymousInnerClass::compare(
    shared_ptr<SuggestScoreDoc> a, shared_ptr<SuggestScoreDoc> b)
{
  // sort by higher score
  int cmp = Float::compare(b->score, a->score);
  if (cmp == 0) {
    // tie break by lower docID:
    cmp = Integer::compare(a->doc, b->doc);
  }
  return cmp;
}

void TopSuggestDocsCollector::collect(int doc) 
{
  // {@link #collect(int, std::wstring, std::wstring, long)} is used
  // instead
}

bool TopSuggestDocsCollector::needsScores() { return true; }
} // namespace org::apache::lucene::search::suggest::document