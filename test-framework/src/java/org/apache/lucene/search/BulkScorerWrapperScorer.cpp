using namespace std;

#include "BulkScorerWrapperScorer.h"

namespace org::apache::lucene::search
{

BulkScorerWrapperScorer::BulkScorerWrapperScorer(shared_ptr<Weight> weight,
                                                 shared_ptr<BulkScorer> scorer,
                                                 int bufferSize)
    : Scorer(weight), scorer(scorer), docs(std::deque<int>(bufferSize)),
      scores(std::deque<float>(bufferSize))
{
}

void BulkScorerWrapperScorer::refill(int target) 
{
  bufferLength = 0;
  while (next != DocIdSetIterator::NO_MORE_DOCS && bufferLength == 0) {
    constexpr int min = max(target, next);
    constexpr int max = min + docs.size();
    next = scorer->score(
        make_shared<LeafCollectorAnonymousInnerClass>(shared_from_this()),
        nullptr, min, max);
  }
  i = -1;
}

BulkScorerWrapperScorer::LeafCollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass(
        shared_ptr<BulkScorerWrapperScorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

void BulkScorerWrapperScorer::LeafCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
}

void BulkScorerWrapperScorer::LeafCollectorAnonymousInnerClass::collect(
    int doc) 
{
  outerInstance->docs[outerInstance->bufferLength] = doc;
  outerInstance->scores[outerInstance->bufferLength] =
      outerInstance->scorer->score();
  outerInstance->bufferLength += 1;
}

float BulkScorerWrapperScorer::score()  { return scores[i]; }

int BulkScorerWrapperScorer::docID() { return doc; }

shared_ptr<DocIdSetIterator> BulkScorerWrapperScorer::iterator()
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this());
}

BulkScorerWrapperScorer::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<BulkScorerWrapperScorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int BulkScorerWrapperScorer::DocIdSetIteratorAnonymousInnerClass::docID()
{
  return outerInstance->doc;
}

int BulkScorerWrapperScorer::DocIdSetIteratorAnonymousInnerClass::
    nextDoc() 
{
  return advance(outerInstance->docID() + 1);
}

int BulkScorerWrapperScorer::DocIdSetIteratorAnonymousInnerClass::advance(
    int target) 
{
  if (outerInstance->bufferLength == 0 ||
      outerInstance->docs[outerInstance->bufferLength - 1] < target) {
    outerInstance->refill(target);
  }

  outerInstance->i =
      Arrays::binarySearch(outerInstance->docs, outerInstance->i + 1,
                           outerInstance->bufferLength, target);
  if (outerInstance->i < 0) {
    outerInstance->i = -1 - outerInstance->i;
  }
  if (outerInstance->i == outerInstance->bufferLength) {
    return outerInstance->doc = DocIdSetIterator::NO_MORE_DOCS;
  }
  return outerInstance->doc = outerInstance->docs[outerInstance->i];
}

int64_t BulkScorerWrapperScorer::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return outerInstance->scorer->cost();
}
} // namespace org::apache::lucene::search