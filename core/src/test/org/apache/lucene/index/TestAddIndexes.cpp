using namespace std;

#include "TestAddIndexes.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using MemoryPostingsFormat =
    org::apache::lucene::codecs::memory::MemoryPostingsFormat;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using LockObtainFailedException =
    org::apache::lucene::store::LockObtainFailedException;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestAddIndexes::testSimpleCase() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // two auxiliary directories
  shared_ptr<Directory> aux = newDirectory();
  shared_ptr<Directory> aux2 = newDirectory();

  shared_ptr<IndexWriter> writer = nullptr;

  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::CREATE));
  // add 100 documents
  addDocs(writer, 100);
  TestUtil::assertEquals(100, writer->maxDoc());
  delete writer;
  TestUtil::checkIndex(dir);

  writer =
      newWriter(aux, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::CREATE)
                         ->setMergePolicy(newLogMergePolicy(false)));
  // add 40 documents in separate files
  addDocs(writer, 40);
  TestUtil::assertEquals(40, writer->maxDoc());
  delete writer;

  writer =
      newWriter(aux2, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                          ->setOpenMode(OpenMode::CREATE));
  // add 50 documents in compound files
  addDocs2(writer, 50);
  TestUtil::assertEquals(50, writer->maxDoc());
  delete writer;

  // test doc count before segments are merged
  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND));
  TestUtil::assertEquals(100, writer->maxDoc());
  writer->addIndexes({aux, aux2});
  TestUtil::assertEquals(190, writer->maxDoc());
  delete writer;
  TestUtil::checkIndex(dir);

  // make sure the old index is correct
  verifyNumDocs(aux, 40);

  // make sure the new index is correct
  verifyNumDocs(dir, 190);

  // now add another set in.
  shared_ptr<Directory> aux3 = newDirectory();
  writer = newWriter(aux3,
                     newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  // add 40 documents
  addDocs(writer, 40);
  TestUtil::assertEquals(40, writer->maxDoc());
  delete writer;

  // test doc count before segments are merged
  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND));
  TestUtil::assertEquals(190, writer->maxDoc());
  writer->addIndexes({aux3});
  TestUtil::assertEquals(230, writer->maxDoc());
  delete writer;

  // make sure the new index is correct
  verifyNumDocs(dir, 230);

  verifyTermDocs(dir, make_shared<Term>(L"content", L"aaa"), 180);

  verifyTermDocs(dir, make_shared<Term>(L"content", L"bbb"), 50);

  // now fully merge it.
  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND));
  writer->forceMerge(1);
  delete writer;

  // make sure the new index is correct
  verifyNumDocs(dir, 230);

  verifyTermDocs(dir, make_shared<Term>(L"content", L"aaa"), 180);

  verifyTermDocs(dir, make_shared<Term>(L"content", L"bbb"), 50);

  // now add a single document
  shared_ptr<Directory> aux4 = newDirectory();
  writer = newWriter(aux4,
                     newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  addDocs2(writer, 1);
  delete writer;

  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND));
  TestUtil::assertEquals(230, writer->maxDoc());
  writer->addIndexes({aux4});
  TestUtil::assertEquals(231, writer->maxDoc());
  delete writer;

  verifyNumDocs(dir, 231);

  verifyTermDocs(dir, make_shared<Term>(L"content", L"bbb"), 51);
  delete dir;
  delete aux;
  delete aux2;
  delete aux3;
  delete aux4;
}

void TestAddIndexes::testWithPendingDeletes() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // auxiliary directory
  shared_ptr<Directory> aux = newDirectory();

  setUpDirs(dir, aux);
  shared_ptr<IndexWriter> writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND));
  writer->addIndexes({aux});

  // Adds 10 docs, then replaces them with another 10
  // docs, so 10 pending deletes:
  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"id", L"" + (i % 10), Field::Store::NO));
    doc->push_back(
        newTextField(L"content", L"bbb " + to_wstring(i), Field::Store::NO));
    doc->push_back(make_shared<IntPoint>(L"doc", i));
    doc->push_back(make_shared<IntPoint>(L"doc2d", i, i));
    doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
    writer->updateDocument(make_shared<Term>(L"id", L"" + (i % 10)), doc);
  }
  // Deletes one of the 10 added docs, leaving 9:
  shared_ptr<PhraseQuery> q =
      make_shared<PhraseQuery>(L"content", L"bbb", L"14");
  writer->deleteDocuments({q});

  writer->forceMerge(1);
  writer->commit();

  verifyNumDocs(dir, 1039);
  verifyTermDocs(dir, make_shared<Term>(L"content", L"aaa"), 1030);
  verifyTermDocs(dir, make_shared<Term>(L"content", L"bbb"), 9);

  delete writer;
  delete dir;
  delete aux;
}

void TestAddIndexes::testWithPendingDeletes2() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // auxiliary directory
  shared_ptr<Directory> aux = newDirectory();

  setUpDirs(dir, aux);
  shared_ptr<IndexWriter> writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND));

  // Adds 10 docs, then replaces them with another 10
  // docs, so 10 pending deletes:
  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"id", L"" + (i % 10), Field::Store::NO));
    doc->push_back(
        newTextField(L"content", L"bbb " + to_wstring(i), Field::Store::NO));
    doc->push_back(make_shared<IntPoint>(L"doc", i));
    doc->push_back(make_shared<IntPoint>(L"doc2d", i, i));
    doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
    writer->updateDocument(make_shared<Term>(L"id", L"" + (i % 10)), doc);
  }

  writer->addIndexes({aux});

  // Deletes one of the 10 added docs, leaving 9:
  shared_ptr<PhraseQuery> q =
      make_shared<PhraseQuery>(L"content", L"bbb", L"14");
  writer->deleteDocuments({q});

  writer->forceMerge(1);
  writer->commit();

  verifyNumDocs(dir, 1039);
  verifyTermDocs(dir, make_shared<Term>(L"content", L"aaa"), 1030);
  verifyTermDocs(dir, make_shared<Term>(L"content", L"bbb"), 9);

  delete writer;
  delete dir;
  delete aux;
}

