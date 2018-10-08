using namespace std;

#include "TestLeakFS.h"

namespace org::apache::lucene::mockfile
{

shared_ptr<Path> TestLeakFS::wrap(shared_ptr<Path> path)
{
  shared_ptr<FileSystem> fs = (make_shared<LeakFS>(path->getFileSystem()))
                                  ->getFileSystem(URI::create(L"file:///"));
  return make_shared<FilterPath>(path, fs);
}

void TestLeakFS::testLeakInputStream() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"stillopen"));
  file->write(5);
  file->close();
  shared_ptr<InputStream> leak =
      Files::newInputStream(dir->resolve(L"stillopen"));
  try {
    dir->getFileSystem()->close();
    fail(L"should have gotten exception");
  } catch (const runtime_error &e) {
    assertTrue(e.what()->contains(L"file handle leaks"));
  }
  leak->close();
}

void TestLeakFS::testLeakOutputStream() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> leak =
      Files::newOutputStream(dir->resolve(L"leaky"));
  try {
    dir->getFileSystem()->close();
    fail(L"should have gotten exception");
  } catch (const runtime_error &e) {
    assertTrue(e.what()->contains(L"file handle leaks"));
  }
  leak->close();
}

void TestLeakFS::testLeakFileChannel() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"stillopen"));
  file->write(5);
  file->close();
  shared_ptr<FileChannel> leak = FileChannel::open(dir->resolve(L"stillopen"));
  try {
    dir->getFileSystem()->close();
    fail(L"should have gotten exception");
  } catch (const runtime_error &e) {
    assertTrue(e.what()->contains(L"file handle leaks"));
  }
  leak->close();
}

void TestLeakFS::testLeakAsyncFileChannel() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"stillopen"));
  file->write(5);
  file->close();
  shared_ptr<AsynchronousFileChannel> leak =
      AsynchronousFileChannel::open(dir->resolve(L"stillopen"));
  try {
    dir->getFileSystem()->close();
    fail(L"should have gotten exception");
  } catch (const runtime_error &e) {
    assertTrue(e.what()->contains(L"file handle leaks"));
  }
  leak->close();
}

void TestLeakFS::testLeakByteChannel() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"stillopen"));
  file->write(5);
  file->close();
  shared_ptr<SeekableByteChannel> leak =
      Files::newByteChannel(dir->resolve(L"stillopen"));
  try {
    dir->getFileSystem()->close();
    fail(L"should have gotten exception");
  } catch (const runtime_error &e) {
    assertTrue(e.what()->contains(L"file handle leaks"));
  }
  leak->close();
}
} // namespace org::apache::lucene::mockfile