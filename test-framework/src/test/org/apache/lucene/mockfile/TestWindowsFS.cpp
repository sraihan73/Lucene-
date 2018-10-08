using namespace std;

#include "TestWindowsFS.h"

namespace org::apache::lucene::mockfile
{
using Constants = org::apache::lucene::util::Constants;

void TestWindowsFS::setUp() 
{
  MockFileSystemTestCase::setUp();
  // irony: currently we don't emulate windows well enough to work on windows!
  assumeFalse(L"windows is not supported", Constants::WINDOWS);
}

shared_ptr<Path> TestWindowsFS::wrap(shared_ptr<Path> path)
{
  shared_ptr<FileSystem> fs = (make_shared<WindowsFS>(path->getFileSystem()))
                                  ->getFileSystem(URI::create(L"file:///"));
  return make_shared<FilterPath>(path, fs);
}

void TestWindowsFS::testDeleteOpenFile() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"stillopen"));
  file->write(5);
  file->close();
  shared_ptr<InputStream> is =
      Files::newInputStream(dir->resolve(L"stillopen"));
  try {
    Files::delete (dir->resolve(L"stillopen"));
    fail(L"should have gotten exception");
  } catch (const IOException &e) {
    assertTrue(e->getMessage()->contains(L"access denied"));
  }
  is->close();
}

void TestWindowsFS::testDeleteIfExistsOpenFile() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"stillopen"));
  file->write(5);
  file->close();
  shared_ptr<InputStream> is =
      Files::newInputStream(dir->resolve(L"stillopen"));
  try {
    Files::deleteIfExists(dir->resolve(L"stillopen"));
    fail(L"should have gotten exception");
  } catch (const IOException &e) {
    assertTrue(e->getMessage()->contains(L"access denied"));
  }
  is->close();
}

void TestWindowsFS::testRenameOpenFile() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"stillopen"));
  file->write(5);
  file->close();
  shared_ptr<InputStream> is =
      Files::newInputStream(dir->resolve(L"stillopen"));
  try {
    Files::move(dir->resolve(L"stillopen"), dir->resolve(L"target"),
                StandardCopyOption::ATOMIC_MOVE);
    fail(L"should have gotten exception");
  } catch (const IOException &e) {
    assertTrue(e->getMessage()->contains(L"access denied"));
  }
  is->close();
}

void TestWindowsFS::testOpenDeleteConcurrently() throw(IOException,
                                                       runtime_error)
{
  shared_ptr<Path> *const dir = wrap(createTempDir());
  shared_ptr<Path> *const file = dir->resolve(L"thefile");
  shared_ptr<CyclicBarrier> *const barrier = make_shared<CyclicBarrier>(2);
  shared_ptr<AtomicBoolean> *const stopped = make_shared<AtomicBoolean>(false);
  shared_ptr<Thread> t = make_shared<ThreadAnonymousInnerClass>(
      shared_from_this(), file, barrier, stopped);
  t->start();
  barrier->await();
  try {
    constexpr int iters = 10 + random()->nextInt(100);
    for (int i = 0; i < iters; i++) {
      bool opened = false;
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (java.io.OutputStream stream =
      // java.nio.file.Files.newOutputStream(file))
      {
        java::io::OutputStream stream =
            java::nio::file::Files::newOutputStream(file);
        try {
          opened = true;
          stream.write(0);
          // just create
        }
        // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
        catch (FileNotFoundException | NoSuchFileException ex) {
          assertEquals(
              L"File handle leaked - file is closed but still registered", 0,
              (std::static_pointer_cast<WindowsFS>(
                   dir->getFileSystem().provider()))
                  ->openFiles.size());
          assertFalse(L"caught FNF on close", opened);
        }
      }
      assertEquals(
          L"File handle leaked - file is closed but still registered", 0,
          (std::static_pointer_cast<WindowsFS>(dir->getFileSystem().provider()))
              ->openFiles.size());
      Files::deleteIfExists(file);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    stopped->set(true);
    t->join();
  }
}

TestWindowsFS::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestWindowsFS> outerInstance, shared_ptr<Path> file,
    shared_ptr<CyclicBarrier> barrier, shared_ptr<AtomicBoolean> stopped)
{
  this->outerInstance = outerInstance;
  this->file = file;
  this->barrier = barrier;
  this->stopped = stopped;
}

void TestWindowsFS::ThreadAnonymousInnerClass::run()
{
  try {
    barrier->await();
  } catch (const runtime_error &ex) {
    throw runtime_error(ex);
  }
  while (stopped->get() == false) {
    try {
      if (random()->nextBoolean()) {
        Files::delete (file);
      } else if (random()->nextBoolean()) {
        Files::deleteIfExists(file);
      } else {
        shared_ptr<Path> target = file->resolveSibling(L"other");
        Files::move(file, target);
        Files::delete (target);
      }
    } catch (const IOException &ex) {
      // continue
    }
  }
}

void TestWindowsFS::testMove() 
{
  shared_ptr<Path> dir = wrap(createTempDir());
  shared_ptr<OutputStream> file = Files::newOutputStream(dir->resolve(L"file"));
  file->write(1);
  file->close();
  Files::move(dir->resolve(L"file"), dir->resolve(L"target"));
  assertTrue(Files::exists(dir->resolve(L"target")));
  assertFalse(Files::exists(dir->resolve(L"file")));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream stream =
  // java.nio.file.Files.newInputStream(dir.resolve("target")))
  {
    java::io::InputStream stream =
        java::nio::file::Files::newInputStream(dir->resolve(L"target"));
    assertEquals(1, stream.read());
  }
  file = Files::newOutputStream(dir->resolve(L"otherFile"));
  file->write(2);
  file->close();

  Files::move(dir->resolve(L"otherFile"), dir->resolve(L"target"),
              StandardCopyOption::REPLACE_EXISTING);
  assertTrue(Files::exists(dir->resolve(L"target")));
  assertFalse(Files::exists(dir->resolve(L"otherFile")));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream stream =
  // java.nio.file.Files.newInputStream(dir.resolve("target")))
  {
    java::io::InputStream stream =
        java::nio::file::Files::newInputStream(dir->resolve(L"target"));
    assertEquals(2, stream.read());
  }
}
} // namespace org::apache::lucene::mockfile