void TestAddIndexes::testWithPendingDeletes3() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // auxiliary directory
  shared_ptr<Directory> aux = newDirectory();

  setUpDirs(dir, aux);
  shared_ptr<IndexWriter> writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND));

  // Adds 10 docs, then replaces them with another 10
  // docs, so 10 pending deletes:
  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"id", L"" + (i % 10), Field::Store::NO));
    doc->push_back(
        newTextField(L"content", L"bbb " + to_wstring(i), Field::Store::NO));
    doc->push_back(make_shared<IntPoint>(L"doc", i));
    doc->push_back(make_shared<IntPoint>(L"doc2d", i, i));
    doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
    writer->updateDocument(make_shared<Term>(L"id", L"" + (i % 10)), doc);
  }

  // Deletes one of the 10 added docs, leaving 9:
  shared_ptr<PhraseQuery> q =
      make_shared<PhraseQuery>(L"content", L"bbb", L"14");
  writer->deleteDocuments({q});

  writer->addIndexes({aux});

  writer->forceMerge(1);
  writer->commit();

  verifyNumDocs(dir, 1039);
  verifyTermDocs(dir, make_shared<Term>(L"content", L"aaa"), 1030);
  verifyTermDocs(dir, make_shared<Term>(L"content", L"bbb"), 9);

  delete writer;
  delete dir;
  delete aux;
}

void TestAddIndexes::testAddSelf() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // auxiliary directory
  shared_ptr<Directory> aux = newDirectory();

  shared_ptr<IndexWriter> writer = nullptr;

  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  // add 100 documents
  addDocs(writer, 100);
  TestUtil::assertEquals(100, writer->maxDoc());
  delete writer;

  writer =
      newWriter(aux, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::CREATE)
                         ->setMaxBufferedDocs(1000)
                         ->setMergePolicy(newLogMergePolicy(false)));
  // add 140 documents in separate files
  addDocs(writer, 40);
  delete writer;
  writer =
      newWriter(aux, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::CREATE)
                         ->setMaxBufferedDocs(1000)
                         ->setMergePolicy(newLogMergePolicy(false)));
  addDocs(writer, 100);
  delete writer;

  // cannot add self
  shared_ptr<IndexWriter> writer2 =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND));
  expectThrows(invalid_argument::typeid, [&]() {
    writer2->addIndexes({aux, dir});
  });
  TestUtil::assertEquals(100, writer2->maxDoc());
  delete writer2;

  // make sure the index is correct
  verifyNumDocs(dir, 100);
  delete dir;
  delete aux;
}

void TestAddIndexes::testNoTailSegments() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // auxiliary directory
  shared_ptr<Directory> aux = newDirectory();

  setUpDirs(dir, aux);

  shared_ptr<IndexWriter> writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND)
                         ->setMaxBufferedDocs(10)
                         ->setMergePolicy(newLogMergePolicy(4)));
  addDocs(writer, 10);

  writer->addIndexes({aux});
  TestUtil::assertEquals(1040, writer->maxDoc());
  TestUtil::assertEquals(1000, writer->maxDoc(0));
  delete writer;

  // make sure the index is correct
  verifyNumDocs(dir, 1040);
  delete dir;
  delete aux;
}

void TestAddIndexes::testNoCopySegments() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // auxiliary directory
  shared_ptr<Directory> aux = newDirectory();

  setUpDirs(dir, aux);

  shared_ptr<IndexWriter> writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND)
                         ->setMaxBufferedDocs(9)
                         ->setMergePolicy(newLogMergePolicy(4)));
  addDocs(writer, 2);

  writer->addIndexes({aux});
  TestUtil::assertEquals(1032, writer->maxDoc());
  TestUtil::assertEquals(1000, writer->maxDoc(0));
  delete writer;

  // make sure the index is correct
  verifyNumDocs(dir, 1032);
  delete dir;
  delete aux;
}

void TestAddIndexes::testNoMergeAfterCopy() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // auxiliary directory
  shared_ptr<Directory> aux = newDirectory();

  setUpDirs(dir, aux);

  shared_ptr<IndexWriter> writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND)
                         ->setMaxBufferedDocs(10)
                         ->setMergePolicy(newLogMergePolicy(4)));

  writer->addIndexes({aux, make_shared<MockDirectoryWrapper>(
                               random(), TestUtil::ramCopyOf(aux))});
  TestUtil::assertEquals(1060, writer->maxDoc());
  TestUtil::assertEquals(1000, writer->maxDoc(0));
  delete writer;

  // make sure the index is correct
  verifyNumDocs(dir, 1060);
  delete dir;
  delete aux;
}

void TestAddIndexes::testMergeAfterCopy() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // auxiliary directory
  shared_ptr<Directory> aux = newDirectory();

  setUpDirs(dir, aux, true);

  shared_ptr<IndexWriterConfig> dontMergeConfig =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(aux, dontMergeConfig);
  for (int i = 0; i < 20; i++) {
    writer->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }
  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(aux);
  TestUtil::assertEquals(10, reader->numDocs());
  delete reader;

  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND)
                         ->setMaxBufferedDocs(4)
                         ->setMergePolicy(newLogMergePolicy(4)));

  if (VERBOSE) {
    wcout << L"\nTEST: now addIndexes" << endl;
  }
  writer->addIndexes({aux, make_shared<MockDirectoryWrapper>(
                               random(), TestUtil::ramCopyOf(aux))});
  TestUtil::assertEquals(1020, writer->maxDoc());
  TestUtil::assertEquals(1000, writer->maxDoc(0));
  delete writer;
  delete dir;
  delete aux;
}

