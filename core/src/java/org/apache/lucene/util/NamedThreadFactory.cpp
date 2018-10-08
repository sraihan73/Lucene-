using namespace std;

#include "NamedThreadFactory.h"

namespace org::apache::lucene::util
{

const shared_ptr<java::util::concurrent::atomic::AtomicInteger>
    NamedThreadFactory::threadPoolNumber =
        make_shared<java::util::concurrent::atomic::AtomicInteger>(1);
const wstring NamedThreadFactory::NAME_PATTERN = L"%s-%d-thread";

NamedThreadFactory::NamedThreadFactory(const wstring &threadNamePrefix)
    : group((s != nullptr) ? s::getThreadGroup()
                           : Thread::currentThread().getThreadGroup()),
      threadNamePrefix(wstring::format(Locale::ROOT, NAME_PATTERN,
                                       checkPrefix(threadNamePrefix),
                                       threadPoolNumber->getAndIncrement()))
{
  shared_ptr<SecurityManager> *const s = System::getSecurityManager();
}

wstring NamedThreadFactory::checkPrefix(const wstring &prefix)
{
  return prefix == L"" || prefix.length() == 0 ? L"Lucene" : prefix;
}

shared_ptr<Thread> NamedThreadFactory::newThread(Runnable r)
{
  shared_ptr<Thread> *const t = make_shared<Thread>(
      group, r,
      wstring::format(Locale::ROOT, L"%s-%d", this->threadNamePrefix,
                      threadNumber->getAndIncrement()),
      0);
  t->setDaemon(false);
  t->setPriority(Thread::NORM_PRIORITY);
  return t;
}
} // namespace org::apache::lucene::util