using namespace std;

#include "RawDirectoryWrapper.h"

namespace org::apache::lucene::store
{

RawDirectoryWrapper::RawDirectoryWrapper(shared_ptr<Directory> delegate_)
    : BaseDirectoryWrapper(delegate_)
{
}

void RawDirectoryWrapper::copyFrom(
    shared_ptr<Directory> from, const wstring &src, const wstring &dest,
    shared_ptr<IOContext> context) 
{
  in_->copyFrom(from, src, dest, context);
}

shared_ptr<ChecksumIndexInput> RawDirectoryWrapper::openChecksumInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  return in_->openChecksumInput(name, context);
}

void RawDirectoryWrapper::ensureOpen() 
{
  in_->ensureOpen();
}
} // namespace org::apache::lucene::store