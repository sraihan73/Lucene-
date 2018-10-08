using namespace std;

#include "TestMockDirectoryWrapper.h"

namespace org::apache::lucene::store
{
using Document = org::apache::lucene::document::Document;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;

shared_ptr<Directory>
TestMockDirectoryWrapper::getDirectory(shared_ptr<Path> path) 
{
  shared_ptr<MockDirectoryWrapper> *const dir;
  if (random()->nextBoolean()) {
    dir = newMockDirectory();
  } else {
    dir = newMockFSDirectory(path);
  }
  return dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Nightly public void testThreadSafety() throws
// Exception
void TestMockDirectoryWrapper::testThreadSafety() 
{
  BaseDirectoryTestCase::testThreadSafety();
}

void TestMockDirectoryWrapper::testDiskFull() 
{
  // test writeBytes
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->setMaxSizeInBytes(3);
  const std::deque<char> bytes = std::deque<char>{1, 2};
  shared_ptr<IndexOutput> out = dir->createOutput(L"foo", IOContext::DEFAULT);
  out->writeBytes(bytes, bytes.size()); // first write should succeed
  // close() to ensure the written bytes are not buffered and counted
  // against the directory size
  delete out;
  out = dir->createOutput(L"bar", IOContext::DEFAULT);
  try {
    out->writeBytes(bytes, bytes.size());
    fail(L"should have failed on disk full");
  } catch (const IOException &e) {
    // expected
  }
  delete out;
  delete dir;

  // test copyBytes
  dir = newMockDirectory();
  dir->setMaxSizeInBytes(3);
  out = dir->createOutput(L"foo", IOContext::DEFAULT);
  out->copyBytes(make_shared<ByteArrayDataInput>(bytes),
                 bytes.size()); // first copy should succeed
  // close() to ensure the written bytes are not buffered and counted
  // against the directory size
  delete out;
  out = dir->createOutput(L"bar", IOContext::DEFAULT);
  try {
    out->copyBytes(make_shared<ByteArrayDataInput>(bytes), bytes.size());
    fail(L"should have failed on disk full");
  } catch (const IOException &e) {
    // expected
  }
  delete out;
  delete dir;
}

void TestMockDirectoryWrapper::testMDWinsideOfMDW() 
{
  // add MDW inside another MDW
  shared_ptr<Directory> dir =
      make_shared<MockDirectoryWrapper>(random(), newMockDirectory());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  for (int i = 0; i < 20; i++) {
    iw->addDocument(make_shared<Document>());
  }
  iw->commit();
  delete iw;
  delete dir;
}

TestMockDirectoryWrapper::PreventCloseDirectoryWrapper::
    PreventCloseDirectoryWrapper(shared_ptr<Directory> in_)
    : FilterDirectory(in_)
{
}

TestMockDirectoryWrapper::PreventCloseDirectoryWrapper::
    ~PreventCloseDirectoryWrapper()
{
}

void TestMockDirectoryWrapper::testCorruptOnCloseIsWorkingFSDir() throw(
    runtime_error)
{
  shared_ptr<Path> path = createTempDir();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(Directory dir = newFSDirectory(path))
  {
    Directory dir = newFSDirectory(path);
    testCorruptOnCloseIsWorking(dir);
  }
}

void TestMockDirectoryWrapper::testCorruptOnCloseIsWorkingRAMDir() throw(
    runtime_error)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(Directory dir = new RAMDirectory())
  {
    Directory dir = RAMDirectory();
    testCorruptOnCloseIsWorking(dir);
  }
}

void TestMockDirectoryWrapper::testCorruptOnCloseIsWorking(
    shared_ptr<Directory> dir) 
{

  dir = make_shared<PreventCloseDirectoryWrapper>(dir);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (MockDirectoryWrapper wrapped = new
  // MockDirectoryWrapper(random(), dir))
  {
    MockDirectoryWrapper wrapped = MockDirectoryWrapper(random(), dir);

    // otherwise MDW sometimes randomly leaves the file intact and we'll see
    // false test failures:
    wrapped->alwaysCorrupt = true;

    // MDW will only try to corrupt things if it sees an index:
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    iw->addDocument(make_shared<Document>());
    delete iw;

    // not sync'd!
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (IndexOutput out =
    // wrapped.createOutput("foo", IOContext.DEFAULT))
    {
      IndexOutput out = wrapped->createOutput(L"foo", IOContext::DEFAULT);
      for (int i = 0; i < 100; i++) {
        out->writeInt(i);
      }
    }

    // MDW.close now corrupts our unsync'd file (foo):
  }

  bool changed = false;
  shared_ptr<IndexInput> in_ = nullptr;
  try {
    in_ = dir->openInput(L"foo", IOContext::DEFAULT);
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (NoSuchFileException | FileNotFoundException fnfe) {
    // ok
    changed = true;
  }
  if (in_ != nullptr) {
    for (int i = 0; i < 100; i++) {
      int x;
      try {
        x = in_->readInt();
      } catch (const EOFException &eofe) {
        changed = true;
        break;
      }
      if (x != i) {
        changed = true;
        break;
      }
    }

    delete in_;
  }

  assertTrue(L"MockDirectoryWrapper on dir=" + dir +
                 L" failed to corrupt an unsync'd file",
             changed);
}

void TestMockDirectoryWrapper::testAbuseClosedIndexInput() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<IndexOutput> out = dir->createOutput(L"foo", IOContext::DEFAULT);
  out->writeByte(static_cast<char>(42));
  delete out;
  shared_ptr<IndexInput> *const in_ =
      dir->openInput(L"foo", IOContext::DEFAULT);
  delete in_;
  expectThrows(runtime_error::typeid, in_::readByte);
  delete dir;
}

void TestMockDirectoryWrapper::testAbuseCloneAfterParentClosed() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<IndexOutput> out = dir->createOutput(L"foo", IOContext::DEFAULT);
  out->writeByte(static_cast<char>(42));
  delete out;
  shared_ptr<IndexInput> in_ = dir->openInput(L"foo", IOContext::DEFAULT);
  shared_ptr<IndexInput> *const clone = in_->clone();
  delete in_;
  expectThrows(runtime_error::typeid, clone::readByte);
  delete dir;
}

void TestMockDirectoryWrapper::testAbuseCloneOfCloneAfterParentClosed() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<IndexOutput> out = dir->createOutput(L"foo", IOContext::DEFAULT);
  out->writeByte(static_cast<char>(42));
  delete out;
  shared_ptr<IndexInput> in_ = dir->openInput(L"foo", IOContext::DEFAULT);
  shared_ptr<IndexInput> clone1 = in_->clone();
  shared_ptr<IndexInput> clone2 = clone1->clone();
  delete in_;
  expectThrows(runtime_error::typeid, clone2::readByte);
  delete dir;
}
} // namespace org::apache::lucene::store