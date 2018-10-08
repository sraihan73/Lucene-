using namespace std;

#include "FilterCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

FilterCollector::FilterCollector(shared_ptr<Collector> in_) : in_(in_) {}

shared_ptr<LeafCollector> FilterCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  return in_->getLeafCollector(context);
}

wstring FilterCollector::toString()
{
  return getClass().getSimpleName() + L"(" + in_ + L")";
}

bool FilterCollector::needsScores() { return in_->needsScores(); }
} // namespace org::apache::lucene::search