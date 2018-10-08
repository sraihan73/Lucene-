using namespace std;

#include "TestVerboseFS.h"

namespace org::apache::lucene::mockfile
{
using InfoStream = org::apache::lucene::util::InfoStream;

shared_ptr<Path> TestVerboseFS::wrap(shared_ptr<Path> path)
{
  return wrap(path, InfoStream::NO_OUTPUT);
}

shared_ptr<Path> TestVerboseFS::wrap(shared_ptr<Path> path,
                                     shared_ptr<InfoStream> stream)
{
  shared_ptr<FileSystem> fs =
      (make_shared<VerboseFS>(path->getFileSystem(), stream))
          ->getFileSystem(URI::create(L"file:///"));
  return make_shared<FilterPath>(path, fs);
}

TestVerboseFS::InfoStreamListener::InfoStreamListener(
    const wstring &messageStartsWith)
    : messageStartsWith(messageStartsWith)
{
}

TestVerboseFS::InfoStreamListener::~InfoStreamListener() {}

void TestVerboseFS::InfoStreamListener::message(const wstring &component,
                                                const wstring &message)
{
  if (L"FS" == component &&
      StringHelper::startsWith(message, messageStartsWith)) {
    seenMessage->set(true);
  }
}

bool TestVerboseFS::InfoStreamListener::isEnabled(const wstring &component)
{
  return true;
}

bool TestVerboseFS::InfoStreamListener::sawMessage()
{
  return seenMessage->get();
}

void TestVerboseFS::testCreateDirectory() 
{
  shared_ptr<InfoStreamListener> stream =
      make_shared<InfoStreamListener>(L"createDirectory");
  shared_ptr<Path> dir = wrap(createTempDir(), stream);
  Files::createDirectory(dir->resolve(L"subdir"));
  assertTrue(stream->sawMessage());

  try {
    Files::createDirectory(dir->resolve(L"subdir"));
    fail(L"didn't get expected exception");
  } catch (const IOException &expected) {
  }
}

void TestVerboseFS::testDelete() 
{
  shared_ptr<InfoStreamListener> stream =
      make_shared<InfoStreamListener>(L"delete");
  shared_ptr<Path> dir = wrap(createTempDir(), stream);
  Files::createFile(dir->resolve(L"foobar"));
  Files::delete (dir->resolve(L"foobar"));
  assertTrue(stream->sawMessage());

  try {
    Files::delete (dir->resolve(L"foobar"));
    fail(L"didn't get expected exception");
  } catch (const IOException &expected) {
  }
}

void TestVerboseFS::testDeleteIfExists() 
{
  shared_ptr<InfoStreamListener> stream =
      make_shared<InfoStreamListener>(L"deleteIfExists");
  shared_ptr<Path> dir = wrap(createTempDir(), stream);
  Files::createFile(dir->resolve(L"foobar"));
  Files::deleteIfExists(dir->resolve(L"foobar"));
  assertTrue(stream->sawMessage());

  // no exception
  Files::deleteIfExists(dir->resolve(L"foobar"));
}

void TestVerboseFS::testCopy() 
{
  shared_ptr<InfoStreamListener> stream =
      make_shared<InfoStreamListener>(L"copy");
  shared_ptr<Path> dir = wrap(createTempDir(), stream);
  Files::createFile(dir->resolve(L"foobar"));
  Files::copy(dir->resolve(L"foobar"), dir->resolve(L"baz"));
  assertTrue(stream->sawMessage());

  try {
    Files::copy(dir->resolve(L"nonexistent"), dir->resolve(L"something"));
    fail(L"didn't get expected exception");
  } catch (const IOException &expected) {
  }
}

void TestVerboseFS::testMove() 
{
  shared_ptr<InfoStreamListener> stream =
      make_shared<InfoStreamListener>(L"move");
  shared_ptr<Path> dir = wrap(createTempDir(), stream);
  Files::createFile(dir->resolve(L"foobar"));
  Files::move(dir->resolve(L"foobar"), dir->resolve(L"baz"));
  assertTrue(stream->sawMessage());

  try {
    Files::move(dir->resolve(L"nonexistent"), dir->resolve(L"something"));
    fail(L"didn't get expected exception");
  } catch (const IOException &expected) {
  }
}

void TestVerboseFS::testNewOutputStream() 
{
  shared_ptr<InfoStreamListener> stream =
      make_shared<InfoStreamListener>(L"newOutputStream");
  shared_ptr<Path> dir = wrap(createTempDir(), stream);
  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"output"));
  assertTrue(stream->sawMessage());
  file->close();

  try {
    Files::newOutputStream(dir->resolve(L"output"),
                           StandardOpenOption::CREATE_NEW);
    fail(L"didn't get expected exception");
  } catch (const IOException &expected) {
  }
}

