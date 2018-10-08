using namespace std;

#include "CompositeReaderContext.h"

namespace org::apache::lucene::index
{

shared_ptr<CompositeReaderContext>
CompositeReaderContext::create(shared_ptr<CompositeReader> reader)
{
  return (make_shared<Builder>(reader))->build();
}

CompositeReaderContext::CompositeReaderContext(
    shared_ptr<CompositeReaderContext> parent,
    shared_ptr<CompositeReader> reader, int ordInParent, int docbaseInParent,
    deque<std::shared_ptr<IndexReaderContext>> &children)
    : CompositeReaderContext(parent, reader, ordInParent, docbaseInParent,
                             children, nullptr)
{
}

CompositeReaderContext::CompositeReaderContext(
    shared_ptr<CompositeReader> reader,
    deque<std::shared_ptr<IndexReaderContext>> &children,
    deque<std::shared_ptr<LeafReaderContext>> &leaves)
    : CompositeReaderContext(nullptr, reader, 0, 0, children, leaves)
{
}

CompositeReaderContext::CompositeReaderContext(
    shared_ptr<CompositeReaderContext> parent,
    shared_ptr<CompositeReader> reader, int ordInParent, int docbaseInParent,
    deque<std::shared_ptr<IndexReaderContext>> &children,
    deque<std::shared_ptr<LeafReaderContext>> &leaves)
    : IndexReaderContext(parent, ordInParent, docbaseInParent),
      children(Collections::unmodifiableList(children)),
      leaves(leaves.empty() ? nullptr : Collections::unmodifiableList(leaves)),
      reader(reader)
{
}

deque<std::shared_ptr<LeafReaderContext>>
CompositeReaderContext::leaves() 
{
  if (!isTopLevel) {
    throw make_shared<UnsupportedOperationException>(
        L"This is not a top-level context.");
  }
  assert(leaves_.size() > 0);
  return leaves_;
}

deque<std::shared_ptr<IndexReaderContext>> CompositeReaderContext::children()
{
  return children_;
}

shared_ptr<CompositeReader> CompositeReaderContext::reader() { return reader_; }

CompositeReaderContext::Builder::Builder(shared_ptr<CompositeReader> reader)
    : reader(reader)
{
}

shared_ptr<CompositeReaderContext> CompositeReaderContext::Builder::build()
{
  return std::static_pointer_cast<CompositeReaderContext>(
      build(nullptr, reader, 0, 0));
}

shared_ptr<IndexReaderContext> CompositeReaderContext::Builder::build(
    shared_ptr<CompositeReaderContext> parent, shared_ptr<IndexReader> reader,
    int ord, int docBase)
{
  if (std::dynamic_pointer_cast<LeafReader>(reader) != nullptr) {
    shared_ptr<LeafReader> *const ar =
        std::static_pointer_cast<LeafReader>(reader);
    shared_ptr<LeafReaderContext> *const atomic =
        make_shared<LeafReaderContext>(parent, ar, ord, docBase, leaves.size(),
                                       leafDocBase);
    leaves.push_back(atomic);
    leafDocBase += reader->maxDoc();
    return atomic;
  } else {
    shared_ptr<CompositeReader> *const cr =
        std::static_pointer_cast<CompositeReader>(reader);
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.List<? extends IndexReader>
    // sequentialSubReaders = cr.getSequentialSubReaders();
    const deque < ? extends IndexReader > sequentialSubReaders =
                         cr->getSequentialSubReaders();
    const deque<std::shared_ptr<IndexReaderContext>> children =
        Arrays::asList(std::deque<std::shared_ptr<IndexReaderContext>>(
            sequentialSubReaders.size()));
    shared_ptr<CompositeReaderContext> *const newParent;
    if (parent == nullptr) {
      newParent = make_shared<CompositeReaderContext>(cr, children, leaves);
    } else {
      newParent = make_shared<CompositeReaderContext>(parent, cr, ord, docBase,
                                                      children);
    }
    int newDocBase = 0;
    for (int i = 0, c = sequentialSubReaders.size(); i < c; i++) {
      shared_ptr<IndexReader> *const r = sequentialSubReaders[i];
      children[i] = build(newParent, r, i, newDocBase);
      newDocBase += r->maxDoc();
    }
    assert(newDocBase == cr->maxDoc());
    return newParent;
  }
}
} // namespace org::apache::lucene::index