void TestAddIndexes::testMoreMerges() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // auxiliary directory
  shared_ptr<Directory> aux = newDirectory();
  shared_ptr<Directory> aux2 = newDirectory();

  setUpDirs(dir, aux, true);

  shared_ptr<IndexWriter> writer =
      newWriter(aux2, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                          ->setOpenMode(OpenMode::CREATE)
                          ->setMaxBufferedDocs(100)
                          ->setMergePolicy(newLogMergePolicy(10)));
  writer->addIndexes({aux});
  TestUtil::assertEquals(30, writer->maxDoc());
  TestUtil::assertEquals(3, writer->getSegmentCount());
  delete writer;

  shared_ptr<IndexWriterConfig> dontMergeConfig =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMergePolicy(NoMergePolicy::INSTANCE);
  writer = make_shared<IndexWriter>(aux, dontMergeConfig);
  for (int i = 0; i < 27; i++) {
    writer->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }
  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(aux);
  TestUtil::assertEquals(3, reader->numDocs());
  delete reader;

  dontMergeConfig =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMergePolicy(NoMergePolicy::INSTANCE);
  writer = make_shared<IndexWriter>(aux2, dontMergeConfig);
  for (int i = 0; i < 8; i++) {
    writer->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }
  delete writer;
  reader = DirectoryReader::open(aux2);
  TestUtil::assertEquals(22, reader->numDocs());
  delete reader;

  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND)
                         ->setMaxBufferedDocs(6)
                         ->setMergePolicy(newLogMergePolicy(4)));

  writer->addIndexes({aux, aux2});
  TestUtil::assertEquals(1040, writer->maxDoc());
  TestUtil::assertEquals(1000, writer->maxDoc(0));
  delete writer;
  delete dir;
  delete aux;
  delete aux2;
}

shared_ptr<IndexWriter>
TestAddIndexes::newWriter(shared_ptr<Directory> dir,
                          shared_ptr<IndexWriterConfig> conf) 
{
  conf->setMergePolicy(make_shared<LogDocMergePolicy>());
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(dir, conf);
  return writer;
}

void TestAddIndexes::addDocs(shared_ptr<IndexWriter> writer,
                             int numDocs) 
{
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
    doc->push_back(make_shared<IntPoint>(L"doc", i));
    doc->push_back(make_shared<IntPoint>(L"doc2d", i, i));
    doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
    writer->addDocument(doc);
  }
}

void TestAddIndexes::addDocs2(shared_ptr<IndexWriter> writer,
                              int numDocs) 
{
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"bbb", Field::Store::NO));
    doc->push_back(make_shared<IntPoint>(L"doc", i));
    doc->push_back(make_shared<IntPoint>(L"doc2d", i, i));
    doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
    writer->addDocument(doc);
  }
}

void TestAddIndexes::verifyNumDocs(shared_ptr<Directory> dir,
                                   int numDocs) 
{
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(numDocs, reader->maxDoc());
  TestUtil::assertEquals(numDocs, reader->numDocs());
  delete reader;
}

void TestAddIndexes::verifyTermDocs(shared_ptr<Directory> dir,
                                    shared_ptr<Term> term,
                                    int numDocs) 
{
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<PostingsEnum> postingsEnum =
      TestUtil::docs(random(), reader, term->field_, term->bytes_, nullptr,
                     PostingsEnum::NONE);
  int count = 0;
  while (postingsEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    count++;
  }
  TestUtil::assertEquals(numDocs, count);
  delete reader;
}

void TestAddIndexes::setUpDirs(shared_ptr<Directory> dir,
                               shared_ptr<Directory> aux) 
{
  setUpDirs(dir, aux, false);
}

void TestAddIndexes::setUpDirs(shared_ptr<Directory> dir,
                               shared_ptr<Directory> aux,
                               bool withID) 
{
  shared_ptr<IndexWriter> writer = nullptr;

  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::CREATE)
                         ->setMaxBufferedDocs(1000));
  // add 1000 documents in 1 segment
  if (withID) {
    addDocsWithID(writer, 1000, 0);
  } else {
    addDocs(writer, 1000);
  }
  TestUtil::assertEquals(1000, writer->maxDoc());
  TestUtil::assertEquals(1, writer->getSegmentCount());
  delete writer;

  writer =
      newWriter(aux, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::CREATE)
                         ->setMaxBufferedDocs(1000)
                         ->setMergePolicy(newLogMergePolicy(false, 10)));
  // add 30 documents in 3 segments
  for (int i = 0; i < 3; i++) {
    if (withID) {
      addDocsWithID(writer, 10, 10 * i);
    } else {
      addDocs(writer, 10);
    }
    delete writer;
    writer =
        newWriter(aux, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                           ->setOpenMode(OpenMode::APPEND)
                           ->setMaxBufferedDocs(1000)
                           ->setMergePolicy(newLogMergePolicy(false, 10)));
  }
  TestUtil::assertEquals(30, writer->maxDoc());
  TestUtil::assertEquals(3, writer->getSegmentCount());
  delete writer;
}

