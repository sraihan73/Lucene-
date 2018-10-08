using namespace std;

#include "RAFDirectory.h"

namespace org::apache::lucene::store
{
using org::apache::lucene::util::SuppressForbidden;

RAFDirectory::RAFDirectory(
    shared_ptr<Path> path,
    shared_ptr<LockFactory> lockFactory) 
    : FSDirectory(path, lockFactory)
{
  path->toFile(); // throw exception if we can't get a File
}

RAFDirectory::RAFDirectory(shared_ptr<Path> path) 
    : RAFDirectory(path, FSLockFactory::getDefault())
{
}

shared_ptr<IndexInput>
RAFDirectory::openInput(const wstring &name,
                        shared_ptr<IOContext> context) 
{
  ensureOpen();
  ensureCanRead(name);
  shared_ptr<File> *const path = directory->resolve(name).toFile();
  shared_ptr<RandomAccessFile> raf = make_shared<RandomAccessFile>(path, L"r");
  return make_shared<RAFIndexInput>(
      L"SimpleFSIndexInput(path=\"" + path->getPath() + L"\")", raf, context);
}

RAFDirectory::RAFIndexInput::RAFIndexInput(
    const wstring &resourceDesc, shared_ptr<RandomAccessFile> file,
    shared_ptr<IOContext> context) 
    : BufferedIndexInput(resourceDesc, context), file(file), off(0LL),
      end(file->length())
{
}

RAFDirectory::RAFIndexInput::RAFIndexInput(const wstring &resourceDesc,
                                           shared_ptr<RandomAccessFile> file,
                                           int64_t off, int64_t length,
                                           int bufferSize)
    : BufferedIndexInput(resourceDesc, bufferSize), file(file), off(off),
      end(off + length)
{
  this->isClone = true;
}

RAFDirectory::RAFIndexInput::~RAFIndexInput()
{
  if (!isClone) {
    file->close();
  }
}

shared_ptr<RAFIndexInput> RAFDirectory::RAFIndexInput::clone()
{
  shared_ptr<RAFIndexInput> clone =
      std::static_pointer_cast<RAFIndexInput>(BufferedIndexInput::clone());
  clone->isClone = true;
  return clone;
}

shared_ptr<IndexInput>
RAFDirectory::RAFIndexInput::slice(const wstring &sliceDescription,
                                   int64_t offset,
                                   int64_t length) 
{
  if (offset < 0 || length < 0 || offset + length > this->length()) {
    throw invalid_argument(L"slice() " + sliceDescription +
                           L" out of bounds: " + shared_from_this());
  }
  return make_shared<RAFIndexInput>(sliceDescription, file, off + offset,
                                    length, getBufferSize());
}

int64_t RAFDirectory::RAFIndexInput::length() { return end - off; }

void RAFDirectory::RAFIndexInput::readInternal(std::deque<char> &b, int offset,
                                               int len) 
{
  {
    lock_guard<mutex> lock(file);
    int64_t position = off + getFilePointer();
    file->seek(position);
    int total = 0;

    if (position + len > end) {
      // C++ TODO: The following line could not be converted:
      throw java.io.EOFException(L"read past EOF: " + this);
    }

    try {
      while (total < len) {
        constexpr int toRead = min(CHUNK_SIZE, len - total);
        constexpr int i = file->read(b, offset + total, toRead);
        if (i < 0) { // be defensive here, even though we checked before hand,
                     // something could have changed
                     // C++ TODO: The following line could not be converted:
          throw java.io.EOFException(
              L"read past EOF: " + this + L" off: " + offset + L" len: " + len +
              L" total: " + total + L" chunkLen: " + toRead + L" end: " + end);
        }
        assert((i > 0, L"RandomAccessFile.read with non zero-length toRead "
                       L"must always read at least one byte"));
        total += i;
      }
      assert(total == len);
    } catch (const IOException &ioe) {
      throw make_shared<IOException>(
          ioe->getMessage() + L": " + shared_from_this(), ioe);
    }
  }
}

void RAFDirectory::RAFIndexInput::seekInternal(int64_t pos) 
{
  if (pos > length()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"read past EOF: pos=" + pos + L" vs length=" +
                               length() + L": " + this);
  }
}

bool RAFDirectory::RAFIndexInput::isFDValid() 
{
  return file->getFD().valid();
}
} // namespace org::apache::lucene::store