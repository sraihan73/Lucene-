using namespace std;

#include "LeafReaderContext.h"

namespace org::apache::lucene::index
{

LeafReaderContext::LeafReaderContext(shared_ptr<CompositeReaderContext> parent,
                                     shared_ptr<LeafReader> reader, int ord,
                                     int docBase, int leafOrd, int leafDocBase)
    : IndexReaderContext(parent, ord, docBase), ord(leafOrd),
      docBase(leafDocBase), reader(reader),
      leaves(isTopLevel ? Collections::singletonList(shared_from_this())
                        : nullptr)
{
}

LeafReaderContext::LeafReaderContext(shared_ptr<LeafReader> leafReader)
    : LeafReaderContext(nullptr, leafReader, 0, 0, 0, 0)
{
}

deque<std::shared_ptr<LeafReaderContext>> LeafReaderContext::leaves()
{
  if (!isTopLevel) {
    throw make_shared<UnsupportedOperationException>(
        L"This is not a top-level context.");
  }
  assert(leaves_.size() > 0);
  return leaves_;
}

deque<std::shared_ptr<IndexReaderContext>> LeafReaderContext::children()
{
  return nullptr;
}

shared_ptr<LeafReader> LeafReaderContext::reader() { return reader_; }

wstring LeafReaderContext::toString()
{
  return L"LeafReaderContext(" + reader_ + L" docBase=" + to_wstring(docBase) +
         L" ord=" + to_wstring(ord) + L")";
}
} // namespace org::apache::lucene::index