void TestAddIndexes::testHangOnClose() 
{

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<LogByteSizeMergePolicy> lmp =
      make_shared<LogByteSizeMergePolicy>();
  lmp->setNoCFSRatio(0.0);
  lmp->setMergeFactor(100);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(5)
               ->setMergePolicy(lmp));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  doc->push_back(
      newField(L"content", L"aaa bbb ccc ddd eee fff ggg hhh iii", customType));
  for (int i = 0; i < 60; i++) {
    writer->addDocument(doc);
  }

  shared_ptr<Document> doc2 = make_shared<Document>();
  shared_ptr<FieldType> customType2 = make_shared<FieldType>();
  customType2->setStored(true);
  doc2->push_back(newField(L"content", L"aaa bbb ccc ddd eee fff ggg hhh iii",
                           customType2));
  doc2->push_back(newField(L"content", L"aaa bbb ccc ddd eee fff ggg hhh iii",
                           customType2));
  doc2->push_back(newField(L"content", L"aaa bbb ccc ddd eee fff ggg hhh iii",
                           customType2));
  doc2->push_back(newField(L"content", L"aaa bbb ccc ddd eee fff ggg hhh iii",
                           customType2));
  for (int i = 0; i < 10; i++) {
    writer->addDocument(doc2);
  }
  delete writer;

  shared_ptr<Directory> dir2 = newDirectory();
  lmp = make_shared<LogByteSizeMergePolicy>();
  lmp->setMinMergeMB(0.0001);
  lmp->setNoCFSRatio(0.0);
  lmp->setMergeFactor(4);
  writer = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                ->setMergeScheduler(make_shared<SerialMergeScheduler>())
                ->setMergePolicy(lmp));
  writer->addIndexes({dir});
  delete writer;
  delete dir;
  delete dir2;
}

void TestAddIndexes::addDoc(shared_ptr<IndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  writer->addDocument(doc);
}

TestAddIndexes::RunAddIndexesThreads::RunAddIndexesThreads(
    shared_ptr<TestAddIndexes> outerInstance, int numCopy) 
    : readers(std::deque<std::shared_ptr<DirectoryReader>>(NUM_COPY)),
      NUM_COPY(numCopy), outerInstance(outerInstance)
{
  dir = make_shared<MockDirectoryWrapper>(LuceneTestCase::random(),
                                          make_shared<RAMDirectory>());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(
               make_shared<MockAnalyzer>(LuceneTestCase::random()))
               .setMaxBufferedDocs(2));
  for (int i = 0; i < NUM_INIT_DOCS; i++) {
    outerInstance->addDoc(writer);
  }
  delete writer;

  dir2 = LuceneTestCase::newDirectory();
  writer2 = make_shared<IndexWriter>(
      dir2, make_shared<IndexWriterConfig>(
                make_shared<MockAnalyzer>(LuceneTestCase::random())));
  writer2->commit();

  for (int i = 0; i < NUM_COPY; i++) {
    readers[i] = DirectoryReader::open(dir);
  }
}

void TestAddIndexes::RunAddIndexesThreads::launchThreads(int const numIter)
{

  for (int i = 0; i < NUM_THREADS; i++) {
    threads[i] =
        make_shared<ThreadAnonymousInnerClass>(shared_from_this(), numIter);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    threads[i]->start();
  }
}

TestAddIndexes::RunAddIndexesThreads::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(shared_ptr<RunAddIndexesThreads> outerInstance,
                              int numIter)
{
  this->outerInstance = outerInstance;
  this->numIter = numIter;
}

void TestAddIndexes::RunAddIndexesThreads::ThreadAnonymousInnerClass::run()
{
  try {

    std::deque<std::shared_ptr<Directory>> dirs(outerInstance->NUM_COPY);
    for (int k = 0; k < outerInstance->NUM_COPY; k++) {
      dirs[k] = make_shared<MockDirectoryWrapper>(
          LuceneTestCase::random(), TestUtil::ramCopyOf(outerInstance->dir));
    }

    int j = 0;

    while (true) {
      // System.out.println(Thread.currentThread().getName() + ": iter j=" + j);
      if (numIter > 0 && j == numIter) {
        break;
      }
      outerInstance->doBody(j++, dirs);
    }
  } catch (const runtime_error &t) {
    outerInstance->handle(t);
  }
}

void TestAddIndexes::RunAddIndexesThreads::joinThreads() 
{
  for (int i = 0; i < NUM_THREADS; i++) {
    threads[i]->join();
  }
}

void TestAddIndexes::RunAddIndexesThreads::close(bool doWait) throw(
    runtime_error)
{
  didClose = true;
  if (doWait == false) {
    writer2->rollback();
  } else {
    delete writer2;
  }
}

void TestAddIndexes::RunAddIndexesThreads::closeDir() 
{
  for (int i = 0; i < NUM_COPY; i++) {
    readers[i]->close();
  }
  delete dir2;
}

TestAddIndexes::CommitAndAddIndexes::CommitAndAddIndexes(
    shared_ptr<TestAddIndexes> outerInstance, int numCopy) 
    : RunAddIndexesThreads(outerInstance, numCopy), outerInstance(outerInstance)
{
}

void TestAddIndexes::CommitAndAddIndexes::handle(runtime_error t)
{
  t.printStackTrace(System::out);
  {
    lock_guard<mutex> lock(failures);
    failures.push_back(t);
  }
}

void TestAddIndexes::CommitAndAddIndexes::doBody(
    int j, std::deque<std::shared_ptr<Directory>> &dirs) 
{
  switch (j % 5) {
  case 0:
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": TEST: addIndexes(Dir[]) then full merge" << endl;
    }
    writer2->addIndexes(dirs);
    try {
      writer2->forceMerge(1);
    } catch (const IOException &ioe) {
      if (std::dynamic_pointer_cast<MergePolicy::MergeAbortedException>(
              ioe->getCause()) != nullptr) {
        // OK
      } else {
        throw ioe;
      }
    }
    break;
  case 1:
    if (VERBOSE) {
      wcout << Thread::currentThread().getName() << L": TEST: addIndexes(Dir[])"
            << endl;
    }
    writer2->addIndexes(dirs);
    break;
  case 2:
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": TEST: addIndexes(LeafReader[])" << endl;
    }
    TestUtil::addIndexesSlowly(writer2, readers);
    break;
  case 3:
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": TEST: addIndexes(Dir[]) then maybeMerge" << endl;
    }
    writer2->addIndexes(dirs);
    writer2->maybeMerge();
    break;
  case 4:
    if (VERBOSE) {
      wcout << Thread::currentThread().getName() << L": TEST: commit" << endl;
    }
    writer2->commit();
  }
}

