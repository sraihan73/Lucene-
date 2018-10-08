using namespace std;

#include "TestNRTThreads.h"

namespace org::apache::lucene::index
{
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::junit::Before;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void setUp() throws Exception
void TestNRTThreads::setUp() 
{
  ThreadedIndexingAndSearchingTestCase::setUp();
  useNonNrtReaders = random()->nextBoolean();
}

void TestNRTThreads::doSearching(shared_ptr<ExecutorService> es,
                                 int64_t stopTime) 
{

  bool anyOpenDelFiles = false;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(writer);

  while (System::currentTimeMillis() < stopTime && !failed->get()) {
    if (random()->nextBoolean()) {
      if (VERBOSE) {
        wcout << L"TEST: now reopen r=" << r << endl;
      }
      shared_ptr<DirectoryReader> *const r2 = DirectoryReader::openIfChanged(r);
      if (r2 != nullptr) {
        r->close();
        r = r2;
      }
    } else {
      if (VERBOSE) {
        wcout << L"TEST: now close reader=" << r << endl;
      }
      r->close();
      writer->commit();
      shared_ptr<Set<wstring>> *const openDeletedFiles =
          (std::static_pointer_cast<MockDirectoryWrapper>(dir))
              ->getOpenDeletedFiles();
      if (openDeletedFiles->size() > 0) {
        wcout << L"OBD files: " << openDeletedFiles << endl;
      }
      anyOpenDelFiles |= openDeletedFiles->size() > 0;
      // assertEquals("open but deleted: " + openDeletedFiles, 0,
      // openDeletedFiles.size());
      if (VERBOSE) {
        wcout << L"TEST: now open" << endl;
      }
      r = DirectoryReader::open(writer);
    }
    if (VERBOSE) {
      wcout << L"TEST: got new reader=" << r << endl;
    }
    // System.out.println("numDocs=" + r.numDocs() + "
    // openDelFileCount=" + dir.openDeleteFileCount());

    if (r->numDocs() > 0) {
      fixedSearcher = make_shared<IndexSearcher>(r, es);
      smokeTestSearcher(fixedSearcher);
      runSearchThreads(System::currentTimeMillis() + 500);
    }
  }
  r->close();

  // System.out.println("numDocs=" + r.numDocs() + " openDelFileCount=" +
  // dir.openDeleteFileCount());
  shared_ptr<Set<wstring>> *const openDeletedFiles =
      (std::static_pointer_cast<MockDirectoryWrapper>(dir))
          ->getOpenDeletedFiles();
  if (openDeletedFiles->size() > 0) {
    wcout << L"OBD files: " << openDeletedFiles << endl;
  }
  anyOpenDelFiles |= openDeletedFiles->size() > 0;

  assertFalse(L"saw non-zero open-but-deleted count", anyOpenDelFiles);
}

shared_ptr<Directory> TestNRTThreads::getDirectory(shared_ptr<Directory> in_)
{
  assert(std::dynamic_pointer_cast<MockDirectoryWrapper>(in_) != nullptr);
  if (!useNonNrtReaders) {
    (std::static_pointer_cast<MockDirectoryWrapper>(in_))
        ->setAssertNoDeleteOpenFile(true);
  }
  return in_;
}

void TestNRTThreads::doAfterWriter(shared_ptr<ExecutorService> es) throw(
    runtime_error)
{
  // Force writer to do reader pooling, always, so that
  // all merged segments, even for merges before
  // doSearching is called, are warmed:
  writer->getReader()->close();
}

shared_ptr<IndexSearcher>
TestNRTThreads::getCurrentSearcher() 
{
  return fixedSearcher;
}

void TestNRTThreads::releaseSearcher(shared_ptr<IndexSearcher> s) throw(
    runtime_error)
{
  if (s != fixedSearcher) {
    // Final searcher:
    delete s->getIndexReader();
  }
}

shared_ptr<IndexSearcher>
TestNRTThreads::getFinalSearcher() 
{
  shared_ptr<IndexReader> *const r2;
  if (useNonNrtReaders) {
    if (random()->nextBoolean()) {
      r2 = writer->getReader();
    } else {
      writer->commit();
      r2 = DirectoryReader::open(dir);
    }
  } else {
    r2 = writer->getReader();
  }
  return newSearcher(r2);
}

void TestNRTThreads::testNRTThreads() 
{
  runTest(L"TestNRTThreads");
}
} // namespace org::apache::lucene::index