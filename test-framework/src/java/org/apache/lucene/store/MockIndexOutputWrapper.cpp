using namespace std;

#include "MockIndexOutputWrapper.h"

namespace org::apache::lucene::store
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

MockIndexOutputWrapper::MockIndexOutputWrapper(
    shared_ptr<MockDirectoryWrapper> dir, shared_ptr<IndexOutput> delegate_,
    const wstring &name)
    : IndexOutput(L"MockIndexOutputWrapper(" + delegate_ + L")",
                  delegate_->getName()),
      delegate_(delegate_), name(name)
{
  this->dir = dir;
}

void MockIndexOutputWrapper::checkCrashed() 
{
  // If MockRAMDir crashed since we were opened, then don't write anything
  if (dir->crashed) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"MockRAMDirectory was crashed; cannot write to " + name);
  }
}

void MockIndexOutputWrapper::checkDiskFull(std::deque<char> &b, int offset,
                                           shared_ptr<DataInput> in_,
                                           int64_t len) 
{
  int64_t freeSpace =
      dir->maxSize == 0 ? 0 : dir->maxSize - dir->sizeInBytes();
  int64_t realUsage = 0;

  // Enforce disk full:
  if (dir->maxSize != 0 && freeSpace <= len) {
    // Compute the real disk free.  This will greatly slow
    // down our test but makes it more accurate:
    realUsage = dir->getRecomputedActualSizeInBytes();
    freeSpace = dir->maxSize - realUsage;
  }

  if (dir->maxSize != 0 && freeSpace <= len) {
    if (freeSpace > 0) {
      realUsage += freeSpace;
      if (b.size() > 0) {
        delegate_->writeBytes(b, offset, static_cast<int>(freeSpace));
      } else {
        delegate_->copyBytes(in_, static_cast<int>(freeSpace));
      }
    }
    if (realUsage > dir->maxUsedSize) {
      dir->maxUsedSize = realUsage;
    }
    wstring message = L"fake disk full at " +
                      to_wstring(dir->getRecomputedActualSizeInBytes()) +
                      L" bytes when writing " + name + L" (file length=" +
                      to_wstring(delegate_->getFilePointer());
    if (freeSpace > 0) {
      message += L"; wrote " + to_wstring(freeSpace) + L" of " +
                 to_wstring(len) + L" bytes";
    }
    message += L")";
    if (LuceneTestCase::VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": MDW: now throw fake disk full" << endl;
      // C++ TODO: This exception's constructor requires an argument:
      // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
      (runtime_error())->printStackTrace(System::out);
    }
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(message);
  }
}

MockIndexOutputWrapper::~MockIndexOutputWrapper()
{
  if (closed) {
    delete delegate_; // don't mask double-close bugs
    return;
  }
  closed = true;

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.Closeable delegate = this.delegate)
  {
    java::io::Closeable delegate_ = this->delegate_;
    try {
      assert(delegate_ != nullptr);
      dir->maybeThrowDeterministicException();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      dir->removeIndexOutput(shared_from_this(), name);
      if (dir->trackDiskUsage) {
        // Now compute actual disk usage & track the maxUsedSize
        // in the MockDirectoryWrapper:
        int64_t size = dir->getRecomputedActualSizeInBytes();
        if (size > dir->maxUsedSize) {
          dir->maxUsedSize = size;
        }
      }
    }
  }
}

void MockIndexOutputWrapper::ensureOpen()
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(L"Already closed: " +
                                              shared_from_this());
  }
}

void MockIndexOutputWrapper::writeByte(char b) 
{
  singleByte[0] = b;
  writeBytes(singleByte, 0, 1);
}

void MockIndexOutputWrapper::writeBytes(std::deque<char> &b, int offset,
                                        int len) 
{
  ensureOpen();
  checkCrashed();
  checkDiskFull(b, offset, nullptr, len);

  if (dir->randomState->nextInt(200) == 0) {
    constexpr int half = len / 2;
    delegate_->writeBytes(b, offset, half);
    Thread::yield();
    delegate_->writeBytes(b, offset + half, len - half);
  } else {
    delegate_->writeBytes(b, offset, len);
  }

  dir->maybeThrowDeterministicException();

  if (first) {
    // Maybe throw random exception; only do this on first
    // write to a new file:
    first = false;
    dir->maybeThrowIOException(name);
  }
}

int64_t MockIndexOutputWrapper::getFilePointer()
{
  return delegate_->getFilePointer();
}

void MockIndexOutputWrapper::copyBytes(shared_ptr<DataInput> input,
                                       int64_t numBytes) 
{
  ensureOpen();
  checkCrashed();
  checkDiskFull(nullptr, 0, input, numBytes);

  delegate_->copyBytes(input, numBytes);
  dir->maybeThrowDeterministicException();
}

int64_t MockIndexOutputWrapper::getChecksum() 
{
  return delegate_->getChecksum();
}

wstring MockIndexOutputWrapper::toString()
{
  return L"MockIndexOutputWrapper(" + delegate_ + L")";
}
} // namespace org::apache::lucene::store