void TestAddIndexes::testAddIndexesWithThreads() 
{

  constexpr int NUM_ITER = TEST_NIGHTLY ? 15 : 5;
  constexpr int NUM_COPY = 3;
  shared_ptr<CommitAndAddIndexes> c =
      make_shared<CommitAndAddIndexes>(shared_from_this(), NUM_COPY);
  c->launchThreads(NUM_ITER);

  for (int i = 0; i < 100; i++) {
    addDoc(c->writer2);
  }

  c->joinThreads();

  int expectedNumDocs = 100 + NUM_COPY * (4 * NUM_ITER / 5) *
                                  RunAddIndexesThreads::NUM_THREADS *
                                  RunAddIndexesThreads::NUM_INIT_DOCS;
  assertEquals(L"expected num docs don't match - failures: " + c->failures,
               expectedNumDocs, c->writer2->numDocs());

  c->close(true);

  assertTrue(L"found unexpected failures: " + c->failures, c->failures.empty());

  shared_ptr<IndexReader> reader = DirectoryReader::open(c->dir2);
  TestUtil::assertEquals(expectedNumDocs, reader->numDocs());
  delete reader;

  c->closeDir();
}

TestAddIndexes::CommitAndAddIndexes2::CommitAndAddIndexes2(
    shared_ptr<TestAddIndexes> outerInstance, int numCopy) 
    : CommitAndAddIndexes(outerInstance, numCopy), outerInstance(outerInstance)
{
}

void TestAddIndexes::CommitAndAddIndexes2::handle(runtime_error t)
{
  if (!(std::dynamic_pointer_cast<AlreadyClosedException>(t) != nullptr) &&
      !(std::dynamic_pointer_cast<NullPointerException>(t) != nullptr)) {
    t.printStackTrace(System::out);
    {
      lock_guard<mutex> lock(failures);
      failures.push_back(t);
    }
  }
}

void TestAddIndexes::testAddIndexesWithClose() 
{
  constexpr int NUM_COPY = 3;
  shared_ptr<CommitAndAddIndexes2> c =
      make_shared<CommitAndAddIndexes2>(shared_from_this(), NUM_COPY);
  // c.writer2.setInfoStream(System.out);
  c->launchThreads(-1);

  // Close w/o first stopping/joining the threads
  c->close(true);
  // c.writer2.close();

  c->joinThreads();

  c->closeDir();

  assertTrue(c->failures.empty());
}

TestAddIndexes::CommitAndAddIndexes3::CommitAndAddIndexes3(
    shared_ptr<TestAddIndexes> outerInstance, int numCopy) 
    : RunAddIndexesThreads(outerInstance, numCopy), outerInstance(outerInstance)
{
}

void TestAddIndexes::CommitAndAddIndexes3::doBody(
    int j, std::deque<std::shared_ptr<Directory>> &dirs) 
{
  switch (j % 5) {
  case 0:
    if (VERBOSE) {
      wcout << L"TEST: " << Thread::currentThread().getName()
            << L": addIndexes + full merge" << endl;
    }
    writer2->addIndexes(dirs);
    writer2->forceMerge(1);
    break;
  case 1:
    if (VERBOSE) {
      wcout << L"TEST: " << Thread::currentThread().getName() << L": addIndexes"
            << endl;
    }
    writer2->addIndexes(dirs);
    break;
  case 2:
    if (VERBOSE) {
      wcout << L"TEST: " << Thread::currentThread().getName()
            << L": addIndexes(LR[])" << endl;
    }
    TestUtil::addIndexesSlowly(writer2, readers);
    break;
  case 3:
    if (VERBOSE) {
      wcout << L"TEST: " << Thread::currentThread().getName() << L": full merge"
            << endl;
    }
    writer2->forceMerge(1);
    break;
  case 4:
    if (VERBOSE) {
      wcout << L"TEST: " << Thread::currentThread().getName() << L": commit"
            << endl;
    }
    writer2->commit();
  }
}

void TestAddIndexes::CommitAndAddIndexes3::handle(runtime_error t)
{
  bool report = true;

  if (std::dynamic_pointer_cast<AlreadyClosedException>(t) != nullptr ||
      std::dynamic_pointer_cast<MergePolicy::MergeAbortedException>(t) !=
          nullptr ||
      std::dynamic_pointer_cast<NullPointerException>(t) != nullptr) {
    report = !didClose;
  } else if (std::dynamic_pointer_cast<FileNotFoundException>(t) != nullptr ||
             std::dynamic_pointer_cast<NoSuchFileException>(t) != nullptr) {
    report = !didClose;
  } else if (std::dynamic_pointer_cast<IOException>(t) != nullptr) {
    runtime_error t2 = t.getCause();
    if (std::dynamic_pointer_cast<MergePolicy::MergeAbortedException>(t2) !=
        nullptr) {
      report = !didClose;
    }
  }
  if (report) {
    t.printStackTrace(System::out);
    {
      lock_guard<mutex> lock(failures);
      failures.push_back(t);
    }
  }
}

void TestAddIndexes::testAddIndexesWithCloseNoWait() 
{

  constexpr int NUM_COPY = 50;
  shared_ptr<CommitAndAddIndexes3> c =
      make_shared<CommitAndAddIndexes3>(shared_from_this(), NUM_COPY);
  c->launchThreads(-1);

  delay(TestUtil::nextInt(random(), 10, 500));

  // Close w/o first stopping/joining the threads
  if (VERBOSE) {
    wcout << L"TEST: now close(false)" << endl;
  }
  c->close(false);

  c->joinThreads();

  if (VERBOSE) {
    wcout << L"TEST: done join threads" << endl;
  }
  c->closeDir();

  assertTrue(c->failures.empty());
}

