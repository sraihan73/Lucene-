using namespace std;

#include "FSLockFactory.h"

namespace org::apache::lucene::store
{

shared_ptr<FSLockFactory> FSLockFactory::getDefault()
{
  return NativeFSLockFactory::INSTANCE;
}

shared_ptr<Lock>
FSLockFactory::obtainLock(shared_ptr<Directory> dir,
                          const wstring &lockName) 
{
  if (!(std::dynamic_pointer_cast<FSDirectory>(dir) != nullptr)) {
    throw make_shared<UnsupportedOperationException>(
        getClass().getSimpleName() +
        L" can only be used with FSDirectory subclasses, got: " + dir);
  }
  return obtainFSLock(std::static_pointer_cast<FSDirectory>(dir), lockName);
}
} // namespace org::apache::lucene::store