using namespace std;

#include "TestHandleTrackingFS.h"

namespace org::apache::lucene::mockfile
{
using HandleTrackingFS = org::apache::lucene::mockfile::HandleTrackingFS;
using LeakFS = org::apache::lucene::mockfile::LeakFS;

shared_ptr<Path> TestHandleTrackingFS::wrap(shared_ptr<Path> path)
{
  shared_ptr<FileSystem> fs = (make_shared<LeakFS>(path->getFileSystem()))
                                  ->getFileSystem(URI::create(L"file:///"));
  return make_shared<FilterPath>(path, fs);
}

void TestHandleTrackingFS::testOnCloseThrowsException() 
{
  shared_ptr<Path> path =
      wrap(createTempDir()); // we are using LeakFS under the hood if we don't
                             // get closed the test fails
  shared_ptr<FileSystem> fs =
      make_shared<HandleTrackingFSAnonymousInnerClass>(
          shared_from_this(), path->getFileSystem(), path)
          .getFileSystem(URI::create(L"file:///"));
  shared_ptr<Path> dir = make_shared<FilterPath>(path, fs);

  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"somefile"));
  file->write(5);
  try {
    file->close();
    fail(L"expected IOException");
  } catch (const IOException &ex) {
    // expected
  }

  shared_ptr<SeekableByteChannel> channel =
      Files::newByteChannel(dir->resolve(L"somefile"));
  try {
    channel->close();
    fail(L"expected IOException");
  } catch (const IOException &ex) {
    // expected
  }

  shared_ptr<InputStream> stream =
      Files::newInputStream(dir->resolve(L"somefile"));
  try {
    stream->close();
    fail(L"expected IOException");
  } catch (const IOException &ex) {
    // expected
  }
  fs->close();

  shared_ptr<DirectoryStream<std::shared_ptr<Path>>> dirStream =
      Files::newDirectoryStream(dir);
  try {
    dirStream->close();
    fail(L"expected IOException");
  } catch (const IOException &ex) {
    // expected
  }
}

TestHandleTrackingFS::HandleTrackingFSAnonymousInnerClass::
    HandleTrackingFSAnonymousInnerClass(
        shared_ptr<TestHandleTrackingFS> outerInstance,
        shared_ptr<UnknownType> getFileSystem, shared_ptr<Path> path)
    : org::apache::lucene::mockfile::HandleTrackingFS(L"test://", getFileSystem)
{
  this->outerInstance = outerInstance;
  this->path = path;
}

void TestHandleTrackingFS::HandleTrackingFSAnonymousInnerClass::onClose(
    shared_ptr<Path> path, any stream) 
{
  // C++ TODO: The following line could not be converted:
  throw java.io.IOException(L"boom");
}

void TestHandleTrackingFS::HandleTrackingFSAnonymousInnerClass::onOpen(
    shared_ptr<Path> path, any stream) 
{
  //
}

void TestHandleTrackingFS::testOnOpenThrowsException() 
{
  shared_ptr<Path> path =
      wrap(createTempDir()); // we are using LeakFS under the hood if we don't
                             // get closed the test fails
  shared_ptr<FileSystem> fs =
      make_shared<HandleTrackingFSAnonymousInnerClass2>(
          shared_from_this(), path->getFileSystem(), path)
          .getFileSystem(URI::create(L"file:///"));
  shared_ptr<Path> dir = make_shared<FilterPath>(path, fs);

  try {
    shared_ptr<OutputStream> file =
        Files::newOutputStream(dir->resolve(L"somefile"));
    fail(L"expected IOException");
  } catch (const IOException &ex) {
    // expected
  }

  try {
    shared_ptr<SeekableByteChannel> channel =
        Files::newByteChannel(dir->resolve(L"somefile"));
    fail(L"expected IOException");
  } catch (const IOException &ex) {
    // expected
  }

  try {
    shared_ptr<InputStream> stream =
        Files::newInputStream(dir->resolve(L"somefile"));
    fail(L"expected IOException");
  } catch (const IOException &ex) {
    // expected
  }
  fs->close();

  try {
    shared_ptr<DirectoryStream<std::shared_ptr<Path>>> dirStream =
        Files::newDirectoryStream(dir);
    fail(L"expected IOException");
  } catch (const IOException &ex) {
    // expected
  }
  fs->close();
}

TestHandleTrackingFS::HandleTrackingFSAnonymousInnerClass2::
    HandleTrackingFSAnonymousInnerClass2(
        shared_ptr<TestHandleTrackingFS> outerInstance,
        shared_ptr<UnknownType> getFileSystem, shared_ptr<Path> path)
    : org::apache::lucene::mockfile::HandleTrackingFS(L"test://", getFileSystem)
{
  this->outerInstance = outerInstance;
  this->path = path;
}

void TestHandleTrackingFS::HandleTrackingFSAnonymousInnerClass2::onClose(
    shared_ptr<Path> path, any stream) 
{
}

void TestHandleTrackingFS::HandleTrackingFSAnonymousInnerClass2::onOpen(
    shared_ptr<Path> path, any stream) 
{
  // C++ TODO: The following line could not be converted:
  throw java.io.IOException(L"boom");
}
} // namespace org::apache::lucene::mockfile