void TestAddIndexes::testAddIndexesWithRollback() 
{

  constexpr int NUM_COPY = TEST_NIGHTLY ? 50 : 5;
  shared_ptr<CommitAndAddIndexes3> c =
      make_shared<CommitAndAddIndexes3>(shared_from_this(), NUM_COPY);
  c->launchThreads(-1);

  delay(TestUtil::nextInt(random(), 10, 500));

  // Close w/o first stopping/joining the threads
  if (VERBOSE) {
    wcout << L"TEST: now force rollback" << endl;
  }
  c->didClose = true;
  shared_ptr<MergeScheduler> ms = c->writer2->getConfig()->getMergeScheduler();

  c->writer2->rollback();

  c->joinThreads();

  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
    TestUtil::assertEquals(
        0, (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))
               ->mergeThreadCount());
  }

  c->closeDir();

  assertTrue(c->failures.empty());
}

void TestAddIndexes::testExistingDeletes() 
{
  std::deque<std::shared_ptr<Directory>> dirs(2);
  for (int i = 0; i < dirs.size(); i++) {
    dirs[i] = newDirectory();
    shared_ptr<IndexWriterConfig> conf =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dirs[i], conf);
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"id", L"myid", Field::Store::NO));
    writer->addDocument(doc);
    delete writer;
  }

  shared_ptr<IndexWriterConfig> conf =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dirs[0], conf);

  // Now delete the document
  writer->deleteDocuments({make_shared<Term>(L"id", L"myid")});
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (DirectoryReader r =
  // DirectoryReader.open(dirs[1]))
  {
    DirectoryReader r = DirectoryReader::open(dirs[1]);
    TestUtil::addIndexesSlowly(writer, {r});
  }
  writer->commit();
  assertEquals(
      L"Documents from the incoming index should not have been deleted", 1,
      writer->numDocs());
  delete writer;

  for (auto dir : dirs) {
    dir->close();
  }
}

void TestAddIndexes::addDocsWithID(shared_ptr<IndexWriter> writer, int numDocs,
                                   int docStart) 
{
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
    doc->push_back(
        newTextField(L"id", L"" + to_wstring(docStart + i), Field::Store::YES));
    doc->push_back(make_shared<IntPoint>(L"doc", i));
    doc->push_back(make_shared<IntPoint>(L"doc2d", i, i));
    doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
    writer->addDocument(doc);
  }
}

void TestAddIndexes::testSimpleCaseCustomCodec() 
{
  // main directory
  shared_ptr<Directory> dir = newDirectory();
  // two auxiliary directories
  shared_ptr<Directory> aux = newDirectory();
  shared_ptr<Directory> aux2 = newDirectory();
  shared_ptr<Codec> codec = make_shared<CustomPerFieldCodec>();
  shared_ptr<IndexWriter> writer = nullptr;

  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::CREATE)
                         ->setCodec(codec));
  // add 100 documents
  addDocsWithID(writer, 100, 0);
  TestUtil::assertEquals(100, writer->maxDoc());
  writer->commit();
  delete writer;
  TestUtil::checkIndex(dir);

  writer =
      newWriter(aux, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::CREATE)
                         ->setCodec(codec)
                         ->setMaxBufferedDocs(10)
                         ->setMergePolicy(newLogMergePolicy(false)));
  // add 40 documents in separate files
  addDocs(writer, 40);
  TestUtil::assertEquals(40, writer->maxDoc());
  writer->commit();
  delete writer;

  writer =
      newWriter(aux2, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                          ->setOpenMode(OpenMode::CREATE)
                          ->setCodec(codec));
  // add 40 documents in compound files
  addDocs2(writer, 50);
  TestUtil::assertEquals(50, writer->maxDoc());
  writer->commit();
  delete writer;

  // test doc count before segments are merged
  writer =
      newWriter(dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND)
                         ->setCodec(codec));
  TestUtil::assertEquals(100, writer->maxDoc());
  writer->addIndexes({aux, aux2});
  TestUtil::assertEquals(190, writer->maxDoc());
  delete writer;

  delete dir;
  delete aux;
  delete aux2;
}

shared_ptr<PostingsFormat>
TestAddIndexes::CustomPerFieldCodec::getPostingsFormatForField(
    const wstring &field)
{
  if (field == L"id") {
    return directFormat;
  } else if (field == L"content") {
    return memoryFormat;
  } else {
    return defaultFormat;
  }
}

void TestAddIndexes::testNonCFSLeftovers() 
{
  std::deque<std::shared_ptr<Directory>> dirs(2);
  for (int i = 0; i < dirs.size(); i++) {
    dirs[i] = make_shared<RAMDirectory>();
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dirs[i],
        make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
    shared_ptr<Document> d = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_STORED);
    customType->setStoreTermVectors(true);
    d->push_back(make_shared<Field>(L"c", L"v", customType));
    w->addDocument(d);
    delete w;
  }

  std::deque<std::shared_ptr<DirectoryReader>> readers = {
      DirectoryReader::open(dirs[0]), DirectoryReader::open(dirs[1])};

  shared_ptr<MockDirectoryWrapper> dir =
      make_shared<MockDirectoryWrapper>(random(), make_shared<RAMDirectory>());
  shared_ptr<IndexWriterConfig> conf =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMergePolicy(newLogMergePolicy(true));
  shared_ptr<MergePolicy> lmp = conf->getMergePolicy();
  // Force creation of CFS:
  lmp->setNoCFSRatio(1.0);
  lmp->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());
  shared_ptr<IndexWriter> w3 = make_shared<IndexWriter>(dir, conf);
  TestUtil::addIndexesSlowly(w3, readers);
  delete w3;
  // we should now see segments_X,
  // _Y.cfs,_Y.cfe, _Z.si
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(L"Only one compound segment should exist", 1, sis->size());
  assertTrue(sis->info(0)->info->getUseCompoundFile());
  delete dir;
}

TestAddIndexes::UnRegisteredCodec::UnRegisteredCodec()
    : org::apache::lucene::codecs::FilterCodec(L"NotRegistered",
                                               TestUtil::getDefaultCodec())
{
}

