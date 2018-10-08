using namespace std;

#include "BufferedIndexInput.h"

namespace org::apache::lucene::store
{

char BufferedIndexInput::readByte() 
{
  if (bufferPosition >= bufferLength) {
    refill();
  }
  return buffer[bufferPosition++];
}

BufferedIndexInput::BufferedIndexInput(const wstring &resourceDesc)
    : BufferedIndexInput(resourceDesc, BUFFER_SIZE)
{
}

BufferedIndexInput::BufferedIndexInput(const wstring &resourceDesc,
                                       shared_ptr<IOContext> context)
    : BufferedIndexInput(resourceDesc, bufferSize(context))
{
}

BufferedIndexInput::BufferedIndexInput(const wstring &resourceDesc,
                                       int bufferSize)
    : IndexInput(resourceDesc)
{
  checkBufferSize(bufferSize);
  this->bufferSize_ = bufferSize;
}

void BufferedIndexInput::setBufferSize(int newSize)
{
  assert((buffer.empty() || bufferSize_ == buffer.size(),
          L"buffer=" + buffer + L" bufferSize=" + to_wstring(bufferSize_) +
              L" buffer.length=" +
              to_wstring(buffer.size() > 0 ? buffer.size() : 0)));
  if (newSize != bufferSize_) {
    checkBufferSize(newSize);
    bufferSize_ = newSize;
    if (buffer.size() > 0) {
      // Resize the existing buffer and carefully save as
      // many bytes as possible starting from the current
      // bufferPosition
      std::deque<char> newBuffer(newSize);
      constexpr int leftInBuffer = bufferLength - bufferPosition;
      constexpr int numToCopy;
      if (leftInBuffer > newSize) {
        numToCopy = newSize;
      } else {
        numToCopy = leftInBuffer;
      }
      System::arraycopy(buffer, bufferPosition, newBuffer, 0, numToCopy);
      bufferStart += bufferPosition;
      bufferPosition = 0;
      bufferLength = numToCopy;
      this->newBuffer(newBuffer);
    }
  }
}

void BufferedIndexInput::newBuffer(std::deque<char> &newBuffer)
{
  // Subclasses can do something here
  buffer = newBuffer;
}

int BufferedIndexInput::getBufferSize() { return bufferSize_; }

void BufferedIndexInput::checkBufferSize(int bufferSize)
{
  if (bufferSize < MIN_BUFFER_SIZE) {
    throw invalid_argument(
        L"bufferSize must be at least MIN_BUFFER_SIZE (got " +
        to_wstring(bufferSize) + L")");
  }
}

void BufferedIndexInput::readBytes(std::deque<char> &b, int offset,
                                   int len) 
{
  readBytes(b, offset, len, true);
}

void BufferedIndexInput::readBytes(std::deque<char> &b, int offset, int len,
                                   bool useBuffer) 
{
  int available = bufferLength - bufferPosition;
  if (len <= available) {
    // the buffer contains enough data to satisfy this request
    if (len > 0) // to allow b to be null if len is 0...
    {
      System::arraycopy(buffer, bufferPosition, b, offset, len);
    }
    bufferPosition += len;
  } else {
    // the buffer does not have enough data. First serve all we've got.
    if (available > 0) {
      System::arraycopy(buffer, bufferPosition, b, offset, available);
      offset += available;
      len -= available;
      bufferPosition += available;
    }
    // and now, read the remaining 'len' bytes:
    if (useBuffer && len < bufferSize_) {
      // If the amount left to read is small enough, and
      // we are allowed to use our buffer, do it in the usual
      // buffered way: fill the buffer and copy from it:
      refill();
      if (bufferLength < len) {
        // Throw an exception when refill() could not read len bytes:
        System::arraycopy(buffer, 0, b, offset, bufferLength);
        // C++ TODO: The following line could not be converted:
        throw java.io.EOFException(L"read past EOF: " + this);
      } else {
        System::arraycopy(buffer, 0, b, offset, len);
        bufferPosition = len;
      }
    } else {
      // The amount left to read is larger than the buffer
      // or we've been asked to not use our buffer -
      // there's no performance reason not to read it all
      // at once. Note that unlike the previous code of
      // this function, there is no need to do a seek
      // here, because there's no need to reread what we
      // had in the buffer.
      int64_t after = bufferStart + bufferPosition + len;
      if (after > length()) {
        // C++ TODO: The following line could not be converted:
        throw java.io.EOFException(L"read past EOF: " + this);
      }
      readInternal(b, offset, len);
      bufferStart = after;
      bufferPosition = 0;
      bufferLength = 0; // trigger refill() on read
    }
  }
}

short BufferedIndexInput::readShort() 
{
  if (2 <= (bufferLength - bufferPosition)) {
    return static_cast<short>(((buffer[bufferPosition++] & 0xFF) << 8) |
                              (buffer[bufferPosition++] & 0xFF));
  } else {
    return IndexInput::readShort();
  }
}

int BufferedIndexInput::readInt() 
{
  if (4 <= (bufferLength - bufferPosition)) {
    return ((buffer[bufferPosition++] & 0xFF) << 24) |
           ((buffer[bufferPosition++] & 0xFF) << 16) |
           ((buffer[bufferPosition++] & 0xFF) << 8) |
           (buffer[bufferPosition++] & 0xFF);
  } else {
    return IndexInput::readInt();
  }
}

int64_t BufferedIndexInput::readLong() 
{
  if (8 <= (bufferLength - bufferPosition)) {
    constexpr int i1 = ((buffer[bufferPosition++] & 0xff) << 24) |
                       ((buffer[bufferPosition++] & 0xff) << 16) |
                       ((buffer[bufferPosition++] & 0xff) << 8) |
                       (buffer[bufferPosition++] & 0xff);
    constexpr int i2 = ((buffer[bufferPosition++] & 0xff) << 24) |
                       ((buffer[bufferPosition++] & 0xff) << 16) |
                       ((buffer[bufferPosition++] & 0xff) << 8) |
                       (buffer[bufferPosition++] & 0xff);
    return ((static_cast<int64_t>(i1)) << 32) | (i2 & 0xFFFFFFFFLL);
  } else {
    return IndexInput::readLong();
  }
}

int BufferedIndexInput::readVInt() 
{
  if (5 <= (bufferLength - bufferPosition)) {
    char b = buffer[bufferPosition++];
    if (b >= 0) {
      return b;
    }
    int i = b & 0x7F;
    b = buffer[bufferPosition++];
    i |= (b & 0x7F) << 7;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    i |= (b & 0x7F) << 14;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    i |= (b & 0x7F) << 21;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    // Warning: the next ands use 0x0F / 0xF0 - beware copy/paste errors:
    i |= (b & 0x0F) << 28;
    if ((b & 0xF0) == 0) {
      return i;
    }
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Invalid vInt detected (too many bits)");
  } else {
    return IndexInput::readVInt();
  }
}

int64_t BufferedIndexInput::readVLong() 
{
  if (9 <= bufferLength - bufferPosition) {
    char b = buffer[bufferPosition++];
    if (b >= 0) {
      return b;
    }
    int64_t i = b & 0x7FLL;
    b = buffer[bufferPosition++];
    i |= (b & 0x7FLL) << 7;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    i |= (b & 0x7FLL) << 14;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    i |= (b & 0x7FLL) << 21;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    i |= (b & 0x7FLL) << 28;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    i |= (b & 0x7FLL) << 35;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    i |= (b & 0x7FLL) << 42;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    i |= (b & 0x7FLL) << 49;
    if (b >= 0) {
      return i;
    }
    b = buffer[bufferPosition++];
    i |= (b & 0x7FLL) << 56;
    if (b >= 0) {
      return i;
    }
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Invalid vLong detected (negative values disallowed)");
  } else {
    return IndexInput::readVLong();
  }
}

char BufferedIndexInput::readByte(int64_t pos) 
{
  int64_t index = pos - bufferStart;
  if (index < 0 || index >= bufferLength) {
    bufferStart = pos;
    bufferPosition = 0;
    bufferLength = 0; // trigger refill() on read()
    seekInternal(pos);
    refill();
    index = 0;
  }
  return buffer[static_cast<int>(index)];
}

short BufferedIndexInput::readShort(int64_t pos) 
{
  int64_t index = pos - bufferStart;
  if (index < 0 || index >= bufferLength - 1) {
    bufferStart = pos;
    bufferPosition = 0;
    bufferLength = 0; // trigger refill() on read()
    seekInternal(pos);
    refill();
    index = 0;
  }
  return static_cast<short>(((buffer[static_cast<int>(index)] & 0xFF) << 8) |
                            (buffer[static_cast<int>(index) + 1] & 0xFF));
}

int BufferedIndexInput::readInt(int64_t pos) 
{
  int64_t index = pos - bufferStart;
  if (index < 0 || index >= bufferLength - 3) {
    bufferStart = pos;
    bufferPosition = 0;
    bufferLength = 0; // trigger refill() on read()
    seekInternal(pos);
    refill();
    index = 0;
  }
  return ((buffer[static_cast<int>(index)] & 0xFF) << 24) |
         ((buffer[static_cast<int>(index) + 1] & 0xFF) << 16) |
         ((buffer[static_cast<int>(index) + 2] & 0xFF) << 8) |
         (buffer[static_cast<int>(index) + 3] & 0xFF);
}

int64_t BufferedIndexInput::readLong(int64_t pos) 
{
  int64_t index = pos - bufferStart;
  if (index < 0 || index >= bufferLength - 7) {
    bufferStart = pos;
    bufferPosition = 0;
    bufferLength = 0; // trigger refill() on read()
    seekInternal(pos);
    refill();
    index = 0;
  }
  constexpr int i1 = ((buffer[static_cast<int>(index)] & 0xFF) << 24) |
                     ((buffer[static_cast<int>(index) + 1] & 0xFF) << 16) |
                     ((buffer[static_cast<int>(index) + 2] & 0xFF) << 8) |
                     (buffer[static_cast<int>(index) + 3] & 0xFF);
  constexpr int i2 = ((buffer[static_cast<int>(index) + 4] & 0xFF) << 24) |
                     ((buffer[static_cast<int>(index) + 5] & 0xFF) << 16) |
                     ((buffer[static_cast<int>(index) + 6] & 0xFF) << 8) |
                     (buffer[static_cast<int>(index) + 7] & 0xFF);
  return ((static_cast<int64_t>(i1)) << 32) | (i2 & 0xFFFFFFFFLL);
}

void BufferedIndexInput::refill() 
{
  int64_t start = bufferStart + bufferPosition;
  int64_t end = start + bufferSize_;
  if (end > length()) // don't read past EOF
  {
    end = length();
  }
  int newLength = static_cast<int>(end - start);
  if (newLength <= 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"read past EOF: " + this);
  }

