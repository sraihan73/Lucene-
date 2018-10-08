using namespace std;

#include "SingleInstanceLockFactory.h"

namespace org::apache::lucene::store
{

shared_ptr<Lock> SingleInstanceLockFactory::obtainLock(
    shared_ptr<Directory> dir, const wstring &lockName) 
{
  {
    lock_guard<mutex> lock(locks);
    if (locks.insert(lockName)) {
      return make_shared<SingleInstanceLock>(shared_from_this(), lockName);
    } else {
      throw make_shared<LockObtainFailedException>(
          L"lock instance already obtained: (dir=" + dir + L", lockName=" +
          lockName + L")");
    }
  }
}

SingleInstanceLockFactory::SingleInstanceLock::SingleInstanceLock(
    shared_ptr<SingleInstanceLockFactory> outerInstance,
    const wstring &lockName)
    : lockName(lockName), outerInstance(outerInstance)
{
}

void SingleInstanceLockFactory::SingleInstanceLock::ensureValid() throw(
    IOException)
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(
        L"Lock instance already released: " + shared_from_this());
  }
  {
    // check we are still in the locks map_obj (some debugger or something crazy
    // didn't remove us)
    lock_guard<mutex> lock(outerInstance->locks);
    if (!find(locks.begin(), locks.end(), lockName) != locks.end()) {
      throw make_shared<AlreadyClosedException>(
          L"Lock instance was invalidated from map_obj: " + shared_from_this());
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
SingleInstanceLockFactory::SingleInstanceLock::~SingleInstanceLock()
{
  if (closed) {
    return;
  }
  try {
    {
      lock_guard<mutex> lock(outerInstance->locks);
      if (!outerInstance->locks.remove(lockName)) {
        throw make_shared<AlreadyClosedException>(
            L"Lock was already released: " + shared_from_this());
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    closed = true;
  }
}

wstring SingleInstanceLockFactory::SingleInstanceLock::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Lock::toString() + L": " + lockName;
}
} // namespace org::apache::lucene::store