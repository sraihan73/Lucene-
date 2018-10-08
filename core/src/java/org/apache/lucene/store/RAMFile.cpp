using namespace std;

#include "RAMFile.h"

namespace org::apache::lucene::store
{
using Accountable = org::apache::lucene::util::Accountable;

RAMFile::RAMFile() {}

RAMFile::RAMFile(shared_ptr<RAMDirectory> directory)
{
  this->directory = directory;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t RAMFile::getLength() { return length; }

// C++ WARNING: The following method was originally marked 'synchronized':
void RAMFile::setLength(int64_t length) { this->length = length; }

std::deque<char> RAMFile::addBuffer(int size)
{
  std::deque<char> buffer = newBuffer(size);
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    buffers.push_back(buffer);
    sizeInBytes += size;
  }

  if (directory != nullptr) {
    directory->sizeInBytes->getAndAdd(size);
  }
  return buffer;
}

// C++ WARNING: The following method was originally marked 'synchronized':
std::deque<char> RAMFile::getBuffer(int index) { return buffers[index]; }

// C++ WARNING: The following method was originally marked 'synchronized':
int RAMFile::numBuffers() { return buffers.size(); }

std::deque<char> RAMFile::newBuffer(int size)
{
  return std::deque<char>(size);
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t RAMFile::ramBytesUsed() { return sizeInBytes; }

wstring RAMFile::toString()
{
  return getClass().getSimpleName() + L"(length=" + to_wstring(length) + L")";
}

int RAMFile::hashCode()
{
  int h = static_cast<int>(
      length ^
      (static_cast<int64_t>(static_cast<uint64_t>(length) >> 32)));
  for (auto block : buffers) {
    h = 31 * h + Arrays::hashCode(block);
  }
  return h;
}

bool RAMFile::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<RAMFile> other = any_cast<std::shared_ptr<RAMFile>>(obj);
  if (length != other->length) {
    return false;
  }
  if (buffers.size() != other->buffers.size()) {
    return false;
  }
  for (int i = 0; i < buffers.size(); i++) {
    if (!Arrays::equals(buffers[i], other->buffers[i])) {
      return false;
    }
  }
  return true;
}
} // namespace org::apache::lucene::store