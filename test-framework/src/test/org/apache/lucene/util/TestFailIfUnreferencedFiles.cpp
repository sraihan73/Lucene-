using namespace std;

#include "TestFailIfUnreferencedFiles.h"

namespace org::apache::lucene::util
{
using Document = org::apache::lucene::document::Document;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using com::carrotsearch::randomizedtesting::RandomizedTest;
using org::junit::Assert;
using org::junit::Test;
using org::junit::runner::JUnitCore;
using org::junit::runner::Result;
using org::junit::runner::notification::Failure;

TestFailIfUnreferencedFiles::TestFailIfUnreferencedFiles()
    : WithNestedTests(true)
{
}

void TestFailIfUnreferencedFiles::Nested1::testDummy() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->setAssertNoUnrefencedFilesOnClose(true);
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  iw->addDocument(make_shared<Document>());
  delete iw;
  shared_ptr<IndexOutput> output =
      dir->createOutput(L"_hello.world", IOContext::DEFAULT);
  output->writeString(L"i am unreferenced!");
  delete output;
  dir->sync(Collections::singleton(L"_hello.world"));
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailIfUnreferencedFiles()
void TestFailIfUnreferencedFiles::testFailIfUnreferencedFiles()
{
  shared_ptr<Result> r = JUnitCore::runClasses(Nested1::typeid);
  RandomizedTest::assumeTrue(
      L"Ignoring nested test, very likely zombie threads present.",
      r->getIgnoreCount() == 0);

  // We are suppressing output anyway so dump the failures.
  for (shared_ptr<Failure> f : r->getFailures()) {
    wcout << f->getTrace() << endl;
  }

  Assert::assertEquals(L"Expected exactly one failure.", 1,
                       r->getFailureCount());
  Assert::assertTrue(
      L"Expected unreferenced files assertion.",
      r->getFailures()->get(0).getTrace()->contains(L"unreferenced files:"));
}
} // namespace org::apache::lucene::util