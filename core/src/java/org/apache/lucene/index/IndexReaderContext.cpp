using namespace std;

#include "IndexReaderContext.h"

namespace org::apache::lucene::index
{

IndexReaderContext::IndexReaderContext(
    shared_ptr<CompositeReaderContext> parent, int ordInParent,
    int docBaseInParent)
    : parent(parent), isTopLevel(parent == nullptr),
      docBaseInParent(docBaseInParent), ordInParent(ordInParent)
{
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  if (!(std::dynamic_pointer_cast<CompositeReaderContext>(shared_from_this()) !=
            nullptr ||
        std::dynamic_pointer_cast<LeafReaderContext>(shared_from_this()) !=
            nullptr)) {
    throw make_shared<Error>(
        L"This class should never be extended by custom code!");
  }
}

any IndexReaderContext::id() { return identity; }
} // namespace org::apache::lucene::index