  if (buffer.empty()) {
    newBuffer(std::deque<char>(bufferSize_)); // allocate buffer lazily
    seekInternal(bufferStart);
  }
  readInternal(buffer, 0, newLength);
  bufferLength = newLength;
  bufferStart = start;
  bufferPosition = 0;
}

int64_t BufferedIndexInput::getFilePointer()
{
  return bufferStart + bufferPosition;
}

void BufferedIndexInput::seek(int64_t pos) 
{
  if (pos >= bufferStart && pos < (bufferStart + bufferLength)) {
    bufferPosition = static_cast<int>(pos - bufferStart); // seek within buffer
  } else {
    bufferStart = pos;
    bufferPosition = 0;
    bufferLength = 0; // trigger refill() on read()
    seekInternal(pos);
  }
}

shared_ptr<BufferedIndexInput> BufferedIndexInput::clone()
{
  shared_ptr<BufferedIndexInput> clone =
      std::static_pointer_cast<BufferedIndexInput>(IndexInput::clone());

  clone->buffer.clear();
  clone->bufferLength = 0;
  clone->bufferPosition = 0;
  clone->bufferStart = getFilePointer();

  return clone;
}

shared_ptr<IndexInput>
BufferedIndexInput::slice(const wstring &sliceDescription, int64_t offset,
                          int64_t length) 
{
  return wrap(sliceDescription, shared_from_this(), offset, length);
}

