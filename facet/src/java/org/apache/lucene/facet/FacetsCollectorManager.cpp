using namespace std;

#include "FacetsCollectorManager.h"

namespace org::apache::lucene::facet
{
using CollectorManager = org::apache::lucene::search::CollectorManager;

FacetsCollectorManager::FacetsCollectorManager() {}

shared_ptr<FacetsCollector>
FacetsCollectorManager::newCollector() 
{
  return make_shared<FacetsCollector>();
}

shared_ptr<FacetsCollector> FacetsCollectorManager::reduce(
    shared_ptr<deque<std::shared_ptr<FacetsCollector>>>
        collectors) 
{
  if (collectors == nullptr || collectors->size() == 0) {
    return make_shared<FacetsCollector>();
  }
  if (collectors->size() == 1) {
    return collectors->begin()->next();
  }
  return make_shared<ReducedFacetsCollector>(collectors);
}

FacetsCollectorManager::ReducedFacetsCollector::ReducedFacetsCollector(
    shared_ptr<deque<std::shared_ptr<FacetsCollector>>> facetsCollectors)
{
  const deque<std::shared_ptr<MatchingDocs>> matchingDocs =
      this->getMatchingDocs();
  facetsCollectors->forEach([&](any facetsCollector) {
    matchingDocs.addAll(facetsCollector::getMatchingDocs());
  });
}
} // namespace org::apache::lucene::facet