using namespace std;

#include "SuppressingConcurrentMergeScheduler.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;

void SuppressingConcurrentMergeScheduler::handleMergeException(
    shared_ptr<Directory> dir, runtime_error exc)
{
  while (true) {
    if (isOK(exc)) {
      return;
    }
    exc = exc.getCause();
    if (exc == nullptr) {
      ConcurrentMergeScheduler::handleMergeException(dir, exc);
    }
  }
}
} // namespace org::apache::lucene::index