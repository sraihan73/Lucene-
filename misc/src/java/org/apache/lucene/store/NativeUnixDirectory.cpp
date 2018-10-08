using namespace std;

#include "NativeUnixDirectory.h"

namespace org::apache::lucene::store
{
using Directory = org::apache::lucene::store::Directory;
using Context = org::apache::lucene::store::IOContext::Context;
using org::apache::lucene::util::SuppressForbidden;

NativeUnixDirectory::NativeUnixDirectory(
    shared_ptr<Path> path, int mergeBufferSize, int64_t minBytesDirect,
    shared_ptr<LockFactory> lockFactory,
    shared_ptr<Directory> delegate_) 
    : FSDirectory(path, lockFactory), mergeBufferSize(mergeBufferSize),
      minBytesDirect(minBytesDirect), delegate_(delegate_)
{
  if ((mergeBufferSize & ALIGN) != 0) {
    throw invalid_argument(L"mergeBufferSize must be 0 mod " +
                           to_wstring(ALIGN) + L" (got: " +
                           to_wstring(mergeBufferSize) + L")");
  }
}

NativeUnixDirectory::NativeUnixDirectory(
    shared_ptr<Path> path, shared_ptr<LockFactory> lockFactory,
    shared_ptr<Directory> delegate_) 
    : NativeUnixDirectory(path, DEFAULT_MERGE_BUFFER_SIZE,
                          DEFAULT_MIN_BYTES_DIRECT, lockFactory, delegate_)
{
}

NativeUnixDirectory::NativeUnixDirectory(
    shared_ptr<Path> path, shared_ptr<Directory> delegate_) 
    : NativeUnixDirectory(path, DEFAULT_MERGE_BUFFER_SIZE,
                          DEFAULT_MIN_BYTES_DIRECT, FSLockFactory::getDefault(),
                          delegate_)
{
}

shared_ptr<IndexInput>
NativeUnixDirectory::openInput(const wstring &name,
                               shared_ptr<IOContext> context) 
{
  ensureOpen();
  if (context->context != Context::MERGE ||
      context->mergeInfo->estimatedMergeBytes < minBytesDirect ||
      fileLength(name) < minBytesDirect) {
    return delegate_->openInput(name, context);
  } else {
    return make_shared<NativeUnixIndexInput>(getDirectory()->resolve(name),
                                             mergeBufferSize);
  }
}

shared_ptr<IndexOutput> NativeUnixDirectory::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  ensureOpen();
  if (context->context != Context::MERGE ||
      context->mergeInfo->estimatedMergeBytes < minBytesDirect) {
    return delegate_->createOutput(name, context);
  } else {
    return make_shared<NativeUnixIndexOutput>(getDirectory()->resolve(name),
                                              name, mergeBufferSize);
  }
}

NativeUnixDirectory::NativeUnixIndexOutput::NativeUnixIndexOutput(
    shared_ptr<Path> path, const wstring &name,
    int bufferSize) 
    : IndexOutput(L"NativeUnixIndexOutput(path=\"" + path->toString() + L"\")",
                  name),
      buffer(ByteBuffer::allocateDirect(bufferSize)),
      fos(make_shared<FileOutputStream>(fd)), channel(fos->getChannel()),
      bufferSize(bufferSize)
{
  // this.path = path;
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileDescriptor> *const fd =
      NativePosixUtil::open_direct(path->toString(), false);
  // fos = new FileOutputStream(path);
  isOpen = true;
}

void NativeUnixDirectory::NativeUnixIndexOutput::writeByte(char b) throw(
    IOException)
{
  assert((bufferPos == buffer->position(),
          L"bufferPos=" + to_wstring(bufferPos) + L" vs buffer.position()=" +
              buffer->position()));
  buffer->put(b);
  if (++bufferPos == bufferSize) {
    dump();
  }
}

void NativeUnixDirectory::NativeUnixIndexOutput::writeBytes(
    std::deque<char> &src, int offset, int len) 
{
  int toWrite = len;
  while (true) {
    constexpr int left = bufferSize - bufferPos;
    if (left <= toWrite) {
      buffer->put(src, offset, left);
      toWrite -= left;
      offset += left;
      bufferPos = bufferSize;
      dump();
    } else {
      buffer->put(src, offset, toWrite);
      bufferPos += toWrite;
      break;
    }
  }
}

void NativeUnixDirectory::NativeUnixIndexOutput::dump() 
{
  buffer->flip();
  constexpr int64_t limit = filePos + buffer->limit();
  if (limit > fileLength) {
    // this dump extends the file
    fileLength = limit;
  } else {
    // we had seek'd back & wrote some changes
  }

  // must always round to next block
  buffer->limit(
      static_cast<int>((buffer->limit() + ALIGN - 1) & ALIGN_NOT_MASK));

  assert(buffer->limit() & ALIGN_NOT_MASK) == buffer->limit()
      : L"limit=" + buffer->limit() + L" vs " +
                                                  to_wstring(buffer->limit() &
                                                             ALIGN_NOT_MASK);
  assert(filePos & ALIGN_NOT_MASK) == filePos;
  // System.out.println(Thread.currentThread().getName() + ": dump to " +
  // filePos + " limit=" + buffer.limit() + " fos=" + fos);
  channel->write(buffer, filePos);
  filePos += bufferPos;
  bufferPos = 0;
  buffer->clear();
  // System.out.println("dump: done");

  // TODO: the case where we'd seek'd back, wrote an
  // entire buffer, we must here read the next buffer;
  // likely Lucene won't trip on this since we only
  // write smallish amounts on seeking back
}

int64_t NativeUnixDirectory::NativeUnixIndexOutput::getFilePointer()
{
  return filePos + bufferPos;
}

