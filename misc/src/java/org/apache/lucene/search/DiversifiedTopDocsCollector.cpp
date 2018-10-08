using namespace std;

#include "DiversifiedTopDocsCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using ScoreDocKey = org::apache::lucene::search::ScoreDocKey;
using org::apache::lucene::util::PriorityQueue;

DiversifiedTopDocsCollector::DiversifiedTopDocsCollector(int numHits,
                                                         int maxHitsPerKey)
    : TopDocsCollector<org::apache::lucene::search::ScoreDocKey>(
          make_shared<ScoreDocKeyQueue>(numHits))
{
  // Need to access pq.lessThan() which is protected so have to cast here...
  this->globalQueue = std::static_pointer_cast<ScoreDocKeyQueue>(pq);
  perKeyQueues = unordered_map<int64_t, std::shared_ptr<ScoreDocKeyQueue>>();
  this->numHits = numHits;
  this->maxNumPerKey = maxHitsPerKey;
}

bool DiversifiedTopDocsCollector::needsScores() { return true; }

shared_ptr<TopDocs> DiversifiedTopDocsCollector::newTopDocs(
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
    for (int i = globalQueue->size(); i > 1; i--) {
      globalQueue->pop();
    }
    maxScore = globalQueue->pop()->score;
  }

  return make_shared<TopDocs>(totalHits, results, maxScore);
}

shared_ptr<ScoreDocKey> DiversifiedTopDocsCollector::insert(
    shared_ptr<ScoreDocKey> addition, int docBase,
    shared_ptr<NumericDocValues> keys) 
{
  if ((globalQueue->size() >= numHits) &&
      (globalQueue->lessThan(addition, globalQueue->top()))) {
    // Queue is full and proposed addition is not a globally
    // competitive score
    return addition;
  }
  // The addition stands a chance of being entered - check the
  // key-specific restrictions.
  // We delay fetching the key until we are certain the score is globally
  // competitive. We need to adjust the ScoreDoc's global doc value to be
  // a leaf reader value when looking up keys
  int leafDocID = addition->doc - docBase;
  int64_t value;
  if (keys->advanceExact(leafDocID)) {
    value = keys->longValue();
  } else {
    value = 0;
  }
  addition->key = value;

  // For this to work the choice of key class needs to implement
  // hashcode and equals.
  shared_ptr<ScoreDocKeyQueue> thisKeyQ = perKeyQueues[addition->key];

  if (thisKeyQ->empty()) {
    if (sparePerKeyQueues.empty()) {
      thisKeyQ = make_shared<ScoreDocKeyQueue>(maxNumPerKey);
    } else {
      thisKeyQ = sparePerKeyQueues.pop();
    }
    perKeyQueues.emplace(addition->key, thisKeyQ);
  }
  shared_ptr<ScoreDocKey> perKeyOverflow =
      thisKeyQ->insertWithOverflow(addition);
  if (perKeyOverflow == addition) {
    // This key group has reached capacity and our proposed addition
    // was not competitive in the group - do not insert into the
    // main PQ or the key will be overly-populated in final results.
    return addition;
  }
  if (perKeyOverflow == nullptr) {
    // This proposed addition is also locally competitive within the
    // key group - make a global entry and return
    shared_ptr<ScoreDocKey> globalOverflow =
        globalQueue->insertWithOverflow(addition);
    perKeyGroupRemove(globalOverflow);
    return globalOverflow;
  }
  // For the given key, we have reached max capacity but the new addition
  // is better than a prior entry that still exists in the global results
  // - request the weaker-scoring entry to be removed from the global
  // queue.
  // C++ TODO: The Java deque 'remove(Object)' method is not converted:
  globalQueue->remove(perKeyOverflow);
  // Add the locally-competitive addition into the globally queue
  globalQueue->push_back(addition);
  return perKeyOverflow;
}

void DiversifiedTopDocsCollector::perKeyGroupRemove(
    shared_ptr<ScoreDocKey> globalOverflow)
{
  if (globalOverflow == nullptr) {
    return;
  }
  shared_ptr<ScoreDocKeyQueue> q = perKeyQueues[globalOverflow->key];
  shared_ptr<ScoreDocKey> perKeyLowest = q->pop();
  // The least globally-competitive item should also always be the least
  // key-local item
  assert(globalOverflow == perKeyLowest);
  if (q->empty()) {
    perKeyQueues.erase(globalOverflow->key);
    sparePerKeyQueues.push(q);
  }
}

shared_ptr<LeafCollector> DiversifiedTopDocsCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr int base = context->docBase;
  shared_ptr<NumericDocValues> *const keySource = getKeys(context);

  return make_shared<LeafCollectorAnonymousInnerClass>(shared_from_this(), base,
                                                       keySource);
}

DiversifiedTopDocsCollector::LeafCollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass(
        shared_ptr<DiversifiedTopDocsCollector> outerInstance, int base,
        shared_ptr<NumericDocValues> keySource)
{
  this->outerInstance = outerInstance;
  this->base = base;
  this->keySource = keySource;
}

void DiversifiedTopDocsCollector::LeafCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
}

void DiversifiedTopDocsCollector::LeafCollectorAnonymousInnerClass::collect(
    int doc) 
{
  float score = scorer::score();

  // This collector cannot handle NaN
  assert(!isnan(score));

  outerInstance->totalHits++;

  doc += base;

  if (outerInstance->spare == nullptr) {
    outerInstance->spare = make_shared<ScoreDocKey>(doc, score);
  } else {
    outerInstance->spare->doc = doc;
    outerInstance->spare->score = score;
  }
  outerInstance->spare =
      outerInstance->insert(outerInstance->spare, base, keySource);
}

bool operator < (shared_ptr<ScoreDocKey> const hitA, shared_ptr<ScoreDocKey> const hitB)
{
  if (hitA->score == hitB->score) {
    return hitA->doc > hitB->doc;
  } else {
    return hitA->score < hitB->score;
  }
}

ScoreDocKey::ScoreDocKey(int doc, float score)
{
  ScoreDoc(doc, score);
}

optional<int64_t> ScoreDocKey::getKey()
{
  return key;
}

wstring ScoreDocKey::toString()
{
  return L"key:" + to_wstring(*key) + L" doc=" + to_wstring(doc) + L" s=" +
         to_wstring(score);
}
} // namespace org::apache::lucene::search