using namespace std;

#include "NativePosixUtil.h"

namespace org::apache::lucene::store
{

NativePosixUtil::StaticConstructor::StaticConstructor()
{
  System::loadLibrary(L"NativePosixUtil");
}

NativePosixUtil::StaticConstructor NativePosixUtil::staticConstructor;

void NativePosixUtil::advise(shared_ptr<FileDescriptor> fd, int64_t offset,
                             int64_t len, int advise) 
{
  constexpr int code = posix_fadvise(fd, offset, len, advise);
  if (code != 0) {
    throw runtime_error(L"posix_fadvise failed code=" + to_wstring(code));
  }
}
} // namespace org::apache::lucene::store