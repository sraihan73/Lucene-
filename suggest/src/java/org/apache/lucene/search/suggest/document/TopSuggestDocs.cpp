using namespace std;

#include "TopSuggestDocs.h"

namespace org::apache::lucene::search::suggest::document
{
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using Lookup = org::apache::lucene::search::suggest::Lookup;
const shared_ptr<TopSuggestDocs> TopSuggestDocs::EMPTY =
    make_shared<TopSuggestDocs>(
        0, std::deque<std::shared_ptr<SuggestScoreDoc>>(0), 0);

TopSuggestDocs::SuggestScoreDoc::SuggestScoreDoc(
    int doc, shared_ptr<std::wstring> key, shared_ptr<std::wstring> context,
    float score)
    : org::apache::lucene::search::ScoreDoc(doc, score), key(key),
      context(context)
{
}

int TopSuggestDocs::SuggestScoreDoc::compareTo(shared_ptr<SuggestScoreDoc> o)
{
  return Lookup::CHARSEQUENCE_COMPARATOR->compare(key, o->key);
}

bool TopSuggestDocs::SuggestScoreDoc::equals(any other)
{
  if (std::dynamic_pointer_cast<SuggestScoreDoc>(other) != nullptr == false) {
    return false;
  } else {
    return key->equals((any_cast<std::shared_ptr<SuggestScoreDoc>>(other)).key);
  }
}

int TopSuggestDocs::SuggestScoreDoc::hashCode() { return key->hashCode(); }

wstring TopSuggestDocs::SuggestScoreDoc::toString()
{
  return L"key=" + key + L" doc=" + to_wstring(doc) + L" score=" +
         to_wstring(score) + L" shardIndex=" + to_wstring(shardIndex);
}

TopSuggestDocs::TopSuggestDocs(
    int totalHits, std::deque<std::shared_ptr<SuggestScoreDoc>> &scoreDocs,
    float maxScore)
    : org::apache::lucene::search::TopDocs(totalHits, scoreDocs, maxScore)
{
}

std::deque<std::shared_ptr<SuggestScoreDoc>> TopSuggestDocs::scoreLookupDocs()
{
  return static_cast<std::deque<std::shared_ptr<SuggestScoreDoc>>>(scoreDocs);
}

shared_ptr<TopSuggestDocs>
TopSuggestDocs::merge(int topN,
                      std::deque<std::shared_ptr<TopSuggestDocs>> &shardHits)
{
  shared_ptr<SuggestScoreDocPriorityQueue> priorityQueue =
      make_shared<SuggestScoreDocPriorityQueue>(topN);
  for (auto shardHit : shardHits) {
    for (auto scoreDoc : shardHit->scoreLookupDocs()) {
      if (scoreDoc == priorityQueue->insertWithOverflow(scoreDoc)) {
        break;
      }
    }
  }
  std::deque<std::shared_ptr<SuggestScoreDoc>> topNResults =
      priorityQueue->getResults();
  if (topNResults.size() > 0) {
    return make_shared<TopSuggestDocs>(topNResults.size(), topNResults,
                                       topNResults[0]->score);
  } else {
    return TopSuggestDocs::EMPTY;
  }
}
} // namespace org::apache::lucene::search::suggest::document