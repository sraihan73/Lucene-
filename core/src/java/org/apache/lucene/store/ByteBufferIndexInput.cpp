using namespace std;

#include "ByteBufferIndexInput.h"

namespace org::apache::lucene::store
{

shared_ptr<ByteBufferIndexInput> ByteBufferIndexInput::newInstance(
    const wstring &resourceDescription,
    std::deque<std::shared_ptr<ByteBuffer>> &buffers, int64_t length,
    int chunkSizePower, shared_ptr<ByteBufferGuard> guard)
{
  if (buffers.size() == 1) {
    return make_shared<SingleBufferImpl>(resourceDescription, buffers[0],
                                         length, chunkSizePower, guard);
  } else {
    return make_shared<MultiBufferImpl>(resourceDescription, buffers, 0, length,
                                        chunkSizePower, guard);
  }
}

ByteBufferIndexInput::ByteBufferIndexInput(
    const wstring &resourceDescription,
    std::deque<std::shared_ptr<ByteBuffer>> &buffers, int64_t length,
    int chunkSizePower, shared_ptr<ByteBufferGuard> guard)
    : IndexInput(resourceDescription), length(length),
      chunkSizeMask((1LL << chunkSizePower) - 1LL),
      chunkSizePower(chunkSizePower), guard(guard)
{
  this->buffers = buffers;
  assert(chunkSizePower >= 0 && chunkSizePower <= 30);
  assert(static_cast<int64_t>(static_cast<uint64_t>(length) >>
                                chunkSizePower)) < numeric_limits<int>::max();
}

char ByteBufferIndexInput::readByte() 
{
  try {
    return guard->getByte(curBuf);
  } catch (const BufferUnderflowException &e) {
    do {
      curBufIndex++;
      if (curBufIndex >= buffers.size()) {
        // C++ TODO: The following line could not be converted:
        throw java.io.EOFException(L"read past EOF: " + this);
      }
      curBuf = buffers[curBufIndex];
      curBuf->position(0);
    } while (!curBuf->hasRemaining());
    return guard->getByte(curBuf);
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

void ByteBufferIndexInput::readBytes(std::deque<char> &b, int offset,
                                     int len) 
{
  try {
    guard->getBytes(curBuf, b, offset, len);
  } catch (const BufferUnderflowException &e) {
    int curAvail = curBuf->remaining();
    while (len > curAvail) {
      guard->getBytes(curBuf, b, offset, curAvail);
      len -= curAvail;
      offset += curAvail;
      curBufIndex++;
      if (curBufIndex >= buffers.size()) {
        // C++ TODO: The following line could not be converted:
        throw java.io.EOFException(L"read past EOF: " + this);
      }
      curBuf = buffers[curBufIndex];
      curBuf->position(0);
      curAvail = curBuf->remaining();
    }
    guard->getBytes(curBuf, b, offset, len);
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

short ByteBufferIndexInput::readShort() 
{
  try {
    return guard->getShort(curBuf);
  } catch (const BufferUnderflowException &e) {
    return IndexInput::readShort();
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

int ByteBufferIndexInput::readInt() 
{
  try {
    return guard->getInt(curBuf);
  } catch (const BufferUnderflowException &e) {
    return IndexInput::readInt();
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

int64_t ByteBufferIndexInput::readLong() 
{
  try {
    return guard->getLong(curBuf);
  } catch (const BufferUnderflowException &e) {
    return IndexInput::readLong();
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

int64_t ByteBufferIndexInput::getFilePointer()
{
  try {
    return ((static_cast<int64_t>(curBufIndex)) << chunkSizePower) +
           curBuf->position();
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

void ByteBufferIndexInput::seek(int64_t pos) 
{
  // we use >> here to preserve negative, so we will catch AIOOBE,
  // in case pos + offset overflows.
  constexpr int bi = static_cast<int>(pos >> chunkSizePower);
  try {
    if (bi == curBufIndex) {
      curBuf->position(static_cast<int>(pos & chunkSizeMask));
    } else {
      shared_ptr<ByteBuffer> *const b = buffers[bi];
      b->position(static_cast<int>(pos & chunkSizeMask));
      // write values, on exception all is unchanged
      this->curBufIndex = bi;
      this->curBuf = b;
    }
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (ArrayIndexOutOfBoundsException | invalid_argument e) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"seek past EOF: " + this);
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

char ByteBufferIndexInput::readByte(int64_t pos) 
{
  try {
    constexpr int bi = static_cast<int>(pos >> chunkSizePower);
    return guard->getByte(buffers[bi], static_cast<int>(pos & chunkSizeMask));
  } catch (const out_of_range &ioobe) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"seek past EOF: " + this);
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

void ByteBufferIndexInput::setPos(int64_t pos, int bi) 
{
  try {
    shared_ptr<ByteBuffer> *const b = buffers[bi];
    b->position(static_cast<int>(pos & chunkSizeMask));
    this->curBufIndex = bi;
    this->curBuf = b;
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (ArrayIndexOutOfBoundsException | invalid_argument aioobe) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"seek past EOF: " + this);
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

short ByteBufferIndexInput::readShort(int64_t pos) 
{
  constexpr int bi = static_cast<int>(pos >> chunkSizePower);
  try {
    return guard->getShort(buffers[bi], static_cast<int>(pos & chunkSizeMask));
  } catch (const out_of_range &ioobe) {
    // either it's a boundary, or read past EOF, fall back:
    setPos(pos, bi);
    return readShort();
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

int ByteBufferIndexInput::readInt(int64_t pos) 
{
  constexpr int bi = static_cast<int>(pos >> chunkSizePower);
  try {
    return guard->getInt(buffers[bi], static_cast<int>(pos & chunkSizeMask));
  } catch (const out_of_range &ioobe) {
    // either it's a boundary, or read past EOF, fall back:
    setPos(pos, bi);
    return readInt();
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

int64_t ByteBufferIndexInput::readLong(int64_t pos) 
{
  constexpr int bi = static_cast<int>(pos >> chunkSizePower);
  try {
    return guard->getLong(buffers[bi], static_cast<int>(pos & chunkSizeMask));
  } catch (const out_of_range &ioobe) {
    // either it's a boundary, or read past EOF, fall back:
    setPos(pos, bi);
    return readLong();
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

int64_t ByteBufferIndexInput::length() { return length_; }

shared_ptr<ByteBufferIndexInput> ByteBufferIndexInput::clone()
{
  shared_ptr<ByteBufferIndexInput> *const clone =
      buildSlice(static_cast<wstring>(nullptr), 0LL, this->length_);
  try {
    clone->seek(getFilePointer());
  } catch (const IOException &ioe) {
    throw make_shared<AssertionError>(ioe);
  }

  return clone;
}

shared_ptr<ByteBufferIndexInput>
ByteBufferIndexInput::slice(const wstring &sliceDescription, int64_t offset,
                            int64_t length)
{
  if (offset < 0 || length < 0 || offset + length > this->length_) {
    throw invalid_argument(L"slice() " + sliceDescription +
                           L" out of bounds: offset=" + to_wstring(offset) +
                           L",length=" + to_wstring(length) + L",fileLength=" +
                           to_wstring(this->length_) + L": " +
                           shared_from_this());
  }

  return buildSlice(sliceDescription, offset, length);
}

shared_ptr<ByteBufferIndexInput>
ByteBufferIndexInput::buildSlice(const wstring &sliceDescription,
                                 int64_t offset, int64_t length)
{
  if (buffers.empty()) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }

  std::deque<std::shared_ptr<ByteBuffer>> newBuffers =
      buildSlice(buffers, offset, length);
  constexpr int ofs = static_cast<int>(offset & chunkSizeMask);

  shared_ptr<ByteBufferIndexInput> *const clone = newCloneInstance(
      getFullSliceDescription(sliceDescription), newBuffers, ofs, length);
  clone->isClone = true;

  return clone;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("resource") protected ByteBufferIndexInput
// newCloneInstance(std::wstring newResourceDescription, java.nio.ByteBuffer[]
// newBuffers, int offset, long length)
shared_ptr<ByteBufferIndexInput> ByteBufferIndexInput::newCloneInstance(
    const wstring &newResourceDescription,
    std::deque<std::shared_ptr<ByteBuffer>> &newBuffers, int offset,
    int64_t length)
{
  if (newBuffers.size() == 1) {
    newBuffers[0]->position(offset);
    return make_shared<SingleBufferImpl>(newResourceDescription,
                                         newBuffers[0]->slice(), length,
                                         chunkSizePower, this->guard);
  } else {
    return make_shared<MultiBufferImpl>(newResourceDescription, newBuffers,
                                        offset, length, chunkSizePower, guard);
  }
}

std::deque<std::shared_ptr<ByteBuffer>> ByteBufferIndexInput::buildSlice(
    std::deque<std::shared_ptr<ByteBuffer>> &buffers, int64_t offset,
    int64_t length)
{
  constexpr int64_t sliceEnd = offset + length;

  constexpr int startIndex = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(offset) >> chunkSizePower));
  constexpr int endIndex = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(sliceEnd) >> chunkSizePower));

  // we always allocate one more slice, the last one may be a 0 byte one
  std::deque<std::shared_ptr<ByteBuffer>> slices(endIndex - startIndex + 1);

  for (int i = 0; i < slices.size(); i++) {
    slices[i] = buffers[startIndex + i]->duplicate();
  }

  // set the last buffer's limit for the sliced view.
  slices[slices.size() - 1]->limit(static_cast<int>(sliceEnd & chunkSizeMask));

  return slices;
}

ByteBufferIndexInput::~ByteBufferIndexInput()
{
  try {
    if (buffers.empty()) {
      return;
    }

    // make local copy, then un-set early
    std::deque<std::shared_ptr<ByteBuffer>> bufs = buffers;
    unsetBuffers();

    if (isClone) {
      return;
    }

    // tell the guard to invalidate and later unmap the bytebuffers (if
    // supported):
    guard->invalidateAndUnmap(bufs);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    unsetBuffers();
  }
}

void ByteBufferIndexInput::unsetBuffers()
{
  buffers.clear();
  curBuf.reset();
  curBufIndex = 0;
}

ByteBufferIndexInput::SingleBufferImpl::SingleBufferImpl(
    const wstring &resourceDescription, shared_ptr<ByteBuffer> buffer,
    int64_t length, int chunkSizePower, shared_ptr<ByteBufferGuard> guard)
    : ByteBufferIndexInput(resourceDescription, new ByteBuffer[]{buffer},
                           length, chunkSizePower, guard)
{
  this->curBufIndex = 0;
  this->curBuf = buffer;
  buffer->position(0);
}

void ByteBufferIndexInput::SingleBufferImpl::seek(int64_t pos) throw(
    IOException)
{
  try {
    curBuf->position(static_cast<int>(pos));
  } catch (const invalid_argument &e) {
    if (pos < 0) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new IllegalArgumentException("Seeking to negative
      // position: " + this, e);
      throw invalid_argument(L"Seeking to negative position: " +
                             shared_from_this());
    } else {
      // C++ TODO: The following line could not be converted:
      throw java.io.EOFException(L"seek past EOF: " + this);
    }
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

int64_t ByteBufferIndexInput::SingleBufferImpl::getFilePointer()
{
  try {
    return curBuf->position();
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

char ByteBufferIndexInput::SingleBufferImpl::readByte(int64_t pos) throw(
    IOException)
{
  try {
    return guard->getByte(curBuf, static_cast<int>(pos));
  } catch (const invalid_argument &e) {
    if (pos < 0) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new IllegalArgumentException("Seeking to negative
      // position: " + this, e);
      throw invalid_argument(L"Seeking to negative position: " +
                             shared_from_this());
    } else {
      // C++ TODO: The following line could not be converted:
      throw java.io.EOFException(L"seek past EOF: " + this);
    }
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

short ByteBufferIndexInput::SingleBufferImpl::readShort(int64_t pos) throw(
    IOException)
{
  try {
    return guard->getShort(curBuf, static_cast<int>(pos));
  } catch (const invalid_argument &e) {
    if (pos < 0) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new IllegalArgumentException("Seeking to negative
      // position: " + this, e);
      throw invalid_argument(L"Seeking to negative position: " +
                             shared_from_this());
    } else {
      // C++ TODO: The following line could not be converted:
      throw java.io.EOFException(L"seek past EOF: " + this);
    }
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

int ByteBufferIndexInput::SingleBufferImpl::readInt(int64_t pos) throw(
    IOException)
{
  try {
    return guard->getInt(curBuf, static_cast<int>(pos));
  } catch (const invalid_argument &e) {
    if (pos < 0) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new IllegalArgumentException("Seeking to negative
      // position: " + this, e);
      throw invalid_argument(L"Seeking to negative position: " +
                             shared_from_this());
    } else {
      // C++ TODO: The following line could not be converted:
      throw java.io.EOFException(L"seek past EOF: " + this);
    }
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

int64_t ByteBufferIndexInput::SingleBufferImpl::readLong(int64_t pos) throw(
    IOException)
{
  try {
    return guard->getLong(curBuf, static_cast<int>(pos));
  } catch (const invalid_argument &e) {
    if (pos < 0) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new IllegalArgumentException("Seeking to negative
      // position: " + this, e);
      throw invalid_argument(L"Seeking to negative position: " +
                             shared_from_this());
    } else {
      // C++ TODO: The following line could not be converted:
      throw java.io.EOFException(L"seek past EOF: " + this);
    }
  } catch (const NullPointerException &npe) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

ByteBufferIndexInput::MultiBufferImpl::MultiBufferImpl(
    const wstring &resourceDescription,
    std::deque<std::shared_ptr<ByteBuffer>> &buffers, int offset,
    int64_t length, int chunkSizePower, shared_ptr<ByteBufferGuard> guard)
    : ByteBufferIndexInput(resourceDescription, buffers, length, chunkSizePower,
                           guard),
      offset(offset)
{
  try {
    seek(0LL);
  } catch (const IOException &ioe) {
    throw make_shared<AssertionError>(ioe);
  }
}

void ByteBufferIndexInput::MultiBufferImpl::seek(int64_t pos) throw(
    IOException)
{
  assert(pos >= 0LL);
  ByteBufferIndexInput::seek(pos + offset);
}

int64_t ByteBufferIndexInput::MultiBufferImpl::getFilePointer()
{
  return ByteBufferIndexInput::getFilePointer() - offset;
}

char ByteBufferIndexInput::MultiBufferImpl::readByte(int64_t pos) throw(
    IOException)
{
  return ByteBufferIndexInput::readByte(pos + offset);
}

short ByteBufferIndexInput::MultiBufferImpl::readShort(int64_t pos) throw(
    IOException)
{
  return ByteBufferIndexInput::readShort(pos + offset);
}

int ByteBufferIndexInput::MultiBufferImpl::readInt(int64_t pos) throw(
    IOException)
{
  return ByteBufferIndexInput::readInt(pos + offset);
}

int64_t ByteBufferIndexInput::MultiBufferImpl::readLong(int64_t pos) throw(
    IOException)
{
  return ByteBufferIndexInput::readLong(pos + offset);
}

shared_ptr<ByteBufferIndexInput>
ByteBufferIndexInput::MultiBufferImpl::buildSlice(
    const wstring &sliceDescription, int64_t ofs, int64_t length)
{
  return ByteBufferIndexInput::buildSlice(sliceDescription, this->offset + ofs,
                                          length);
}
} // namespace org::apache::lucene::store