using namespace std;

#include "SimpleFSDirectory.h"

namespace org::apache::lucene::store
{

SimpleFSDirectory::SimpleFSDirectory(
    shared_ptr<Path> path,
    shared_ptr<LockFactory> lockFactory) 
    : FSDirectory(path, lockFactory)
{
}

SimpleFSDirectory::SimpleFSDirectory(shared_ptr<Path> path) 
    : SimpleFSDirectory(path, FSLockFactory::getDefault())
{
}

shared_ptr<IndexInput>
SimpleFSDirectory::openInput(const wstring &name,
                             shared_ptr<IOContext> context) 
{
  ensureOpen();
  ensureCanRead(name);
  shared_ptr<Path> path = directory->resolve(name);
  shared_ptr<SeekableByteChannel> channel =
      Files::newByteChannel(path, StandardOpenOption::READ);
  return make_shared<SimpleFSIndexInput>(
      L"SimpleFSIndexInput(path=\"" + path + L"\")", channel, context);
}

SimpleFSDirectory::SimpleFSIndexInput::SimpleFSIndexInput(
    const wstring &resourceDesc, shared_ptr<SeekableByteChannel> channel,
    shared_ptr<IOContext> context) 
    : BufferedIndexInput(resourceDesc, context), channel(channel), off(0LL),
      end(channel->size())
{
}

SimpleFSDirectory::SimpleFSIndexInput::SimpleFSIndexInput(
    const wstring &resourceDesc, shared_ptr<SeekableByteChannel> channel,
    int64_t off, int64_t length, int bufferSize)
    : BufferedIndexInput(resourceDesc, bufferSize), channel(channel), off(off),
      end(off + length)
{
  this->isClone = true;
}

SimpleFSDirectory::SimpleFSIndexInput::~SimpleFSIndexInput()
{
  if (!isClone) {
    channel->close();
  }
}

shared_ptr<SimpleFSIndexInput> SimpleFSDirectory::SimpleFSIndexInput::clone()
{
  shared_ptr<SimpleFSIndexInput> clone =
      std::static_pointer_cast<SimpleFSIndexInput>(BufferedIndexInput::clone());
  clone->isClone = true;
  return clone;
}

shared_ptr<IndexInput> SimpleFSDirectory::SimpleFSIndexInput::slice(
    const wstring &sliceDescription, int64_t offset,
    int64_t length) 
{
  if (offset < 0 || length < 0 || offset + length > this->length()) {
    throw invalid_argument(L"slice() " + sliceDescription +
                           L" out of bounds: offset=" + to_wstring(offset) +
                           L",length=" + to_wstring(length) + L",fileLength=" +
                           to_wstring(this->length()) + L": " +
                           shared_from_this());
  }
  return make_shared<SimpleFSIndexInput>(
      getFullSliceDescription(sliceDescription), channel, off + offset, length,
      getBufferSize());
}

int64_t SimpleFSDirectory::SimpleFSIndexInput::length() { return end - off; }

void SimpleFSDirectory::SimpleFSIndexInput::newBuffer(
    std::deque<char> &newBuffer)
{
  BufferedIndexInput::newBuffer(newBuffer);
  byteBuf = ByteBuffer::wrap(newBuffer);
}

void SimpleFSDirectory::SimpleFSIndexInput::readInternal(
    std::deque<char> &b, int offset, int len) 
{
  shared_ptr<ByteBuffer> *const bb;

  // Determine the ByteBuffer we should use
  if (b == buffer) {
    // Use our own pre-wrapped byteBuf:
    assert(byteBuf != nullptr);
    bb = byteBuf;
    byteBuf->clear().position(offset);
  } else {
    bb = ByteBuffer::wrap(b, offset, len);
  }

  {
    lock_guard<mutex> lock(channel);
    int64_t pos = getFilePointer() + off;

    if (pos + len > end) {
      // C++ TODO: The following line could not be converted:
      throw java.io.EOFException(L"read past EOF: " + this);
    }

    try {
      channel->position(pos);

      int readLength = len;
      while (readLength > 0) {
        constexpr int toRead = min(CHUNK_SIZE, readLength);
        bb->limit(bb->position() + toRead);
        assert(bb->remaining() == toRead);
        constexpr int i = channel->read(bb);
        if (i < 0) { // be defensive here, even though we checked before hand,
                     // something could have changed
                     // C++ TODO: The following line could not be converted:
          throw java.io.EOFException(
              L"read past EOF: " + this + L" off: " + offset + L" len: " + len +
              L" pos: " + pos + L" chunkLen: " + toRead + L" end: " + end);
        }
        assert((i > 0,
                L"SeekableByteChannel.read with non zero-length bb.remaining() "
                L"must always read at least one byte (Channel is in blocking "
                L"mode, see spec of ReadableByteChannel)"));
        pos += i;
        readLength -= i;
      }
      assert(readLength == 0);
    } catch (const IOException &ioe) {
      throw make_shared<IOException>(
          ioe->getMessage() + L": " + shared_from_this(), ioe);
    }
  }
}

void SimpleFSDirectory::SimpleFSIndexInput::seekInternal(int64_t pos) throw(
    IOException)
{
  if (pos > length()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"read past EOF: pos=" + pos + L" vs length=" +
                               length() + L": " + this);
  }
}
} // namespace org::apache::lucene::store