void TestVerboseFS::testFileChannel() 
{
  shared_ptr<InfoStreamListener> stream =
      make_shared<InfoStreamListener>(L"newFileChannel");
  shared_ptr<Path> dir = wrap(createTempDir(), stream);
  shared_ptr<FileChannel> channel =
      FileChannel::open(dir->resolve(L"foobar"), StandardOpenOption::CREATE_NEW,
                        StandardOpenOption::READ, StandardOpenOption::WRITE);
  assertTrue(stream->sawMessage());
  channel->close();

  try {
    FileChannel::open(dir->resolve(L"foobar"), StandardOpenOption::CREATE_NEW,
                      StandardOpenOption::READ, StandardOpenOption::WRITE);
    fail(L"didn't get expected exception");
  } catch (const IOException &expected) {
  }
}

void TestVerboseFS::testAsyncFileChannel() 
{
  shared_ptr<InfoStreamListener> stream =
      make_shared<InfoStreamListener>(L"newAsynchronousFileChannel");
  shared_ptr<Path> dir = wrap(createTempDir(), stream);
  shared_ptr<AsynchronousFileChannel> channel = AsynchronousFileChannel::open(
      dir->resolve(L"foobar"), StandardOpenOption::CREATE_NEW,
      StandardOpenOption::READ, StandardOpenOption::WRITE);
  assertTrue(stream->sawMessage());
  channel->close();

  try {
    AsynchronousFileChannel::open(
        dir->resolve(L"foobar"), StandardOpenOption::CREATE_NEW,
        StandardOpenOption::READ, StandardOpenOption::WRITE);
    fail(L"didn't get expected exception");
  } catch (const IOException &expected) {
  }
}

void TestVerboseFS::testByteChannel() 
{
  shared_ptr<InfoStreamListener> stream =
      make_shared<InfoStreamListener>(L"newByteChannel");
  shared_ptr<Path> dir = wrap(createTempDir(), stream);
  shared_ptr<SeekableByteChannel> channel = Files::newByteChannel(
      dir->resolve(L"foobar"), StandardOpenOption::CREATE_NEW,
      StandardOpenOption::READ, StandardOpenOption::WRITE);
  assertTrue(stream->sawMessage());
  channel->close();

  try {
    Files::newByteChannel(dir->resolve(L"foobar"),
                          StandardOpenOption::CREATE_NEW,
                          StandardOpenOption::READ, StandardOpenOption::WRITE);
    fail(L"didn't get expected exception");
  } catch (const IOException &expected) {
  }
}

void TestVerboseFS::testVerboseFSNoSuchFileException() 
{
  shared_ptr<Path> dir = wrap(createTempDir());
  try {
    AsynchronousFileChannel::open(dir->resolve(L"doesNotExist.rip"));
    fail(L"did not hit exception");
  } catch (const NoSuchFileException &nsfe) {
    // expected
  }
  try {
    FileChannel::open(dir->resolve(L"doesNotExist.rip"));
    fail(L"did not hit exception");
  } catch (const NoSuchFileException &nsfe) {
    // expected
  }
  try {
    Files::newByteChannel(dir->resolve(L"stillopen"));
    fail(L"did not hit exception");
  } catch (const NoSuchFileException &nsfe) {
    // expected
  }
}
} // namespace org::apache::lucene::mockfile