using namespace std;

#include "ByteBufferGuard.h"

namespace org::apache::lucene::store
{

ByteBufferGuard::ByteBufferGuard(const wstring &resourceDescription,
                                 BufferCleaner cleaner)
    : resourceDescription(resourceDescription), cleaner(cleaner)
{
}

void ByteBufferGuard::invalidateAndUnmap(deque<ByteBuffer> &bufs) throw(
    IOException)
{
  if (cleaner != nullptr) {
    invalidated = true;
    // This call should hopefully flush any CPU caches and as a result make
    // the "invalidated" field update visible to other threads. We specifically
    // don't make "invalidated" field volatile for performance reasons, hoping
    // the JVM won't optimize away reads of that field and hardware should
    // ensure caches are in sync after this call. This isn't entirely
    // "fool-proof" (see LUCENE-7409 discussion), but it has been shown to work
    // in practice and we count on this behavior.
    barrier->lazySet(0);
    // we give other threads a bit of time to finish reads on their
    // ByteBuffer...:
    Thread::yield();
    // finally unmap the ByteBuffers:
    for (shared_ptr<ByteBuffer> b : bufs) {
      cleaner(resourceDescription, b);
    }
  }
}

void ByteBufferGuard::ensureValid()
{
  if (invalidated) {
    // this triggers an AlreadyClosedException in ByteBufferIndexInput:
    throw make_shared<NullPointerException>();
  }
}

void ByteBufferGuard::getBytes(shared_ptr<ByteBuffer> receiver,
                               std::deque<char> &dst, int offset, int length)
{
  ensureValid();
  receiver->get(dst, offset, length);
}

char ByteBufferGuard::getByte(shared_ptr<ByteBuffer> receiver)
{
  ensureValid();
  return receiver->get();
}

short ByteBufferGuard::getShort(shared_ptr<ByteBuffer> receiver)
{
  ensureValid();
  return receiver->getShort();
}

int ByteBufferGuard::getInt(shared_ptr<ByteBuffer> receiver)
{
  ensureValid();
  return receiver->getInt();
}

int64_t ByteBufferGuard::getLong(shared_ptr<ByteBuffer> receiver)
{
  ensureValid();
  return receiver->getLong();
}

char ByteBufferGuard::getByte(shared_ptr<ByteBuffer> receiver, int pos)
{
  ensureValid();
  return receiver->get(pos);
}

short ByteBufferGuard::getShort(shared_ptr<ByteBuffer> receiver, int pos)
{
  ensureValid();
  return receiver->getShort(pos);
}

int ByteBufferGuard::getInt(shared_ptr<ByteBuffer> receiver, int pos)
{
  ensureValid();
  return receiver->getInt(pos);
}

int64_t ByteBufferGuard::getLong(shared_ptr<ByteBuffer> receiver, int pos)
{
  ensureValid();
  return receiver->getLong(pos);
}
} // namespace org::apache::lucene::store