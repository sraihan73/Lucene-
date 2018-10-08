using namespace std;

#include "BaseDirectory.h"

namespace org::apache::lucene::store
{

BaseDirectory::BaseDirectory(shared_ptr<LockFactory> lockFactory)
    : Directory(), lockFactory(lockFactory)
{
  if (lockFactory == nullptr) {
    throw make_shared<NullPointerException>(
        L"LockFactory must not be null, use an explicit instance!");
  }
}

shared_ptr<Lock>
BaseDirectory::obtainLock(const wstring &name) 
{
  return lockFactory->obtainLock(shared_from_this(), name);
}

void BaseDirectory::ensureOpen() 
{
  if (!isOpen) {
    throw make_shared<AlreadyClosedException>(L"this Directory is closed");
  }
}

wstring BaseDirectory::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Directory::toString() + L" lockFactory=" + lockFactory;
}
} // namespace org::apache::lucene::store