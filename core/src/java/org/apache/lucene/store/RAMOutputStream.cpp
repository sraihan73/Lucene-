using namespace std;

#include "RAMOutputStream.h"

namespace org::apache::lucene::store
{
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;

RAMOutputStream::RAMOutputStream()
    : RAMOutputStream(L"noname", new RAMFile(), false)
{
}

RAMOutputStream::RAMOutputStream(shared_ptr<RAMFile> f, bool checksum)
    : RAMOutputStream(L"noname", f, checksum)
{
}

RAMOutputStream::RAMOutputStream(const wstring &name, shared_ptr<RAMFile> f,
                                 bool checksum)
    : IndexOutput(L"RAMOutputStream(name=\"" + name + L"\")", name), file(f)
{

  // make sure that we switch to the
  // first needed buffer lazily
  currentBufferIndex = -1;
  currentBuffer.clear();
  if (checksum) {
    crc = make_shared<BufferedChecksum>(make_shared<CRC32>());
  } else {
    crc.reset();
  }
}

void RAMOutputStream::writeTo(shared_ptr<DataOutput> out) 
{
  flush();
  constexpr int64_t end = file->length;
  int64_t pos = 0;
  int buffer = 0;
  while (pos < end) {
    int length = BUFFER_SIZE;
    int64_t nextPos = pos + length;
    if (nextPos > end) { // at the last buffer
      length = static_cast<int>(end - pos);
    }
    out->writeBytes(file->getBuffer(buffer++), length);
    pos = nextPos;
  }
}

void RAMOutputStream::writeTo(std::deque<char> &bytes,
                              int offset) 
{
  flush();
  constexpr int64_t end = file->length;
  int64_t pos = 0;
  int buffer = 0;
  int bytesUpto = offset;
  while (pos < end) {
    int length = BUFFER_SIZE;
    int64_t nextPos = pos + length;
    if (nextPos > end) { // at the last buffer
      length = static_cast<int>(end - pos);
    }
    System::arraycopy(file->getBuffer(buffer++), 0, bytes, bytesUpto, length);
    bytesUpto += length;
    pos = nextPos;
  }
}

void RAMOutputStream::reset()
{
  currentBuffer.clear();
  currentBufferIndex = -1;
  bufferPosition = 0;
  bufferStart = 0;
  bufferLength = 0;
  file->setLength(0);
  if (crc != nullptr) {
    crc->reset();
  }
}

RAMOutputStream::~RAMOutputStream() { flush(); }

void RAMOutputStream::writeByte(char b) 
{
  if (bufferPosition == bufferLength) {
    currentBufferIndex++;
    switchCurrentBuffer();
  }
  if (crc != nullptr) {
    crc->update(b);
  }
  currentBuffer[bufferPosition++] = b;
}

void RAMOutputStream::writeBytes(std::deque<char> &b, int offset,
                                 int len) 
{
  assert(b.size() > 0);
  if (crc != nullptr) {
    crc->update(b, offset, len);
  }
  while (len > 0) {
    if (bufferPosition == bufferLength) {
      currentBufferIndex++;
      switchCurrentBuffer();
    }

    int remainInBuffer = currentBuffer.size() - bufferPosition;
    int bytesToCopy = len < remainInBuffer ? len : remainInBuffer;
    System::arraycopy(b, offset, currentBuffer, bufferPosition, bytesToCopy);
    offset += bytesToCopy;
    len -= bytesToCopy;
    bufferPosition += bytesToCopy;
  }
}

void RAMOutputStream::switchCurrentBuffer()
{
  if (currentBufferIndex == file->numBuffers()) {
    currentBuffer = file->addBuffer(BUFFER_SIZE);
  } else {
    currentBuffer = file->getBuffer(currentBufferIndex);
  }
  bufferPosition = 0;
  bufferStart = static_cast<int64_t>(BUFFER_SIZE) *
                static_cast<int64_t>(currentBufferIndex);
  bufferLength = currentBuffer.size();
}

void RAMOutputStream::setFileLength()
{
  int64_t pointer = bufferStart + bufferPosition;
  if (pointer > file->length) {
    file->setLength(pointer);
  }
}

void RAMOutputStream::flush()  { setFileLength(); }

int64_t RAMOutputStream::getFilePointer()
{
  return currentBufferIndex < 0 ? 0 : bufferStart + bufferPosition;
}

int64_t RAMOutputStream::ramBytesUsed()
{
  return static_cast<int64_t>(file->numBuffers()) *
         static_cast<int64_t>(BUFFER_SIZE);
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
RAMOutputStream::getChildResources()
{
  return Collections::singleton(Accountables::namedAccountable(L"file", file));
}

int64_t RAMOutputStream::getChecksum() 
{
  if (crc == nullptr) {
    throw make_shared<IllegalStateException>(
        L"internal RAMOutputStream created with checksum disabled");
  } else {
    return crc->getValue();
  }
}
} // namespace org::apache::lucene::store