using namespace std;

#include "NIOFSDirectory.h"

namespace org::apache::lucene::store
{

NIOFSDirectory::NIOFSDirectory(
    shared_ptr<Path> path,
    shared_ptr<LockFactory> lockFactory) 
    : FSDirectory(path, lockFactory)
{
}

NIOFSDirectory::NIOFSDirectory(shared_ptr<Path> path) 
    : NIOFSDirectory(path, FSLockFactory::getDefault())
{
}

shared_ptr<IndexInput>
NIOFSDirectory::openInput(const wstring &name,
                          shared_ptr<IOContext> context) 
{
  ensureOpen();
  ensureCanRead(name);
  shared_ptr<Path> path = getDirectory()->resolve(name);
  shared_ptr<FileChannel> fc =
      FileChannel::open(path, StandardOpenOption::READ);
  return make_shared<NIOFSIndexInput>(
      L"NIOFSIndexInput(path=\"" + path + L"\")", fc, context);
}

NIOFSDirectory::NIOFSIndexInput::NIOFSIndexInput(
    const wstring &resourceDesc, shared_ptr<FileChannel> fc,
    shared_ptr<IOContext> context) 
    : BufferedIndexInput(resourceDesc, context), channel(fc), off(0LL),
      end(fc->size())
{
}

NIOFSDirectory::NIOFSIndexInput::NIOFSIndexInput(const wstring &resourceDesc,
                                                 shared_ptr<FileChannel> fc,
                                                 int64_t off,
                                                 int64_t length,
                                                 int bufferSize)
    : BufferedIndexInput(resourceDesc, bufferSize), channel(fc), off(off),
      end(off + length)
{
  this->isClone = true;
}

NIOFSDirectory::NIOFSIndexInput::~NIOFSIndexInput()
{
  if (!isClone) {
    channel->close();
  }
}

shared_ptr<NIOFSIndexInput> NIOFSDirectory::NIOFSIndexInput::clone()
{
  shared_ptr<NIOFSIndexInput> clone =
      std::static_pointer_cast<NIOFSIndexInput>(BufferedIndexInput::clone());
  clone->isClone = true;
  return clone;
}

shared_ptr<IndexInput>
NIOFSDirectory::NIOFSIndexInput::slice(const wstring &sliceDescription,
                                       int64_t offset,
                                       int64_t length) 
{
  if (offset < 0 || length < 0 || offset + length > this->length()) {
    throw invalid_argument(L"slice() " + sliceDescription +
                           L" out of bounds: offset=" + to_wstring(offset) +
                           L",length=" + to_wstring(length) + L",fileLength=" +
                           to_wstring(this->length()) + L": " +
                           shared_from_this());
  }
  return make_shared<NIOFSIndexInput>(getFullSliceDescription(sliceDescription),
                                      channel, off + offset, length,
                                      getBufferSize());
}

int64_t NIOFSDirectory::NIOFSIndexInput::length() { return end - off; }

void NIOFSDirectory::NIOFSIndexInput::newBuffer(std::deque<char> &newBuffer)
{
  BufferedIndexInput::newBuffer(newBuffer);
  byteBuf = ByteBuffer::wrap(newBuffer);
}

void NIOFSDirectory::NIOFSIndexInput::readInternal(std::deque<char> &b,
                                                   int offset,
                                                   int len) 
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

  int64_t pos = getFilePointer() + off;

  if (pos + len > end) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"read past EOF: " + this);
  }

  try {
    int readLength = len;
    while (readLength > 0) {
      constexpr int toRead = min(CHUNK_SIZE, readLength);
      bb->limit(bb->position() + toRead);
      assert(bb->remaining() == toRead);
      constexpr int i = channel->read(bb, pos);
      if (i < 0) { // be defensive here, even though we checked before hand,
                   // something could have changed
                   // C++ TODO: The following line could not be converted:
        throw java.io.EOFException(L"read past EOF: " + this + L" off: " +
                                   offset + L" len: " + len + L" pos: " + pos +
                                   L" chunkLen: " + toRead + L" end: " + end);
      }
      assert((i > 0, L"FileChannel.read with non zero-length bb.remaining() "
                     L"must always read at least one byte (FileChannel is in "
                     L"blocking mode, see spec of ReadableByteChannel)"));
      pos += i;
      readLength -= i;
    }
    assert(readLength == 0);
  } catch (const IOException &ioe) {
    throw make_shared<IOException>(
        ioe->getMessage() + L": " + shared_from_this(), ioe);
  }
}

void NIOFSDirectory::NIOFSIndexInput::seekInternal(int64_t pos) throw(
    IOException)
{
  if (pos > length()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"read past EOF: pos=" + pos + L" vs length=" +
                               length() + L": " + this);
  }
}
} // namespace org::apache::lucene::store