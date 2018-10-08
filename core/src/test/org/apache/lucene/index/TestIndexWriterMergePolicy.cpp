using namespace std;

#include "TestIndexWriterMergePolicy.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestIndexWriterMergePolicy::testNormalCase() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(make_shared<LogDocMergePolicy>()));

  for (int i = 0; i < 100; i++) {
    addDoc(writer);
    checkInvariants(writer);
  }

  delete writer;
  delete dir;
}

void TestIndexWriterMergePolicy::testNoOverMerge() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(make_shared<LogDocMergePolicy>()));

  bool noOverMerge = false;
  for (int i = 0; i < 100; i++) {
    addDoc(writer);
    checkInvariants(writer);
    if (writer->getNumBufferedDocuments() + writer->getSegmentCount() >= 18) {
      noOverMerge = true;
    }
  }
  assertTrue(noOverMerge);

  delete writer;
  delete dir;
}

void TestIndexWriterMergePolicy::testForceFlush() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<LogDocMergePolicy> mp = make_shared<LogDocMergePolicy>();
  mp->setMinMergeDocs(100);
  mp->setMergeFactor(10);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(mp));

  for (int i = 0; i < 100; i++) {
    addDoc(writer);
    delete writer;

    mp = make_shared<LogDocMergePolicy>();
    mp->setMergeFactor(10);
    writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setOpenMode(OpenMode::APPEND)
                 ->setMaxBufferedDocs(10)
                 ->setMergePolicy(mp));
    mp->setMinMergeDocs(100);
    checkInvariants(writer);
  }

  delete writer;
  delete dir;
}

void TestIndexWriterMergePolicy::testMergeFactorChange() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(newLogMergePolicy())
               ->setMergeScheduler(make_shared<SerialMergeScheduler>()));

  for (int i = 0; i < 250; i++) {
    addDoc(writer);
    checkInvariants(writer);
  }

  (std::static_pointer_cast<LogMergePolicy>(
       writer->getConfig()->getMergePolicy()))
      ->setMergeFactor(5);

  // merge policy only fixes segments on levels where merges
  // have been triggered, so check invariants after all adds
  for (int i = 0; i < 10; i++) {
    addDoc(writer);
  }
  checkInvariants(writer);

  delete writer;
  delete dir;
}

void TestIndexWriterMergePolicy::testMaxBufferedDocsChange() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(101)
               ->setMergePolicy(make_shared<LogDocMergePolicy>())
               ->setMergeScheduler(make_shared<SerialMergeScheduler>()));

  // leftmost* segment has 1 doc
  // rightmost* segment has 100 docs
  for (int i = 1; i <= 100; i++) {
    for (int j = 0; j < i; j++) {
      addDoc(writer);
      checkInvariants(writer);
    }
    delete writer;

    writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setOpenMode(OpenMode::APPEND)
                 ->setMaxBufferedDocs(101)
                 ->setMergePolicy(make_shared<LogDocMergePolicy>())
                 ->setMergeScheduler(make_shared<SerialMergeScheduler>()));
  }

  delete writer;
  shared_ptr<LogDocMergePolicy> ldmp = make_shared<LogDocMergePolicy>();
  ldmp->setMergeFactor(10);
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(ldmp)
               ->setMergeScheduler(make_shared<SerialMergeScheduler>()));

  // merge policy only fixes segments on levels where merges
  // have been triggered, so check invariants after all adds
  for (int i = 0; i < 100; i++) {
    addDoc(writer);
  }
  checkInvariants(writer);

  for (int i = 100; i < 1000; i++) {
    addDoc(writer);
  }
  writer->commit();
  writer->waitForMerges();
  writer->commit();
  checkInvariants(writer);

  delete writer;
  delete dir;
}

