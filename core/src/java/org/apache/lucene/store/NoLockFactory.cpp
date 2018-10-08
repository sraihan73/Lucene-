using namespace std;

#include "NoLockFactory.h"

namespace org::apache::lucene::store
{

const shared_ptr<NoLockFactory> NoLockFactory::INSTANCE =
    make_shared<NoLockFactory>();
const shared_ptr<NoLock> NoLockFactory::SINGLETON_LOCK = make_shared<NoLock>();

NoLockFactory::NoLockFactory() {}

shared_ptr<Lock> NoLockFactory::obtainLock(shared_ptr<Directory> dir,
                                           const wstring &lockName)
{
  return SINGLETON_LOCK;
}

NoLockFactory::NoLock::~NoLock() {}

void NoLockFactory::NoLock::ensureValid()  {}

wstring NoLockFactory::NoLock::toString() { return L"NoLock"; }
} // namespace org::apache::lucene::store