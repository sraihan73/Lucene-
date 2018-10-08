using namespace std;

#include "MultiCollectorManager.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SafeVarargs @SuppressWarnings({"varargs", "unchecked"})
// public MultiCollectorManager(final CollectorManager<? extends Collector, ?>...
// collectorManagers) C++ TODO: Java wildcard generics are not converted to C++:
    MultiCollectorManager::MultiCollectorManager(deque<CollectorManager<? extends Collector, ?>> &collectorManagers) : collectorManagers(static_cast<std::deque<std::shared_ptr<CollectorManager>>>(collectorManagers))
    {
    }

    shared_ptr<Collectors>
    MultiCollectorManager::newCollector() 
    {
      return make_shared<Collectors>(shared_from_this());
    }

    std::deque<any> MultiCollectorManager::reduce(
        shared_ptr<deque<std::shared_ptr<Collectors>>>
            reducableCollectors) 
    {
      constexpr int size = reducableCollectors->size();
      const std::deque<any> results =
          std::deque<any>(collectorManagers.size());
      for (int i = 0; i < collectorManagers.size(); i++) {
        const deque<std::shared_ptr<Collector>> reducableCollector =
            deque<std::shared_ptr<Collector>>(size);
        for (auto collectors : reducableCollectors) {
          reducableCollector.push_back(collectors->collectors[i]);
        }
        results[i] = collectorManagers[i]->reduce(reducableCollector);
      }
      return results;
    }

    MultiCollectorManager::Collectors::Collectors(
        shared_ptr<MultiCollectorManager> outerInstance) 
        : collectors(std::deque<std::shared_ptr<Collector>>(
              outerInstance->collectorManagers.size())),
          outerInstance(outerInstance)
    {
      for (int i = 0; i < collectors.size(); i++) {
        collectors[i] = outerInstance->collectorManagers[i]->newCollector();
      }
    }

    shared_ptr<LeafCollector>
    MultiCollectorManager::Collectors::getLeafCollector(
        shared_ptr<LeafReaderContext> context) 
    {
      return make_shared<LeafCollectors>(shared_from_this(), context);
    }

    bool MultiCollectorManager::Collectors::needsScores()
    {
      for (auto collector : collectors) {
        if (collector->needsScores()) {
          return true;
        }
      }
      return false;
    }

    MultiCollectorManager::Collectors::LeafCollectors::LeafCollectors(
        shared_ptr<MultiCollectorManager::Collectors> outerInstance,
        shared_ptr<LeafReaderContext> context) 
        : leafCollectors(std::deque<std::shared_ptr<LeafCollector>>(
              outerInstance->collectors.size())),
          outerInstance(outerInstance)
    {
      for (int i = 0; i < outerInstance->collectors.size(); i++) {
        leafCollectors[i] =
            outerInstance->collectors[i]->getLeafCollector(context);
      }
    }

    void MultiCollectorManager::Collectors::LeafCollectors::setScorer(
        shared_ptr<Scorer> scorer) 
    {
      for (auto leafCollector : leafCollectors) {
        if (leafCollector != nullptr) {
          leafCollector->setScorer(scorer);
        }
      }
    }

    void MultiCollectorManager::Collectors::LeafCollectors::collect(
        int const doc) 
    {
      for (auto leafCollector : leafCollectors) {
        if (leafCollector != nullptr) {
          leafCollector->collect(doc);
        }
      }
    }
    } // namespace org::apache::lucene::search