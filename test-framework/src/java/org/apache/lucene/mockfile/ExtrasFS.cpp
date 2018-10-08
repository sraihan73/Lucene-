using namespace std;

#include "ExtrasFS.h"

namespace org::apache::lucene::mockfile
{

ExtrasFS::ExtrasFS(shared_ptr<FileSystem> delegate_, bool active,
                   bool createDirectory)
    : FilterFileSystemProvider(L"extras://", delegate_), active(active),
      createDirectory(createDirectory)
{
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public void createDirectory(java.nio.file.Path dir,
// java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
    void ExtrasFS::createDirectory(shared_ptr<Path> dir, deque<FileAttribute<?>> &attrs) 
    {
      FilterFileSystemProvider::createDirectory(dir, {attrs});
      // ok, we created the directory successfully.

      if (active) {
        // lets add a bogus file... if this fails, we don't care, its best
        // effort.
        try {
          shared_ptr<Path> target = dir->resolve(L"extra0");
          if (createDirectory_) {
            FilterFileSystemProvider::createDirectory(target);
          } else {
            Files::createFile(target);
          }
        } catch (const runtime_error &ignored) {
          // best effort
        }
      }
    }
    } // namespace org::apache::lucene::mockfile