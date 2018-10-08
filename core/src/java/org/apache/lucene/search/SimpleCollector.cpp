using namespace std;

#include "SimpleCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

shared_ptr<LeafCollector> SimpleCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  doSetNextReader(context);
  return shared_from_this();
}

void SimpleCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
}

void SimpleCollector::setScorer(shared_ptr<Scorer> scorer) 
{
  // no-op by default
}
} // namespace org::apache::lucene::search