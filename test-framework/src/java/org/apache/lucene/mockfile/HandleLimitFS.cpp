using namespace std;

#include "HandleLimitFS.h"

namespace org::apache::lucene::mockfile
{

HandleLimitFS::HandleLimitFS(shared_ptr<FileSystem> delegate_, int limit)
    : HandleTrackingFS(L"handlelimit://", delegate_), limit(limit)
{
}

void HandleLimitFS::onOpen(shared_ptr<Path> path, any stream) 
{
  if (count->incrementAndGet() > limit) {
    count->decrementAndGet();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<FileSystemException>(path->toString(), nullptr,
                                           L"Too many open files");
  }
}

void HandleLimitFS::onClose(shared_ptr<Path> path,
                            any stream) 
{
  count->decrementAndGet();
}
} // namespace org::apache::lucene::mockfile