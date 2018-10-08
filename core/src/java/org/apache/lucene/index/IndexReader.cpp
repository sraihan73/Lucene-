using namespace std;

#include "IndexReader.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using DocumentStoredFieldVisitor =
    org::apache::lucene::document::DocumentStoredFieldVisitor;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Bits = org::apache::lucene::util::Bits;

IndexReader::IndexReader()
{
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  if (!(std::dynamic_pointer_cast<CompositeReader>(shared_from_this()) !=
            nullptr ||
        std::dynamic_pointer_cast<LeafReader>(shared_from_this()) != nullptr)) {
    throw make_shared<Error>(
        L"IndexReader should never be directly extended, subclass LeafReader "
        L"or CompositeReader instead.");
  }
}

IndexReader::CacheKey::CacheKey() {} // only instantiable by core impls

void IndexReader::registerParentReader(shared_ptr<IndexReader> reader)
{
  ensureOpen();
  parentReaders->add(reader);
}

void IndexReader::notifyReaderClosedListeners() 
{
  // nothing to notify in the base impl
}

void IndexReader::reportCloseToParentReaders() 
{
  {
    lock_guard<mutex> lock(parentReaders);
    for (auto parent : parentReaders) {
      parent->closedByChild = true;
      // cross memory barrier by a fake write:
      parent->refCount->addAndGet(0);
      // recurse:
      parent->reportCloseToParentReaders();
    }
  }
}

int IndexReader::getRefCount()
{
  // NOTE: don't ensureOpen, so that callers can see
  // refCount is 0 (reader is closed)
  return refCount->get();
}

void IndexReader::incRef()
{
  if (!tryIncRef()) {
    ensureOpen();
  }
}

bool IndexReader::tryIncRef()
{
  int count;
  while ((count = refCount->get()) > 0) {
    if (refCount->compareAndSet(count, count + 1)) {
      return true;
    }
  }
  return false;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("try") public final void decRef() throws
// java.io.IOException
void IndexReader::decRef() 
{
  // only check refcount here (don't call ensureOpen()), so we can
  // still close the reader if it was made invalid by a child:
  if (refCount->get() <= 0) {
    throw make_shared<AlreadyClosedException>(L"this IndexReader is closed");
  }

  constexpr int rc = refCount->decrementAndGet();
  if (rc == 0) {
    closed = true;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.Closeable finalizer =
    // this::reportCloseToParentReaders; java.io.Closeable finalizer1 =
    // this::notifyReaderClosedListeners)
    {
      java::io::Closeable finalizer =
          shared_from_this()::reportCloseToParentReaders;
      java::io::Closeable finalizer1 =
          shared_from_this()::notifyReaderClosedListeners;
      doClose();
    }
  } else if (rc < 0) {
    throw make_shared<IllegalStateException>(
        L"too many decRef calls: refCount is " + to_wstring(rc) +
        L" after decrement");
  }
}

void IndexReader::ensureOpen() 
{
  if (refCount->get() <= 0) {
    throw make_shared<AlreadyClosedException>(L"this IndexReader is closed");
  }
  // the happens before rule on reading the refCount, which must be after the
  // fake write, ensures that we see the value:
  if (closedByChild) {
    throw make_shared<AlreadyClosedException>(
        L"this IndexReader cannot be used anymore as one of its child readers "
        L"was closed");
  }
}

bool IndexReader::equals(any obj) { return (shared_from_this() == obj); }

int IndexReader::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

shared_ptr<Terms>
IndexReader::getTermVector(int docID, const wstring &field) 
{
  shared_ptr<Fields> vectors = getTermVectors(docID);
  if (vectors->empty()) {
    return nullptr;
  }
  return vectors->terms(field);
}

int IndexReader::numDeletedDocs() { return maxDoc() - numDocs(); }

shared_ptr<Document> IndexReader::document(int docID) 
{
  shared_ptr<DocumentStoredFieldVisitor> *const visitor =
      make_shared<DocumentStoredFieldVisitor>();
  document(docID, visitor);
  return visitor->getDocument();
}

shared_ptr<Document>
IndexReader::document(int docID,
                      shared_ptr<Set<wstring>> fieldsToLoad) 
{
  shared_ptr<DocumentStoredFieldVisitor> *const visitor =
      make_shared<DocumentStoredFieldVisitor>(fieldsToLoad);
  document(docID, visitor);
  return visitor->getDocument();
}

bool IndexReader::hasDeletions() { return numDeletedDocs() > 0; }

// C++ WARNING: The following method was originally marked 'synchronized':
IndexReader::~IndexReader()
{
  if (!closed) {
    decRef();
    closed = true;
  }
}

deque<std::shared_ptr<LeafReaderContext>> IndexReader::leaves()
{
  return getContext()->leaves();
}
} // namespace org::apache::lucene::index