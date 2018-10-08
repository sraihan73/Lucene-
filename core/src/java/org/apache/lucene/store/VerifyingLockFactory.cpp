using namespace std;

#include "VerifyingLockFactory.h"

namespace org::apache::lucene::store
{

VerifyingLockFactory::CheckedLock::CheckedLock(
    shared_ptr<VerifyingLockFactory> outerInstance,
    shared_ptr<Lock> lock) 
    : lock(lock), outerInstance(outerInstance)
{
  verify(static_cast<char>(1));
}

void VerifyingLockFactory::CheckedLock::ensureValid() 
{
  lock->ensureValid();
}

VerifyingLockFactory::CheckedLock::~CheckedLock()
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Lock l = lock)
  {
    Lock l = lock;
    l->ensureValid();
    verify(static_cast<char>(0));
  }
}

void VerifyingLockFactory::CheckedLock::verify(char message) 
{
  outerInstance->out->write(message);
  outerInstance->out->flush();
  constexpr int ret = outerInstance->in_->read();
  if (ret < 0) {
    throw make_shared<IllegalStateException>(
        L"Lock server died because of locking error.");
  }
  if (ret != message) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Protocol violation.");
  }
}

VerifyingLockFactory::VerifyingLockFactory(
    shared_ptr<LockFactory> lf, shared_ptr<InputStream> in_,
    shared_ptr<OutputStream> out) 
    : lf(lf), in_(in_), out(out)
{
}

shared_ptr<Lock>
VerifyingLockFactory::obtainLock(shared_ptr<Directory> dir,
                                 const wstring &lockName) 
{
  return make_shared<CheckedLock>(shared_from_this(),
                                  lf->obtainLock(dir, lockName));
}
} // namespace org::apache::lucene::store