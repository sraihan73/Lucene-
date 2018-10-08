using namespace std;

#include "TestMmapDirectory.h"

namespace org::apache::lucene::store
{
using org::junit::Ignore;

shared_ptr<Directory>
TestMmapDirectory::getDirectory(shared_ptr<Path> path) 
{
  shared_ptr<MMapDirectory> m = make_shared<MMapDirectory>(path);
  m->setPreload(random()->nextBoolean());
  return m;
}

void TestMmapDirectory::setUp() 
{
  BaseDirectoryTestCase::setUp();
  assumeTrue(MMapDirectory::UNMAP_NOT_SUPPORTED_REASON,
             MMapDirectory::UNMAP_SUPPORTED);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore("This test is for JVM testing purposes. There are no
// guarantees that it may not fail with SIGSEGV!") public void
// testAceWithThreads() throws Exception
void TestMmapDirectory::testAceWithThreads() 
{
  for (int iter = 0; iter < 10; iter++) {
    shared_ptr<Directory> dir =
        getDirectory(createTempDir(L"testAceWithThreads"));
    shared_ptr<IndexOutput> out =
        dir->createOutput(L"test", IOContext::DEFAULT);
    shared_ptr<Random> random = TestMmapDirectory::random();
    for (int i = 0; i < 8 * 1024 * 1024; i++) {
      out->writeInt(random->nextInt());
    }
    delete out;
    shared_ptr<IndexInput> in_ = dir->openInput(L"test", IOContext::DEFAULT);
    shared_ptr<IndexInput> clone = in_->clone();
    const std::deque<char> accum = std::deque<char>(32 * 1024 * 1024);
    shared_ptr<CountDownLatch> *const shotgun = make_shared<CountDownLatch>(1);
    shared_ptr<Thread> t1 = make_shared<Thread>([&]() {
      try {
        shotgun->await();
        for (int i = 0; i < 10; i++) {
          clone->seek(0);
          clone->readBytes(accum, 0, accum.length);
        }
      } catch (IOException | AlreadyClosedException ok) {
      } catch (const InterruptedException &e) {
        throw runtime_error(e);
      }
    });
    t1->start();
    shotgun->countDown();
    delete in_;
    t1->join();
    delete dir;
  }
}
} // namespace org::apache::lucene::store