int64_t
NativeUnixDirectory::NativeUnixIndexOutput::getChecksum() 
{
  throw make_shared<UnsupportedOperationException>(
      L"this directory currently does not work at all!");
}

NativeUnixDirectory::NativeUnixIndexOutput::~NativeUnixIndexOutput()
{
  if (isOpen) {
    isOpen = false;
    try {
      dump();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      try {
        // System.out.println("direct close set len=" + fileLength + " vs " +
        // channel.size() + " path=" + path);
        channel->truncate(fileLength);
        // System.out.println("  now: " + channel.size());
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        try {
          channel->close();
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          fos->close();
          // System.out.println("  final len=" + path.length());
        }
      }
    }
  }
}

NativeUnixDirectory::NativeUnixIndexInput::NativeUnixIndexInput(
    shared_ptr<Path> path, int bufferSize) 
    : IndexInput(L"NativeUnixIndexInput(path=\"" + path + L"\")"),
      buffer(ByteBuffer::allocateDirect(bufferSize)),
      fis(make_shared<FileInputStream>(fd)), channel(fis->getChannel()),
      bufferSize(bufferSize)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FileDescriptor> *const fd =
      NativePosixUtil::open_direct(path->toString(), true);
  isOpen = true;
  isClone = false;
  filePos = -bufferSize;
  bufferPos = bufferSize;
  // System.out.println("D open " + path + " this=" + this);
}

NativeUnixDirectory::NativeUnixIndexInput::NativeUnixIndexInput(
    shared_ptr<NativeUnixIndexInput> other) 
    : IndexInput(other->toString()),
      buffer(ByteBuffer::allocateDirect(bufferSize)), fis(this->fis.reset()),
      channel(other->channel), bufferSize(other->bufferSize)
{
  filePos = -bufferSize;
  bufferPos = bufferSize;
  isOpen = true;
  isClone = true;
  // System.out.println("D clone this=" + this);
  seek(other->getFilePointer());
}

NativeUnixDirectory::NativeUnixIndexInput::~NativeUnixIndexInput()
{
  if (isOpen && !isClone) {
    try {
      channel->close();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!isClone) {
        fis->close();
      }
    }
  }
}

int64_t NativeUnixDirectory::NativeUnixIndexInput::getFilePointer()
{
  return filePos + bufferPos;
}

void NativeUnixDirectory::NativeUnixIndexInput::seek(int64_t pos) throw(
    IOException)
{
  if (pos != getFilePointer()) {
    constexpr int64_t alignedPos = pos & ALIGN_NOT_MASK;
    filePos = alignedPos - bufferSize;

    constexpr int delta = static_cast<int>(pos - alignedPos);
    if (delta != 0) {
      refill();
      buffer->position(delta);
      bufferPos = delta;
    } else {
      // force refill on next read
      bufferPos = bufferSize;
    }
  }
}

int64_t NativeUnixDirectory::NativeUnixIndexInput::length()
{
  try {
    return channel->size();
  } catch (const IOException &ioe) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("IOException during length(): "
    // + this, ioe);
    throw runtime_error(L"IOException during length(): " + shared_from_this());
  }
}

char NativeUnixDirectory::NativeUnixIndexInput::readByte() 
{
  // NOTE: we don't guard against EOF here... ie the
  // "final" buffer will typically be filled to less
  // than bufferSize
  if (bufferPos == bufferSize) {
    refill();
  }
  assert((bufferPos == buffer->position(),
          L"bufferPos=" + to_wstring(bufferPos) + L" vs buffer.position()=" +
              buffer->position()));
  bufferPos++;
  return buffer->get();
}

void NativeUnixDirectory::NativeUnixIndexInput::refill() 
{
  buffer->clear();
  filePos += bufferSize;
  bufferPos = 0;
  assert(filePos & ALIGN_NOT_MASK) == filePos
      : L"filePos=" +
        to_wstring(filePos) + L" anded=" + to_wstring(filePos & ALIGN_NOT_MASK);
  // System.out.println("X refill filePos=" + filePos);
  int n;
  try {
    n = channel->read(buffer, filePos);
  } catch (const IOException &ioe) {
    throw make_shared<IOException>(
        ioe->getMessage() + L": " + shared_from_this(), ioe);
  }
  if (n < 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"read past EOF: " + this);
  }
  buffer->rewind();
}

void NativeUnixDirectory::NativeUnixIndexInput::readBytes(
    std::deque<char> &dst, int offset, int len) 
{
  int toRead = len;
  // System.out.println("\nX readBytes len=" + len + " fp=" + getFilePointer() +
  // " size=" + length() + " this=" + this);
  while (true) {
    constexpr int left = bufferSize - bufferPos;
    if (left < toRead) {
      // System.out.println("  copy " + left);
      buffer->get(dst, offset, left);
      toRead -= left;
      offset += left;
      refill();
    } else {
      // System.out.println("  copy " + toRead);
      buffer->get(dst, offset, toRead);
      bufferPos += toRead;
      // System.out.println("  readBytes done");
      break;
    }
  }
}

shared_ptr<NativeUnixIndexInput>
NativeUnixDirectory::NativeUnixIndexInput::clone()
{
  try {
    return make_shared<NativeUnixIndexInput>(shared_from_this());
  } catch (const IOException &ioe) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("IOException during clone: " +
    // this, ioe);
    throw runtime_error(L"IOException during clone: " + shared_from_this());
  }
}

shared_ptr<IndexInput> NativeUnixDirectory::NativeUnixIndexInput::slice(
    const wstring &sliceDescription, int64_t offset,
    int64_t length) 
{
  // TODO: is this the right thing to do?
  return BufferedIndexInput::wrap(sliceDescription, shared_from_this(), offset,
                                  length);
}
} // namespace org::apache::lucene::store