using namespace std;

#include "FilterLeafCollector.h"

namespace org::apache::lucene::search
{

FilterLeafCollector::FilterLeafCollector(shared_ptr<LeafCollector> in_)
    : in_(in_)
{
}

void FilterLeafCollector::setScorer(shared_ptr<Scorer> scorer) throw(
    IOException)
{
  in_->setScorer(scorer);
}

void FilterLeafCollector::collect(int doc) 
{
  in_->collect(doc);
}

wstring FilterLeafCollector::toString()
{
  wstring name = getClass().getSimpleName();
  if (name.length() == 0) {
    // an anonoymous subclass will have empty name?
    name = L"FilterLeafCollector";
  }
  return name + L"(" + in_ + L")";
}
} // namespace org::apache::lucene::search