void TestIndexWriterMergePolicy::testMergeDocCount0() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<LogDocMergePolicy> ldmp = make_shared<LogDocMergePolicy>();
  ldmp->setMergeFactor(100);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(ldmp));

  for (int i = 0; i < 250; i++) {
    addDoc(writer);
    checkInvariants(writer);
  }
  delete writer;

  // delete some docs without merging
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(NoMergePolicy::INSTANCE));
  writer->deleteDocuments({make_shared<Term>(L"content", L"aaa")});
  delete writer;

  ldmp = make_shared<LogDocMergePolicy>();
  ldmp->setMergeFactor(5);
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(ldmp)
               ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>()));

  // merge factor is changed, so check invariants after all adds
  for (int i = 0; i < 10; i++) {
    addDoc(writer);
  }
  writer->commit();
  writer->waitForMerges();
  writer->commit();
  checkInvariants(writer);
  assertEquals(10, writer->maxDoc());

  delete writer;
  delete dir;
}

void TestIndexWriterMergePolicy::addDoc(shared_ptr<IndexWriter> writer) throw(
    IOException)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  writer->addDocument(doc);
}

void TestIndexWriterMergePolicy::checkInvariants(
    shared_ptr<IndexWriter> writer) 
{
  writer->waitForMerges();
  int maxBufferedDocs = writer->getConfig()->getMaxBufferedDocs();
  int mergeFactor = (std::static_pointer_cast<LogMergePolicy>(
                         writer->getConfig()->getMergePolicy()))
                        ->getMergeFactor();
  int maxMergeDocs = (std::static_pointer_cast<LogMergePolicy>(
                          writer->getConfig()->getMergePolicy()))
                         ->getMaxMergeDocs();

  int ramSegmentCount = writer->getNumBufferedDocuments();
  assertTrue(ramSegmentCount < maxBufferedDocs);

  int lowerBound = -1;
  int upperBound = maxBufferedDocs;
  int numSegments = 0;

  int segmentCount = writer->getSegmentCount();
  for (int i = segmentCount - 1; i >= 0; i--) {
    int docCount = writer->maxDoc(i);
    assertTrue(L"docCount=" + to_wstring(docCount) + L" lowerBound=" +
                   to_wstring(lowerBound) + L" upperBound=" +
                   to_wstring(upperBound) + L" i=" + to_wstring(i) +
                   L" segmentCount=" + to_wstring(segmentCount) + L" index=" +
                   writer->segString() + L" config=" + writer->getConfig(),
               docCount > lowerBound);

    if (docCount <= upperBound) {
      numSegments++;
    } else {
      if (upperBound * mergeFactor <= maxMergeDocs) {
        assertTrue(L"maxMergeDocs=" + to_wstring(maxMergeDocs) +
                       L"; numSegments=" + to_wstring(numSegments) +
                       L"; upperBound=" + to_wstring(upperBound) +
                       L"; mergeFactor=" + to_wstring(mergeFactor) +
                       L"; segs=" + writer->segString() + L" config=" +
                       writer->getConfig(),
                   numSegments < mergeFactor);
      }

      do {
        lowerBound = upperBound;
        upperBound *= mergeFactor;
      } while (docCount > upperBound);
      numSegments = 1;
    }
  }
  if (upperBound * mergeFactor <= maxMergeDocs) {
    assertTrue(numSegments < mergeFactor);
  }
}

void TestIndexWriterMergePolicy::testSetters()
{
  assertSetters(make_shared<LogByteSizeMergePolicy>());
  assertSetters(make_shared<LogDocMergePolicy>());
}

void TestIndexWriterMergePolicy::assertSetters(shared_ptr<MergePolicy> lmp)
{
  lmp->setMaxCFSSegmentSizeMB(2.0);
  assertEquals(2.0, lmp->getMaxCFSSegmentSizeMB(), EPSILON);

  lmp->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());
  assertEquals(numeric_limits<int64_t>::max() / 1024 / 1024.0,
               lmp->getMaxCFSSegmentSizeMB(),
               EPSILON * numeric_limits<int64_t>::max());

  lmp->setMaxCFSSegmentSizeMB(numeric_limits<int64_t>::max() / 1024 / 1024.0);
  assertEquals(numeric_limits<int64_t>::max() / 1024 / 1024.0,
               lmp->getMaxCFSSegmentSizeMB(),
               EPSILON * numeric_limits<int64_t>::max());

  expectThrows(invalid_argument::typeid,
               [&]() { lmp->setMaxCFSSegmentSizeMB(-2.0); });

  // TODO: Add more checks for other non-double setters!
}
} // namespace org::apache::lucene::index