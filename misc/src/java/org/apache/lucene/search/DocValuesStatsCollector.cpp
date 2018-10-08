using namespace std;

#include "DocValuesStatsCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

template <typename T1>
DocValuesStatsCollector::DocValuesStatsCollector(
    shared_ptr<DocValuesStats<T1>> stats)
    : stats(stats)
{
}

shared_ptr<LeafCollector> DocValuesStatsCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  bool shouldProcess = stats->init(context);
  if (!shouldProcess) {
    // Stats cannot be computed for this segment, therefore consider all
    // matching documents as a 'miss'.
    return make_shared<LeafCollectorAnonymousInnerClass>(shared_from_this());
  }

  return make_shared<LeafCollectorAnonymousInnerClass2>(shared_from_this());
}

DocValuesStatsCollector::LeafCollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass(
        shared_ptr<DocValuesStatsCollector> outerInstance)
{
  this->outerInstance = outerInstance;
}

void DocValuesStatsCollector::LeafCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer) 
{
}

void DocValuesStatsCollector::LeafCollectorAnonymousInnerClass::collect(
    int doc) 
{
  // All matching documents in this reader are missing a value
  outerInstance->stats->addMissing();
}

DocValuesStatsCollector::LeafCollectorAnonymousInnerClass2::
    LeafCollectorAnonymousInnerClass2(
        shared_ptr<DocValuesStatsCollector> outerInstance)
{
  this->outerInstance = outerInstance;
}

void DocValuesStatsCollector::LeafCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer) 
{
}

void DocValuesStatsCollector::LeafCollectorAnonymousInnerClass2::collect(
    int doc) 
{
  outerInstance->stats->accumulate(doc);
}

bool DocValuesStatsCollector::needsScores() { return false; }
} // namespace org::apache::lucene::search