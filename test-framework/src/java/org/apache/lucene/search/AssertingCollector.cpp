using namespace std;

#include "AssertingCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

shared_ptr<Collector> AssertingCollector::wrap(shared_ptr<Random> random,
                                               shared_ptr<Collector> in_)
{
  if (std::dynamic_pointer_cast<AssertingCollector>(in_) != nullptr) {
    return in_;
  }
  return make_shared<AssertingCollector>(random, in_);
}

AssertingCollector::AssertingCollector(shared_ptr<Random> random,
                                       shared_ptr<Collector> in_)
    : FilterCollector(in_), random(random)
{
}

shared_ptr<LeafCollector> AssertingCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafCollector> *const in_ =
      FilterCollector::getLeafCollector(context);
  constexpr int docBase = context->docBase;
  return make_shared<AssertingLeafCollectorAnonymousInnerClass>(
      shared_from_this(), random, DocIdSetIterator::NO_MORE_DOCS, docBase);
}

AssertingCollector::AssertingLeafCollectorAnonymousInnerClass::
    AssertingLeafCollectorAnonymousInnerClass(
        shared_ptr<AssertingCollector> outerInstance, shared_ptr<Random> random,
        int NO_MORE_DOCS, int docBase)
    : AssertingLeafCollector(random, in_, 0, NO_MORE_DOCS)
{
  this->outerInstance = outerInstance;
  this->docBase = docBase;
}

void AssertingCollector::AssertingLeafCollectorAnonymousInnerClass::collect(
    int doc) 
{
  // check that documents are scored in order globally,
  // not only per segment
  assert((docBase + doc >= outerInstance->maxDoc,
          L"collection is not in order: current doc="));
  +to_wstring(docBase + doc) + L" while " + to_wstring(outerInstance->maxDoc) +
      L" has already been collected";
  outerInstance->super->collect(doc);
  outerInstance->maxDoc = docBase + doc;
}
} // namespace org::apache::lucene::search