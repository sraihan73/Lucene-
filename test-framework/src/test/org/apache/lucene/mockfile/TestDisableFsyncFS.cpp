using namespace std;

#include "TestDisableFsyncFS.h"

namespace org::apache::lucene::mockfile
{

shared_ptr<Path> TestDisableFsyncFS::wrap(shared_ptr<Path> path)
{
  shared_ptr<FileSystem> fs =
      (make_shared<DisableFsyncFS>(path->getFileSystem()))
          ->getFileSystem(URI::create(L"file:///"));
  return make_shared<FilterPath>(path, fs);
}

void TestDisableFsyncFS::testFsyncWorks() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<FileChannel> file =
      FileChannel::open(dir->resolve(L"file"), StandardOpenOption::CREATE_NEW,
                        StandardOpenOption::READ, StandardOpenOption::WRITE);
  std::deque<char> bytes(128);
  random()->nextBytes(bytes);
  file->write(ByteBuffer::wrap(bytes));
  file->force(true);
  file->close();
}
} // namespace org::apache::lucene::mockfile