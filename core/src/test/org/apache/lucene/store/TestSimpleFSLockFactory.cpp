using namespace std;

#include "TestSimpleFSLockFactory.h"

namespace org::apache::lucene::store
{
using IOUtils = org::apache::lucene::util::IOUtils;

shared_ptr<Directory>
TestSimpleFSLockFactory::getDirectory(shared_ptr<Path> path) 
{
  return newFSDirectory(path, SimpleFSLockFactory::INSTANCE);
}

void TestSimpleFSLockFactory::testDeleteLockFile() 
{
  shared_ptr<Directory> dir = getDirectory(createTempDir());
  try {
    shared_ptr<Lock> lock = dir->obtainLock(L"test.lock");
    lock->ensureValid();

    try {
      dir->deleteFile(L"test.lock");
    } catch (const runtime_error &e) {
      // we can't delete a file for some reason, just clean up and assume the
      // test.
      IOUtils::closeWhileHandlingException({lock});
      assumeNoException(L"test requires the ability to delete a locked file",
                        e);
    }

    expectThrows(IOException::typeid, [&]() { lock->ensureValid(); });
    IOUtils::closeWhileHandlingException({lock});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // Do this in finally clause in case the assumeNoException is false:
    delete dir;
  }
}
} // namespace org::apache::lucene::store