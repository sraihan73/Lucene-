using namespace std;

#include "MockIndexInputWrapper.h"

namespace org::apache::lucene::store
{

MockIndexInputWrapper::MockIndexInputWrapper(
    shared_ptr<MockDirectoryWrapper> dir, const wstring &name,
    shared_ptr<IndexInput> delegate_, shared_ptr<MockIndexInputWrapper> parent)
    : IndexInput(L"MockIndexInputWrapper(name=" + name + L" delegate=" +
                 delegate_ + L")"),
      name(name), parent(parent)
{

  // If we are a clone then our parent better not be a clone!
  assert(parent == nullptr || parent->parent == nullptr);

  this->dir = dir;
  this->delegate_ = delegate_;
}

MockIndexInputWrapper::~MockIndexInputWrapper()
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
    // Pending resolution on LUCENE-686 we may want to
    // remove the conditional check so we also track that
    // all clones get closed:
    assert(delegate_ != nullptr);
    if (parent == nullptr) {
      dir->removeIndexInput(shared_from_this(), name);
    }
    dir->maybeThrowDeterministicException();
  }
}

void MockIndexInputWrapper::ensureOpen()
{
  // TODO: not great this is a volatile read (closed) ... we should deploy heavy
  // JVM voodoo like SwitchPoint to avoid this
  if (closed) {
    throw runtime_error(L"Abusing closed IndexInput!");
  }
  if (parent != nullptr && parent->closed) {
    throw runtime_error(L"Abusing clone of a closed IndexInput!");
  }
}

shared_ptr<MockIndexInputWrapper> MockIndexInputWrapper::clone()
{
  ensureOpen();
  if (dir->verboseClone) {
    (runtime_error(L"clone: " + shared_from_this()))
        ->printStackTrace(System::out);
  }
  dir->inputCloneCount->incrementAndGet();
  shared_ptr<IndexInput> iiclone = delegate_->clone();
  shared_ptr<MockIndexInputWrapper> clone = make_shared<MockIndexInputWrapper>(
      dir, name, iiclone, parent != nullptr ? parent : shared_from_this());
  // Pending resolution on LUCENE-686 we may want to
  // uncomment this code so that we also track that all
  // clones get closed:
  /*
  synchronized(dir.openFiles) {
    if (dir.openFiles.containsKey(name)) {
      Integer v = (Integer) dir.openFiles.get(name);
      v = Integer.valueOf(v.intValue()+1);
      dir.openFiles.put(name, v);
    } else {
      throw new RuntimeException("BUG: cloned file was not open?");
    }
  }
  */
  return clone;
}

shared_ptr<IndexInput>
MockIndexInputWrapper::slice(const wstring &sliceDescription, int64_t offset,
                             int64_t length) 
{
  ensureOpen();
  if (dir->verboseClone) {
    (runtime_error(L"slice: " + shared_from_this()))
        ->printStackTrace(System::out);
  }
  dir->inputCloneCount->incrementAndGet();
  shared_ptr<IndexInput> slice =
      delegate_->slice(sliceDescription, offset, length);
  shared_ptr<MockIndexInputWrapper> clone = make_shared<MockIndexInputWrapper>(
      dir, sliceDescription, slice,
      parent != nullptr ? parent : shared_from_this());
  return clone;
}

int64_t MockIndexInputWrapper::getFilePointer()
{
  ensureOpen();
  return delegate_->getFilePointer();
}

void MockIndexInputWrapper::seek(int64_t pos) 
{
  ensureOpen();
  delegate_->seek(pos);
}

int64_t MockIndexInputWrapper::length()
{
  ensureOpen();
  return delegate_->length();
}

char MockIndexInputWrapper::readByte() 
{
  ensureOpen();
  return delegate_->readByte();
}

void MockIndexInputWrapper::readBytes(std::deque<char> &b, int offset,
                                      int len) 
{
  ensureOpen();
  delegate_->readBytes(b, offset, len);
}

void MockIndexInputWrapper::readBytes(std::deque<char> &b, int offset, int len,
                                      bool useBuffer) 
{
  ensureOpen();
  delegate_->readBytes(b, offset, len, useBuffer);
}

short MockIndexInputWrapper::readShort() 
{
  ensureOpen();
  return delegate_->readShort();
}

int MockIndexInputWrapper::readInt() 
{
  ensureOpen();
  return delegate_->readInt();
}

int64_t MockIndexInputWrapper::readLong() 
{
  ensureOpen();
  return delegate_->readLong();
}

wstring MockIndexInputWrapper::readString() 
{
  ensureOpen();
  return delegate_->readString();
}

int MockIndexInputWrapper::readVInt() 
{
  ensureOpen();
  return delegate_->readVInt();
}

int64_t MockIndexInputWrapper::readVLong() 
{
  ensureOpen();
  return delegate_->readVLong();
}

int MockIndexInputWrapper::readZInt() 
{
  ensureOpen();
  return delegate_->readZInt();
}

int64_t MockIndexInputWrapper::readZLong() 
{
  ensureOpen();
  return delegate_->readZLong();
}

void MockIndexInputWrapper::skipBytes(int64_t numBytes) 
{
  ensureOpen();
  IndexInput::skipBytes(numBytes);
}

unordered_map<wstring, wstring>
MockIndexInputWrapper::readMapOfStrings() 
{
  ensureOpen();
  return delegate_->readMapOfStrings();
}

shared_ptr<Set<wstring>>
MockIndexInputWrapper::readSetOfStrings() 
{
  ensureOpen();
  return delegate_->readSetOfStrings();
}

wstring MockIndexInputWrapper::toString()
{
  return L"MockIndexInputWrapper(" + delegate_ + L")";
}
} // namespace org::apache::lucene::store