int BufferedIndexInput::flushBuffer(shared_ptr<IndexOutput> out,
                                    int64_t numBytes) 
{
  int toCopy = bufferLength - bufferPosition;
  if (toCopy > numBytes) {
    toCopy = static_cast<int>(numBytes);
  }
  if (toCopy > 0) {
    out->writeBytes(buffer, bufferPosition, toCopy);
    bufferPosition += toCopy;
  }
  return toCopy;
}

int BufferedIndexInput::bufferSize(shared_ptr<IOContext> context)
{
  switch (context->context) {
  case org::apache::lucene::store::IOContext::Context::MERGE:
    return MERGE_BUFFER_SIZE;
  default:
    return BUFFER_SIZE;
  }
}

shared_ptr<BufferedIndexInput>
BufferedIndexInput::wrap(const wstring &sliceDescription,
                         shared_ptr<IndexInput> other, int64_t offset,
                         int64_t length)
{
  return make_shared<SlicedIndexInput>(sliceDescription, other, offset, length);
}

BufferedIndexInput::SlicedIndexInput::SlicedIndexInput(
    const wstring &sliceDescription, shared_ptr<IndexInput> base,
    int64_t offset, int64_t length)
    : BufferedIndexInput(
          (sliceDescription == nullptr)
              ? base->toString()
              : (base->toString() + L" [slice=" + sliceDescription + L"]"),
          BufferedIndexInput::BUFFER_SIZE)
{
  if (offset < 0 || length < 0 || offset + length > base->length()) {
    throw invalid_argument(L"slice() " + sliceDescription +
                           L" out of bounds: " + base);
  }
  this->base = base->clone();
  this->fileOffset = offset;
  this->length_ = length;
}

shared_ptr<SlicedIndexInput> BufferedIndexInput::SlicedIndexInput::clone()
{
  shared_ptr<SlicedIndexInput> clone =
      std::static_pointer_cast<SlicedIndexInput>(BufferedIndexInput::clone());
  clone->base = base->clone();
  clone->fileOffset = fileOffset;
  clone->length_ = length_;
  return clone;
}

void BufferedIndexInput::SlicedIndexInput::readInternal(
    std::deque<char> &b, int offset, int len) 
{
  int64_t start = getFilePointer();
  if (start + len > length_) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"read past EOF: " + this);
  }
  base->seek(fileOffset + start);
  base->readBytes(b, offset, len, false);
}

void BufferedIndexInput::SlicedIndexInput::seekInternal(int64_t pos) {}

BufferedIndexInput::SlicedIndexInput::~SlicedIndexInput() { delete base; }

int64_t BufferedIndexInput::SlicedIndexInput::length() { return length_; }
} // namespace org::apache::lucene::store