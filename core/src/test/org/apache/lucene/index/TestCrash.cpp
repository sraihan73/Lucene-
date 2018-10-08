using namespace std;

#include "TestCrash.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using NoLockFactory = org::apache::lucene::store::NoLockFactory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

shared_ptr<IndexWriter>
TestCrash::initIndex(shared_ptr<Random> random,
                     bool initialCommit) 
{
  return initIndex(random, newMockDirectory(random, NoLockFactory::INSTANCE),
                   initialCommit, true);
}

shared_ptr<IndexWriter>
TestCrash::initIndex(shared_ptr<Random> random,
                     shared_ptr<MockDirectoryWrapper> dir, bool initialCommit,
                     bool commitOnClose) 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
               ->setMaxBufferedDocs(10)
               ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
               ->setCommitOnClose(commitOnClose));
  (std::static_pointer_cast<ConcurrentMergeScheduler>(
       writer->getConfig()->getMergeScheduler()))
      ->setSuppressExceptions();
  if (initialCommit) {
    writer->commit();
  }

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  doc->push_back(newTextField(L"id", L"0", Field::Store::NO));
  for (int i = 0; i < 157; i++) {
    writer->addDocument(doc);
  }

  return writer;
}

void TestCrash::crash(shared_ptr<IndexWriter> writer) 
{
  shared_ptr<MockDirectoryWrapper> *const dir =
      std::static_pointer_cast<MockDirectoryWrapper>(writer->getDirectory());
  shared_ptr<ConcurrentMergeScheduler> cms =
      std::static_pointer_cast<ConcurrentMergeScheduler>(
          writer->getConfig()->getMergeScheduler());
  cms->sync();
  dir->crash();
  cms->sync();
  dir->clearCrash();
}

void TestCrash::testCrashWhileIndexing() 
{
  // This test relies on being able to open a reader before any commit
  // happened, so we must create an initial commit just to allow that, but
  // before any documents were added.
  shared_ptr<IndexWriter> writer = initIndex(random(), true);
  shared_ptr<MockDirectoryWrapper> dir =
      std::static_pointer_cast<MockDirectoryWrapper>(writer->getDirectory());

  // We create leftover files because merging could be
  // running when we crash:
  dir->setAssertNoUnrefencedFilesOnClose(false);

  crash(writer);

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  assertTrue(reader->numDocs() < 157);
  delete reader;

  // Make a new dir, copying from the crashed dir, and
  // open IW on it, to confirm IW "recovers" after a
  // crash:
  shared_ptr<Directory> dir2 = newDirectory(dir);
  delete dir;

  delete (make_shared<RandomIndexWriter>(random(), dir2));
  delete dir2;
}

void TestCrash::testWriterAfterCrash() 
{
  // This test relies on being able to open a reader before any commit
  // happened, so we must create an initial commit just to allow that, but
  // before any documents were added.
  if (VERBOSE) {
    wcout << L"TEST: initIndex" << endl;
  }
  shared_ptr<IndexWriter> writer = initIndex(random(), true);
  if (VERBOSE) {
    wcout << L"TEST: done initIndex" << endl;
  }
  shared_ptr<MockDirectoryWrapper> dir =
      std::static_pointer_cast<MockDirectoryWrapper>(writer->getDirectory());

  // We create leftover files because merging could be
  // running / store files could be open when we crash:
  dir->setAssertNoUnrefencedFilesOnClose(false);

  if (VERBOSE) {
    wcout << L"TEST: now crash" << endl;
  }
  crash(writer);
  writer = initIndex(random(), dir, false, true);
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  assertTrue(reader->numDocs() < 314);
  delete reader;

  // Make a new dir, copying from the crashed dir, and
  // open IW on it, to confirm IW "recovers" after a
  // crash:
  shared_ptr<Directory> dir2 = newDirectory(dir);
  delete dir;

  delete (make_shared<RandomIndexWriter>(random(), dir2));
  delete dir2;
}

void TestCrash::testCrashAfterReopen() 
{
  shared_ptr<IndexWriter> writer = initIndex(random(), false);
  shared_ptr<MockDirectoryWrapper> dir =
      std::static_pointer_cast<MockDirectoryWrapper>(writer->getDirectory());

  // We create leftover files because merging could be
  // running when we crash:
  dir->setAssertNoUnrefencedFilesOnClose(false);

  delete writer;
  writer = initIndex(random(), dir, false, true);
  assertEquals(314, writer->maxDoc());
  crash(writer);

  /*
  System.out.println("\n\nTEST: open reader");
  std::wstring[] l = dir.deque();
  Arrays.sort(l);
  for(int i=0;i<l.length;i++)
    System.out.println("file " + i + " = " + l[i] + " " +
  dir.fileLength(l[i]) + " bytes");
  */

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  assertTrue(reader->numDocs() >= 157);
  delete reader;

  // Make a new dir, copying from the crashed dir, and
  // open IW on it, to confirm IW "recovers" after a
  // crash:
  shared_ptr<Directory> dir2 = newDirectory(dir);
  delete dir;

  delete (make_shared<RandomIndexWriter>(random(), dir2));
  delete dir2;
}

void TestCrash::testCrashAfterClose() 
{

  shared_ptr<IndexWriter> writer = initIndex(random(), false);
  shared_ptr<MockDirectoryWrapper> dir =
      std::static_pointer_cast<MockDirectoryWrapper>(writer->getDirectory());

  delete writer;
  dir->crash();

  /*
  std::wstring[] l = dir.deque();
  Arrays.sort(l);
  for(int i=0;i<l.length;i++)
    System.out.println("file " + i + " = " + l[i] + " " + dir.fileLength(l[i]) +
  " bytes");
  */

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  assertEquals(157, reader->numDocs());
  delete reader;
  delete dir;
}

void TestCrash::testCrashAfterCloseNoWait() 
{
  shared_ptr<Random> random = TestCrash::random();
  shared_ptr<MockDirectoryWrapper> dir =
      newMockDirectory(random, NoLockFactory::INSTANCE);
  shared_ptr<IndexWriter> writer = initIndex(random, dir, false, false);

  try {
    writer->commit();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete writer;
  }

  dir->crash();

  /*
  std::wstring[] l = dir.deque();
  Arrays.sort(l);
  for(int i=0;i<l.length;i++)
    System.out.println("file " + i + " = " + l[i] + " " + dir.fileLength(l[i]) +
  " bytes");
  */
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  assertEquals(157, reader->numDocs());
  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::index