void TestAddIndexes::testAddIndexMissingCodec() 
{
  shared_ptr<BaseDirectoryWrapper> toAdd = newDirectory();
  // Disable checkIndex, else we get an exception because
  // of the unregistered codec:
  toAdd->setCheckIndexOnClose(false);
  {
    shared_ptr<IndexWriterConfig> conf =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
    conf->setCodec(make_shared<UnRegisteredCodec>());
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(toAdd, conf);
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<FieldType> customType = make_shared<FieldType>();
    customType->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
    doc->push_back(newField(L"foo", L"bar", customType));
    w->addDocument(doc);
    delete w;
  }

  {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriterConfig> conf =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
    conf->setCodec(
        TestUtil::alwaysPostingsFormat(make_shared<MemoryPostingsFormat>()));
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, conf);
    expectThrows(invalid_argument::typeid, [&]() { w->addIndexes({toAdd}); });
    delete w;
    shared_ptr<IndexReader> open = DirectoryReader::open(dir);
    TestUtil::assertEquals(0, open->numDocs());
    delete open;
    delete dir;
  }

  expectThrows(invalid_argument::typeid,
               [&]() { DirectoryReader::open(toAdd); });
  delete toAdd;
}

void TestAddIndexes::testFieldNamesChanged() 
{
  shared_ptr<Directory> d1 = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), d1);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"f1", L"doc1 field1", Field::Store::YES));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  w->addDocument(doc);
  shared_ptr<DirectoryReader> r1 = w->getReader();
  delete w;

  shared_ptr<Directory> d2 = newDirectory();
  w = make_shared<RandomIndexWriter>(random(), d2);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"f2", L"doc2 field2", Field::Store::YES));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  w->addDocument(doc);
  shared_ptr<DirectoryReader> r2 = w->getReader();
  delete w;

  shared_ptr<Directory> d3 = newDirectory();
  w = make_shared<RandomIndexWriter>(random(), d3);
  TestUtil::addIndexesSlowly(w->w, {r1, r2});
  r1->close();
  delete d1;
  r2->close();
  delete d2;

  shared_ptr<IndexReader> r3 = w->getReader();
  delete w;
  TestUtil::assertEquals(2, r3->numDocs());
  for (int docID = 0; docID < 2; docID++) {
    shared_ptr<Document> d = r3->document(docID);
    if (d[L"id"].equals(L"1")) {
      TestUtil::assertEquals(L"doc1 field1", d[L"f1"]);
    } else {
      TestUtil::assertEquals(L"doc2 field2", d[L"f2"]);
    }
  }
  delete r3;
  delete d3;
}

void TestAddIndexes::testAddEmpty() 
{
  shared_ptr<Directory> d1 = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), d1);
  w->addIndexes(std::deque<std::shared_ptr<CodecReader>>(0));
  delete w;
  shared_ptr<DirectoryReader> dr = DirectoryReader::open(d1);
  for (shared_ptr<LeafReaderContext> ctx : dr->leaves()) {
    assertTrue(L"empty segments should be dropped by addIndexes",
               ctx->reader()->maxDoc() > 0);
  }
  dr->close();
  delete d1;
}

void TestAddIndexes::testFakeAllDeleted() 
{
  shared_ptr<Directory> src = newDirectory(), dest = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), src);
  w->addDocument(make_shared<Document>());
  shared_ptr<LeafReader> allDeletedReader = make_shared<AllDeletedFilterReader>(
      w->getReader()->leaves()->get(0).reader());
  delete w;

  w = make_shared<RandomIndexWriter>(random(), dest);
  w->addIndexes({SlowCodecReaderWrapper::wrap(allDeletedReader)});
  delete w;
  shared_ptr<DirectoryReader> dr = DirectoryReader::open(src);
  for (shared_ptr<LeafReaderContext> ctx : dr->leaves()) {
    assertTrue(L"empty segments should be dropped by addIndexes",
               ctx->reader()->maxDoc() > 0);
  }
  dr->close();
  delete allDeletedReader;
  delete src;
  delete dest;
}

void TestAddIndexes::testLocksBlock() 
{
  shared_ptr<Directory> src = newDirectory();
  shared_ptr<RandomIndexWriter> w1 =
      make_shared<RandomIndexWriter>(random(), src);
  w1->addDocument(make_shared<Document>());
  w1->commit();

  shared_ptr<Directory> dest = newDirectory();

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> w2 =
      make_shared<RandomIndexWriter>(random(), dest, iwc);

  expectThrows(LockObtainFailedException::typeid,
               [&]() { w2->addIndexes({src}); });

  delete w1;
  delete w2;
  IOUtils::close({src, dest});
}

void TestAddIndexes::testIllegalIndexSortChange1() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc1 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc1->setIndexSort(
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::INT)));
  shared_ptr<RandomIndexWriter> w1 =
      make_shared<RandomIndexWriter>(random(), dir1, iwc1);
  w1->addDocument(make_shared<Document>());
  w1->commit();
  w1->addDocument(make_shared<Document>());
  w1->commit();
  // so the index sort is in fact burned into the index:
  w1->forceMerge(1);
  delete w1;

  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc2 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc2->setIndexSort(make_shared<Sort>(
      make_shared<SortField>(L"foo", SortField::Type::STRING)));
  shared_ptr<RandomIndexWriter> w2 =
      make_shared<RandomIndexWriter>(random(), dir2, iwc2);
  wstring message =
      expectThrows(invalid_argument::typeid, [&]() { w2->addIndexes({dir1}); })
          .what();
  TestUtil::assertEquals(
      L"cannot change index sort from <int: \"foo\"> to <string: \"foo\">",
      message);
  IOUtils::close({dir1, w2, dir2});
}

