using namespace std;

#include "TestSleepingLockWrapper.h"

namespace org::apache::lucene::store
{
using BaseLockFactoryTestCase =
    org::apache::lucene::store::BaseLockFactoryTestCase;
using Directory = org::apache::lucene::store::Directory;
using SingleInstanceLockFactory =
    org::apache::lucene::store::SingleInstanceLockFactory;
using SleepingLockWrapper = org::apache::lucene::store::SleepingLockWrapper;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Directory>
TestSleepingLockWrapper::getDirectory(shared_ptr<Path> path) 
{
  int64_t lockWaitTimeout = TestUtil::nextLong(random(), 20, 100);
  int64_t pollInterval = TestUtil::nextLong(random(), 2, 10);

  int which = random()->nextInt(3);
  switch (which) {
  case 0:
    return make_shared<SleepingLockWrapper>(
        newDirectory(random(), make_shared<SingleInstanceLockFactory>()),
        lockWaitTimeout, pollInterval);
  case 1:
    return make_shared<SleepingLockWrapper>(newFSDirectory(path),
                                            lockWaitTimeout, pollInterval);
  default:
    return make_shared<SleepingLockWrapper>(newFSDirectory(path),
                                            lockWaitTimeout, pollInterval);
  }
}
} // namespace org::apache::lucene::store