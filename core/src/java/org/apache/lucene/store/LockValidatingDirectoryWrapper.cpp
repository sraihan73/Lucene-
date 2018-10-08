using namespace std;

#include "LockValidatingDirectoryWrapper.h"

namespace org::apache::lucene::store
{

LockValidatingDirectoryWrapper::LockValidatingDirectoryWrapper(
    shared_ptr<Directory> in_, shared_ptr<Lock> writeLock)
    : FilterDirectory(in_), writeLock(writeLock)
{
}

void LockValidatingDirectoryWrapper::deleteFile(const wstring &name) throw(
    IOException)
{
  writeLock->ensureValid();
  in_->deleteFile(name);
}

shared_ptr<IndexOutput> LockValidatingDirectoryWrapper::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  writeLock->ensureValid();
  return in_->createOutput(name, context);
}

void LockValidatingDirectoryWrapper::copyFrom(
    shared_ptr<Directory> from, const wstring &src, const wstring &dest,
    shared_ptr<IOContext> context) 
{
  writeLock->ensureValid();
  in_->copyFrom(from, src, dest, context);
}

void LockValidatingDirectoryWrapper::rename(
    const wstring &source, const wstring &dest) 
{
  writeLock->ensureValid();
  in_->rename(source, dest);
}

void LockValidatingDirectoryWrapper::syncMetaData() 
{
  writeLock->ensureValid();
  in_->syncMetaData();
}

void LockValidatingDirectoryWrapper::sync(
    shared_ptr<deque<wstring>> names) 
{
  writeLock->ensureValid();
  in_->sync(names);
}
} // namespace org::apache::lucene::store