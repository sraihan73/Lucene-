using namespace std;

#include "RAMInputStream.h"

namespace org::apache::lucene::store
{
//    import static org.apache.lucene.store.RAMOutputStream.BUFFER_SIZE;

RAMInputStream::RAMInputStream(const wstring &name,
                               shared_ptr<RAMFile> f) 
    : RAMInputStream(name, f, f->length)
{
}

RAMInputStream::RAMInputStream(const wstring &name, shared_ptr<RAMFile> f,
                               int64_t length) 
    : IndexInput(L"RAMInputStream(name=" + name + L")"), file(f), length(length)
{
  if (length / BUFFER_SIZE >= numeric_limits<int>::max()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"RAMInputStream too large length=" + length +
                              L": " + name);
  }

  setCurrentBuffer();
}

RAMInputStream::~RAMInputStream()
{
  // nothing to do here
}

int64_t RAMInputStream::length() { return length_; }

char RAMInputStream::readByte() 
{
  if (bufferPosition == bufferLength) {
    nextBuffer();
  }
  return currentBuffer[bufferPosition++];
}

void RAMInputStream::readBytes(std::deque<char> &b, int offset,
                               int len) 
{
  while (len > 0) {

    if (bufferPosition == bufferLength) {
      nextBuffer();
    }

    int remainInBuffer = bufferLength - bufferPosition;
    int bytesToCopy = len < remainInBuffer ? len : remainInBuffer;
    System::arraycopy(currentBuffer, bufferPosition, b, offset, bytesToCopy);
    offset += bytesToCopy;
    len -= bytesToCopy;
    bufferPosition += bytesToCopy;
  }
}

int64_t RAMInputStream::getFilePointer()
{
  return static_cast<int64_t>(currentBufferIndex) * BUFFER_SIZE +
         bufferPosition;
}

void RAMInputStream::seek(int64_t pos) 
{
  int newBufferIndex = static_cast<int>(pos / BUFFER_SIZE);

  if (newBufferIndex != currentBufferIndex) {
    // we seek'd to a different buffer:
    currentBufferIndex = newBufferIndex;
    setCurrentBuffer();
  }

  bufferPosition = static_cast<int>(pos % BUFFER_SIZE);

  // This is not >= because seeking to exact end of file is OK: this is where
  // you'd also be if you did a readBytes of all bytes in the file
  if (getFilePointer() > length()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"seek beyond EOF: pos=" + getFilePointer() +
                               L" vs length=" + length() + L": " + this);
  }
}

void RAMInputStream::nextBuffer() 
{
  // This is >= because we are called when there is at least 1 more byte to
  // read:
  if (getFilePointer() >= length()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"cannot read another byte at EOF: pos=" +
                               getFilePointer() + L" vs length=" + length() +
                               L": " + this);
  }
  currentBufferIndex++;
  setCurrentBuffer();
  assert(currentBuffer.size() > 0);
  bufferPosition = 0;
}

void RAMInputStream::setCurrentBuffer() 
{
  if (currentBufferIndex < file->numBuffers()) {
    currentBuffer = file->getBuffer(currentBufferIndex);
    assert(currentBuffer.size() > 0);
    int64_t bufferStart = static_cast<int64_t>(BUFFER_SIZE) *
                            static_cast<int64_t>(currentBufferIndex);
    bufferLength = static_cast<int>(min(BUFFER_SIZE, length_ - bufferStart));
  } else {
    currentBuffer.clear();
  }
}

shared_ptr<IndexInput>
RAMInputStream::slice(const wstring &sliceDescription, int64_t const offset,
                      int64_t const sliceLength) 
{
  if (offset < 0 || sliceLength < 0 || offset + sliceLength > this->length_) {
    throw invalid_argument(L"slice() " + sliceDescription +
                           L" out of bounds: " + shared_from_this());
  }
  return make_shared<RAMInputStreamAnonymousInnerClass>(
      shared_from_this(), getFullSliceDescription(sliceDescription), file,
      offset + sliceLength, sliceDescription, offset, sliceLength);
}

RAMInputStream::RAMInputStreamAnonymousInnerClass::
    RAMInputStreamAnonymousInnerClass(
        shared_ptr<RAMInputStream> outerInstance,
        const wstring &getFullSliceDescription,
        shared_ptr<org::apache::lucene::store::RAMFile> file, int64_t offset,
        const wstring &sliceDescription, int64_t offset,
        int64_t sliceLength)
    : RAMInputStream(getFullSliceDescription, file, offset + sliceLength)
{
  this->outerInstance = outerInstance;
  this->sliceDescription = sliceDescription;
  this->offset = offset;
  this->sliceLength = sliceLength;

  outerInstance->seek(0LL);
}

void RAMInputStream::RAMInputStreamAnonymousInnerClass::seek(
    int64_t pos) 
{
  if (pos < 0LL) {
    throw invalid_argument(L"Seeking to negative position: " +
                           shared_from_this());
  }
  outerInstance->super->seek(pos + offset);
}

int64_t RAMInputStream::RAMInputStreamAnonymousInnerClass::getFilePointer()
{
  return outerInstance->super->getFilePointer() - offset;
}

int64_t RAMInputStream::RAMInputStreamAnonymousInnerClass::length()
{
  return sliceLength;
}

shared_ptr<IndexInput> RAMInputStream::RAMInputStreamAnonymousInnerClass::slice(
    const wstring &sliceDescription, int64_t ofs,
    int64_t len) 
{
  return outerInstance->super->slice(sliceDescription, offset + ofs, len);
}
} // namespace org::apache::lucene::store