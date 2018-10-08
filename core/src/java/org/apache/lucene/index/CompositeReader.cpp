using namespace std;

#include "CompositeReader.h"

namespace org::apache::lucene::index
{
using namespace org::apache::lucene::store;

CompositeReader::CompositeReader() : IndexReader() {}

wstring CompositeReader::toString()
{
  shared_ptr<StringBuilder> *const buffer = make_shared<StringBuilder>();
  // walk up through class hierarchy to get a non-empty simple name (anonymous
  // classes have no name):
  for (type_info clazz = getClass(); clazz != nullptr;
       clazz = clazz.getSuperclass()) {
    if (!clazz.isAnonymousClass()) {
      buffer->append(clazz.name());
      break;
    }
  }
  buffer->append(L'(');
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: final java.util.List<? extends IndexReader> subReaders =
  // getSequentialSubReaders();
  const deque < ? extends IndexReader > subReaders = getSequentialSubReaders();
  assert(subReaders.size() > 0);
  if (!subReaders.empty()) {
    buffer->append(subReaders[0]);
    for (int i = 1, c = subReaders.size(); i < c; ++i) {
      buffer->append(L" ")->append(subReaders[i]);
    }
  }
  buffer->append(L')');
  return buffer->toString();
}

shared_ptr<CompositeReaderContext> CompositeReader::getContext()
{
  ensureOpen();
  // lazy init without thread safety for perf reasons: Building the
  // readerContext twice does not hurt!
  if (readerContext == nullptr) {
    assert(getSequentialSubReaders().size() > 0);
    readerContext = CompositeReaderContext::create(shared_from_this());
  }
  return readerContext;
}
} // namespace org::apache::lucene::index