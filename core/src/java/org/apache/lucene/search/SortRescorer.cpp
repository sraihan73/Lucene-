using namespace std;

#include "SortRescorer.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

SortRescorer::SortRescorer(shared_ptr<Sort> sort) : sort(sort) {}

shared_ptr<TopDocs> SortRescorer::rescore(shared_ptr<IndexSearcher> searcher,
                                          shared_ptr<TopDocs> firstPassTopDocs,
                                          int topN) 
{

  // Copy ScoreDoc[] and sort by ascending docID:
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      firstPassTopDocs->scoreDocs.clone();
  Arrays::sort(hits,
               make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  deque<std::shared_ptr<LeafReaderContext>> leaves =
      searcher->getIndexReader()->leaves();

  shared_ptr<TopFieldCollector> collector =
      TopFieldCollector::create(sort, topN, true, true, true, true);

  // Now merge sort docIDs from hits, with reader's leaves:
  int hitUpto = 0;
  int readerUpto = -1;
  int endDoc = 0;
  int docBase = 0;

  shared_ptr<LeafCollector> leafCollector = nullptr;
  shared_ptr<FakeScorer> fakeScorer = make_shared<FakeScorer>();

  while (hitUpto < hits.size()) {
    shared_ptr<ScoreDoc> hit = hits[hitUpto];
    int docID = hit->doc;
    shared_ptr<LeafReaderContext> readerContext = nullptr;
    while (docID >= endDoc) {
      readerUpto++;
      readerContext = leaves[readerUpto];
      endDoc = readerContext->docBase + readerContext->reader()->maxDoc();
    }

    if (readerContext != nullptr) {
      // We advanced to another segment:
      leafCollector = collector->getLeafCollector(readerContext);
      leafCollector->setScorer(fakeScorer);
      docBase = readerContext->docBase;
    }

    fakeScorer->score_ = hit->score;
    fakeScorer->doc = docID - docBase;

    leafCollector->collect(fakeScorer->doc);

    hitUpto++;
  }

  return collector->topDocs();
}

SortRescorer::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<SortRescorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int SortRescorer::ComparatorAnonymousInnerClass::compare(shared_ptr<ScoreDoc> a,
                                                         shared_ptr<ScoreDoc> b)
{
  return a->doc - b->doc;
}

shared_ptr<Explanation>
SortRescorer::explain(shared_ptr<IndexSearcher> searcher,
                      shared_ptr<Explanation> firstPassExplanation,
                      int docID) 
{
  shared_ptr<TopDocs> oneHit = make_shared<TopDocs>(
      1, std::deque<std::shared_ptr<ScoreDoc>>{
             make_shared<ScoreDoc>(docID, firstPassExplanation->getValue())});
  shared_ptr<TopDocs> hits = rescore(searcher, oneHit, 1);
  assert(hits->totalHits == 1);

  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();

  // Add first pass:
  shared_ptr<Explanation> first =
      Explanation::match(firstPassExplanation->getValue(), L"first pass score",
                         firstPassExplanation);
  subs.push_back(first);

  shared_ptr<FieldDoc> fieldDoc =
      std::static_pointer_cast<FieldDoc>(hits->scoreDocs[0]);

  // Add sort values:
  std::deque<std::shared_ptr<SortField>> sortFields = sort->getSort();
  for (int i = 0; i < sortFields.size(); i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    subs.push_back(
        Explanation::match(0.0f, L"sort field " + sortFields[i]->toString() +
                                     L" value=" + fieldDoc->fields[i]));
  }

  // TODO: if we could ask the Sort to explain itself then
  // we wouldn't need the separate ExpressionRescorer...
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Explanation::match(
      0.0f, L"sort field values for sort=" + sort->toString(), subs);
}
} // namespace org::apache::lucene::search