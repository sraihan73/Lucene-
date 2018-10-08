using namespace std;

#include "AssertingLeafCollector.h"

namespace org::apache::lucene::search
{

AssertingLeafCollector::AssertingLeafCollector(
    shared_ptr<Random> random, shared_ptr<LeafCollector> collector, int min,
    int max)
    : FilterLeafCollector(collector), random(random), min(min), max(max)
{
}

void AssertingLeafCollector::setScorer(shared_ptr<Scorer> scorer) throw(
    IOException)
{
  this->scorer = scorer;
  FilterLeafCollector::setScorer(AssertingScorer::wrap(random, scorer, true));
}

void AssertingLeafCollector::collect(int doc) 
{
  assert((doc > lastCollected, L"Out of order : " + to_wstring(lastCollected) +
                                   L" " + to_wstring(doc)));
  assert((doc >= min,
          L"Out of range: " + to_wstring(doc) + L" < " + to_wstring(min)));
  assert((doc < max,
          L"Out of range: " + to_wstring(doc) + L" >= " + to_wstring(max)));
  assert((scorer->docID() == doc, L"Collected: " + to_wstring(doc) +
                                      L" but scorer: " +
                                      to_wstring(scorer->docID())));
  in_->collect(doc);
  lastCollected = doc;
}
} // namespace org::apache::lucene::search