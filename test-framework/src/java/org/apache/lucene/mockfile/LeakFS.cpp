using namespace std;

#include "LeakFS.h"

namespace org::apache::lucene::mockfile
{

LeakFS::LeakFS(shared_ptr<FileSystem> delegate_)
    : HandleTrackingFS(L"leakfs://", delegate_)
{
}

void LeakFS::onOpen(shared_ptr<Path> path, any stream)
{
  // C++ TODO: This exception's constructor requires an argument:
  // ORIGINAL LINE: openHandles.put(stream, new Exception());
  openHandles.emplace(stream, runtime_error());
}

void LeakFS::onClose(shared_ptr<Path> path, any stream)
{
  openHandles.erase(stream);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void LeakFS::onClose()
{
  if (!openHandles.empty()) {
    // print the first one as it's very verbose otherwise
    runtime_error cause = nullptr;
    Iterator<runtime_error> stacktraces = openHandles.values().begin();
    if (stacktraces->hasNext()) {
      cause = stacktraces->next();
    }
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("file handle leaks: " +
    // openHandles.keySet(), cause);
    throw runtime_error(L"file handle leaks: " + openHandles.keySet());
  }
}
} // namespace org::apache::lucene::mockfile