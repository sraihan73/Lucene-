using namespace std;

#include "PositiveScoresOnlyCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

PositiveScoresOnlyCollector::PositiveScoresOnlyCollector(
    shared_ptr<Collector> in_)
    : FilterCollector(in_)
{
}

shared_ptr<LeafCollector> PositiveScoresOnlyCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<FilterLeafCollectorAnonymousInnerClass>(
      shared_from_this(), FilterCollector::getLeafCollector(context));
}

PositiveScoresOnlyCollector::FilterLeafCollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass(
        shared_ptr<PositiveScoresOnlyCollector> outerInstance,
        shared_ptr<org::apache::lucene::search::LeafCollector> getLeafCollector)
    : FilterLeafCollector(getLeafCollector)
{
  this->outerInstance = outerInstance;
}

void PositiveScoresOnlyCollector::FilterLeafCollectorAnonymousInnerClass::
    setScorer(shared_ptr<Scorer> scorer) 
{
  this->scorer = make_shared<ScoreCachingWrappingScorer>(scorer);
  outerInstance->in_->setScorer(this->scorer);
}

void PositiveScoresOnlyCollector::FilterLeafCollectorAnonymousInnerClass::
    collect(int doc) 
{
  if (scorer::score() > 0) {
    outerInstance->in_->collect(doc);
  }
}
} // namespace org::apache::lucene::search