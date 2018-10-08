using namespace std;

#include "TestHandleLimitFS.h"

namespace org::apache::lucene::mockfile
{
using IOUtils = org::apache::lucene::util::IOUtils;

shared_ptr<Path> TestHandleLimitFS::wrap(shared_ptr<Path> path)
{
  return wrap(path, 4096);
}

shared_ptr<Path> TestHandleLimitFS::wrap(shared_ptr<Path> path, int limit)
{
  shared_ptr<FileSystem> fs =
      (make_shared<HandleLimitFS>(path->getFileSystem(), limit))
          ->getFileSystem(URI::create(L"file:///"));
  return make_shared<FilterPath>(path, fs);
}

void TestHandleLimitFS::testTooManyOpenFiles() 
{
  int n = 60;

  shared_ptr<Path> dir = wrap(createTempDir(), n);

  // create open files to exact limit
  deque<std::shared_ptr<Closeable>> toClose =
      deque<std::shared_ptr<Closeable>>();
  for (int i = 0; i < n; i++) {
    shared_ptr<Path> p = Files::createTempFile(dir, nullptr, nullptr);
    toClose.push_back(Files::newOutputStream(p));
  }

  // now exceed
  try {
    Files::newOutputStream(Files::createTempFile(dir, nullptr, nullptr));
    fail(L"didn't hit exception");
  } catch (const IOException &e) {
    assertTrue(e->getMessage()->contains(L"Too many open files"));
  }

  IOUtils::close(toClose);
}
} // namespace org::apache::lucene::mockfile