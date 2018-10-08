using namespace std;

#include "TopScoreDocCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

void TopScoreDocCollector::ScorerLeafCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
}

TopScoreDocCollector::SimpleTopScoreDocCollector::SimpleTopScoreDocCollector(
    int numHits)
    : TopScoreDocCollector(numHits)
{
}

shared_ptr<LeafCollector>
TopScoreDocCollector::SimpleTopScoreDocCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr int docBase = context->docBase;
  return make_shared<ScorerLeafCollectorAnonymousInnerClass>(shared_from_this(),
                                                             docBase);
}

TopScoreDocCollector::SimpleTopScoreDocCollector::
    ScorerLeafCollectorAnonymousInnerClass::
        ScorerLeafCollectorAnonymousInnerClass(
            shared_ptr<SimpleTopScoreDocCollector> outerInstance, int docBase)
{
  this->outerInstance = outerInstance;
  this->docBase = docBase;
}

void TopScoreDocCollector::SimpleTopScoreDocCollector::
    ScorerLeafCollectorAnonymousInnerClass::collect(int doc) 
{
  float score = scorer::score();

  // This collector cannot handle these scores:
  assert((score != -std, : numeric_limits<float>::infinity()));
  assert(!isnan(score));

  outerInstance->totalHits++;
  if (score <= outerInstance->pqTop->score) {
    // Since docs are returned in-order (i.e., increasing doc Id), a document
    // with equal score to pqTop.score cannot compete since HitQueue favors
    // documents with lower doc Ids. Therefore reject those docs too.
    return;
  }
  outerInstance->pqTop->doc = doc + docBase;
  outerInstance->pqTop->score = score;
  outerInstance->pqTop = outerInstance->pq->updateTop();
}

TopScoreDocCollector::PagingTopScoreDocCollector::PagingTopScoreDocCollector(
    int numHits, shared_ptr<ScoreDoc> after)
    : TopScoreDocCollector(numHits), after(after)
{
  this->collectedHits = 0;
}

int TopScoreDocCollector::PagingTopScoreDocCollector::topDocsSize()
{
  return collectedHits < pq->size() ? collectedHits : pq->size();
}

shared_ptr<TopDocs>
TopScoreDocCollector::PagingTopScoreDocCollector::newTopDocs(
    std::deque<std::shared_ptr<ScoreDoc>> &results, int start)
{
  return results.empty()
             ? make_shared<TopDocs>(
                   totalHits, std::deque<std::shared_ptr<ScoreDoc>>(0), NAN)
             : make_shared<TopDocs>(totalHits, results);
}

shared_ptr<LeafCollector>
TopScoreDocCollector::PagingTopScoreDocCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr int docBase = context->docBase;
  constexpr int afterDoc = after->doc - context->docBase;
  return make_shared<ScorerLeafCollectorAnonymousInnerClass>(shared_from_this(),
                                                             docBase, afterDoc);
}

TopScoreDocCollector::PagingTopScoreDocCollector::
    ScorerLeafCollectorAnonymousInnerClass::
        ScorerLeafCollectorAnonymousInnerClass(
            shared_ptr<PagingTopScoreDocCollector> outerInstance, int docBase,
            int afterDoc)
{
  this->outerInstance = outerInstance;
  this->docBase = docBase;
  this->afterDoc = afterDoc;
}

void TopScoreDocCollector::PagingTopScoreDocCollector::
    ScorerLeafCollectorAnonymousInnerClass::collect(int doc) 
{
  float score = scorer::score();

  // This collector cannot handle these scores:
  assert((score != -std, : numeric_limits<float>::infinity()));
  assert(!isnan(score));

  outerInstance->totalHits++;

  if (score > outerInstance->after->score ||
      (score == outerInstance->after->score && doc <= afterDoc)) {
    // hit was collected on a previous page
    return;
  }

  if (score <= outerInstance->pqTop->score) {
    // Since docs are returned in-order (i.e., increasing doc Id), a document
    // with equal score to pqTop.score cannot compete since HitQueue favors
    // documents with lower doc Ids. Therefore reject those docs too.
    return;
  }
  outerInstance->collectedHits++;
  outerInstance->pqTop->doc = doc + docBase;
  outerInstance->pqTop->score = score;
  outerInstance->pqTop = outerInstance->pq->updateTop();
}

shared_ptr<TopScoreDocCollector> TopScoreDocCollector::create(int numHits)
{
  return create(numHits, nullptr);
}

shared_ptr<TopScoreDocCollector>
TopScoreDocCollector::create(int numHits, shared_ptr<ScoreDoc> after)
{

  if (numHits <= 0) {
    throw invalid_argument(
        L"numHits must be > 0; please use TotalHitCountCollector if you just "
        L"need the total hit count");
  }

  if (after == nullptr) {
    return make_shared<SimpleTopScoreDocCollector>(numHits);
  } else {
    return make_shared<PagingTopScoreDocCollector>(numHits, after);
  }
}

TopScoreDocCollector::TopScoreDocCollector(int numHits)
    : TopDocsCollector<ScoreDoc>(new HitQueue(numHits, true))
{
  // HitQueue implements getSentinelObject to return a ScoreDoc, so we know
  // that at this point top() is already initialized.
  pqTop = pq->top();
}

shared_ptr<TopDocs> TopScoreDocCollector::newTopDocs(
    std::deque<std::shared_ptr<ScoreDoc>> &results, int start)
{
  if (results.empty()) {
    return EMPTY_TOPDOCS;
  }

  // We need to compute maxScore in order to set it in TopDocs. If start == 0,
  // it means the largest element is already in results, use its score as
  // maxScore. Otherwise pop everything else, until the largest element is
  // extracted and use its score as maxScore.
  float maxScore = NAN;
  if (start == 0) {
    maxScore = results[0]->score;
  } else {
    for (int i = pq->size(); i > 1; i--) {
      pq->pop();
    }
    maxScore = pq->pop()->score;
  }

  return make_shared<TopDocs>(totalHits, results, maxScore);
}

bool TopScoreDocCollector::needsScores() { return true; }
} // namespace org::apache::lucene::search