void TestAddIndexes::testIllegalIndexSortChange2() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc1 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc1->setIndexSort(
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::INT)));
  shared_ptr<RandomIndexWriter> w1 =
      make_shared<RandomIndexWriter>(random(), dir1, iwc1);
  w1->addDocument(make_shared<Document>());
  w1->commit();
  w1->addDocument(make_shared<Document>());
  w1->commit();
  // so the index sort is in fact burned into the index:
  w1->forceMerge(1);
  delete w1;

  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc2 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc2->setIndexSort(make_shared<Sort>(
      make_shared<SortField>(L"foo", SortField::Type::STRING)));
  shared_ptr<RandomIndexWriter> w2 =
      make_shared<RandomIndexWriter>(random(), dir2, iwc2);
  shared_ptr<IndexReader> r1 = DirectoryReader::open(dir1);
  wstring message =
      expectThrows(invalid_argument::typeid,
                   [&]() {
                     w2->addIndexes({std::static_pointer_cast<SegmentReader>(
                         getOnlyLeafReader(r1))});
                   })
          .what();
  TestUtil::assertEquals(
      L"cannot change index sort from <int: \"foo\"> to <string: \"foo\">",
      message);
  IOUtils::close({r1, dir1, w2, dir2});
}

void TestAddIndexes::testAddIndexesDVUpdateSameSegmentName() throw(
    runtime_error)
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc1 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w1 = make_shared<IndexWriter>(dir1, iwc1);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"soft_delete", 1));
  w1->addDocument(doc);
  w1->flush();

  w1->updateDocValues(make_shared<Term>(L"id", L"1"),
                      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
  w1->commit();
  delete w1;

  shared_ptr<IndexWriterConfig> iwc2 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc2);
  w2->addIndexes({dir1});
  w2->commit();
  delete w2;

  if (VERBOSE) {
    wcout << L"\nTEST: now open w3" << endl;
  }
  shared_ptr<IndexWriterConfig> iwc3 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  if (VERBOSE) {
    iwc3->setInfoStream(System::out);
  }
  shared_ptr<IndexWriter> w3 = make_shared<IndexWriter>(dir2, iwc3);
  delete w3;

  iwc3 = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  w3 = make_shared<IndexWriter>(dir2, iwc3);
  delete w3;
  delete dir1;
  delete dir2;
}

void TestAddIndexes::testAddIndexesDVUpdateNewSegmentName() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc1 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w1 = make_shared<IndexWriter>(dir1, iwc1);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"soft_delete", 1));
  w1->addDocument(doc);
  w1->flush();

  w1->updateDocValues(make_shared<Term>(L"id", L"1"),
                      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
  w1->commit();
  delete w1;

  shared_ptr<IndexWriterConfig> iwc2 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc2);
  w2->addDocument(make_shared<Document>());
  w2->commit();

  w2->addIndexes({dir1});
  w2->commit();
  delete w2;

  if (VERBOSE) {
    wcout << L"\nTEST: now open w3" << endl;
  }
  shared_ptr<IndexWriterConfig> iwc3 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  if (VERBOSE) {
    iwc3->setInfoStream(System::out);
  }
  shared_ptr<IndexWriter> w3 = make_shared<IndexWriter>(dir2, iwc3);
  delete w3;

  iwc3 = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  w3 = make_shared<IndexWriter>(dir2, iwc3);
  delete w3;
  delete dir1;
  delete dir2;
}

void TestAddIndexes::testAddIndicesWithSoftDeletes() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc1 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setSoftDeletesField(L"soft_delete");
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir1, iwc1);
  for (int i = 0; i < 30; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int docID = random()->nextInt(5);
    doc->push_back(make_shared<StringField>(L"id", L"" + to_wstring(docID),
                                            Field::Store::YES));
    writer->softUpdateDocument(
        make_shared<Term>(L"id", L"" + to_wstring(docID)), doc,
        {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
    if (random()->nextBoolean()) {
      writer->flush();
    }
  }
  writer->commit();
  delete writer;
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir1);
  shared_ptr<DirectoryReader> wrappedReader =
      make_shared<SoftDeletesDirectoryReaderWrapper>(reader, L"soft_delete");
  shared_ptr<Directory> dir2 = newDirectory();
  int numDocs = reader->numDocs();
  int maxDoc = reader->maxDoc();
  TestUtil::assertEquals(numDocs, maxDoc);
  iwc1 = newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setSoftDeletesField(L"soft_delete");
  writer = make_shared<IndexWriter>(dir2, iwc1);
  std::deque<std::shared_ptr<CodecReader>> readers(reader->leaves()->size());
  for (int i = 0; i < readers.size(); i++) {
    readers[i] = std::static_pointer_cast<CodecReader>(
        reader->leaves()->get(i).reader());
  }
  writer->addIndexes(readers);
  TestUtil::assertEquals(wrappedReader->numDocs(), writer->numDocs());
  TestUtil::assertEquals(maxDoc, writer->maxDoc());
  writer->commit();
  shared_ptr<SegmentCommitInfo> commitInfo = writer->segmentInfos->asList()[0];
  TestUtil::assertEquals(maxDoc - wrappedReader->numDocs(),
                         commitInfo->getSoftDelCount());
  delete writer;
  shared_ptr<Directory> dir3 = newDirectory();
  iwc1 = newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setSoftDeletesField(L"soft_delete");
  writer = make_shared<IndexWriter>(dir3, iwc1);
  for (int i = 0; i < readers.size(); i++) {
    readers[i] = std::static_pointer_cast<CodecReader>(
        wrappedReader->leaves()->get(i).reader());
  }
  writer->addIndexes(readers);
  TestUtil::assertEquals(wrappedReader->numDocs(), writer->numDocs());
  TestUtil::assertEquals(wrappedReader->numDocs(), writer->maxDoc());
  IOUtils::close({reader, writer, dir3, dir2, dir1});
}
} // namespace org::apache::lucene::index