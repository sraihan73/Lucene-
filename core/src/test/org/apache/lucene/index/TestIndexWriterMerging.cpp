using namespace std;

#include "TestIndexWriterMerging.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestIndexWriterMerging::testLucene() 
{
  int num = 100;

  shared_ptr<Directory> indexA = newDirectory();
  shared_ptr<Directory> indexB = newDirectory();

  fillIndex(random(), indexA, 0, num);
  bool fail = verifyIndex(indexA, 0);
  if (fail) {
    fail(L"Index a is invalid");
  }

  fillIndex(random(), indexB, num, num);
  fail = verifyIndex(indexB, num);
  if (fail) {
    fail(L"Index b is invalid");
  }

  shared_ptr<Directory> merged = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      merged, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                  ->setMergePolicy(newLogMergePolicy(2)));
  writer->addIndexes({indexA, indexB});
  writer->forceMerge(1);
  delete writer;

  fail = verifyIndex(merged, 0);

  assertFalse(L"The merged index is invalid", fail);
  delete indexA;
  delete indexB;
  delete merged;
}

bool TestIndexWriterMerging::verifyIndex(shared_ptr<Directory> directory,
                                         int startAt) 
{
  bool fail = false;
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);

  int max = reader->maxDoc();
  for (int i = 0; i < max; i++) {
    shared_ptr<Document> temp = reader->document(i);
    // System.out.println("doc "+i+"="+temp.getField("count").stringValue());
    // compare the index doc number to the value that it should be
    if (temp->getField(L"count")->stringValue() !=
        to_wstring(i + startAt) + L"") {
      fail = true;
      wcout << L"Document " << (i << startAt) << L" is returning document "
            << temp->getField(L"count")->stringValue() << endl;
    }
  }
  delete reader;
  return fail;
}

void TestIndexWriterMerging::fillIndex(shared_ptr<Random> random,
                                       shared_ptr<Directory> dir, int start,
                                       int numDocs) 
{

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
               ->setOpenMode(OpenMode::CREATE)
               ->setMaxBufferedDocs(2)
               ->setMergePolicy(newLogMergePolicy(2)));

  for (int i = start; i < (start + numDocs); i++) {
    shared_ptr<Document> temp = make_shared<Document>();
    temp->push_back(
        newStringField(L"count", (L"" + to_wstring(i)), Field::Store::YES));

    writer->addDocument(temp);
  }
  delete writer;
}

void TestIndexWriterMerging::testForceMergeDeletes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH));
  shared_ptr<Document> document = make_shared<Document>();

  shared_ptr<FieldType> customType = make_shared<FieldType>();
  customType->setStored(true);

  shared_ptr<FieldType> customType1 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType1->setTokenized(false);
  customType1->setStoreTermVectors(true);
  customType1->setStoreTermVectorPositions(true);
  customType1->setStoreTermVectorOffsets(true);

  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::NO);
  document->push_back(idField);
  shared_ptr<Field> storedField = newField(L"stored", L"stored", customType);
  document->push_back(storedField);
  shared_ptr<Field> termVectorField =
      newField(L"termVector", L"termVector", customType1);
  document->push_back(termVectorField);
  for (int i = 0; i < 10; i++) {
    idField->setStringValue(L"" + to_wstring(i));
    writer->addDocument(document);
  }
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  assertEquals(10, ir->maxDoc());
  assertEquals(10, ir->numDocs());
  delete ir;

  shared_ptr<IndexWriterConfig> dontMergeConfig =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMergePolicy(NoMergePolicy::INSTANCE);
  writer = make_shared<IndexWriter>(dir, dontMergeConfig);
  writer->deleteDocuments({make_shared<Term>(L"id", L"0")});
  writer->deleteDocuments({make_shared<Term>(L"id", L"7")});
  delete writer;

  ir = DirectoryReader::open(dir);
  assertEquals(8, ir->numDocs());
  delete ir;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(newLogMergePolicy()));
  assertEquals(8, writer->numDocs());
  assertEquals(10, writer->maxDoc());
  writer->forceMergeDeletes();
  assertEquals(8, writer->numDocs());
  delete writer;
  ir = DirectoryReader::open(dir);
  assertEquals(8, ir->maxDoc());
  assertEquals(8, ir->numDocs());
  delete ir;
  delete dir;
}

void TestIndexWriterMerging::testForceMergeDeletes2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH)
               ->setMergePolicy(newLogMergePolicy(50)));

  shared_ptr<Document> document = make_shared<Document>();

  shared_ptr<FieldType> customType = make_shared<FieldType>();
  customType->setStored(true);

  shared_ptr<FieldType> customType1 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType1->setTokenized(false);
  customType1->setStoreTermVectors(true);
  customType1->setStoreTermVectorPositions(true);
  customType1->setStoreTermVectorOffsets(true);

  shared_ptr<Field> storedField = newField(L"stored", L"stored", customType);
  document->push_back(storedField);
  shared_ptr<Field> termVectorField =
      newField(L"termVector", L"termVector", customType1);
  document->push_back(termVectorField);
  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::NO);
  document->push_back(idField);
  for (int i = 0; i < 98; i++) {
    idField->setStringValue(L"" + to_wstring(i));
    writer->addDocument(document);
  }
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  assertEquals(98, ir->maxDoc());
  assertEquals(98, ir->numDocs());
  delete ir;

  shared_ptr<IndexWriterConfig> dontMergeConfig =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMergePolicy(NoMergePolicy::INSTANCE);
  writer = make_shared<IndexWriter>(dir, dontMergeConfig);
  for (int i = 0; i < 98; i += 2) {
    writer->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }
  delete writer;

  ir = DirectoryReader::open(dir);
  assertEquals(49, ir->numDocs());
  delete ir;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(newLogMergePolicy(3)));
  assertEquals(49, writer->numDocs());
  writer->forceMergeDeletes();
  delete writer;
  ir = DirectoryReader::open(dir);
  assertEquals(49, ir->maxDoc());
  assertEquals(49, ir->numDocs());
  delete ir;
  delete dir;
}

void TestIndexWriterMerging::testForceMergeDeletes3() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH)
               ->setMergePolicy(newLogMergePolicy(50)));

  shared_ptr<FieldType> customType = make_shared<FieldType>();
  customType->setStored(true);

  shared_ptr<FieldType> customType1 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType1->setTokenized(false);
  customType1->setStoreTermVectors(true);
  customType1->setStoreTermVectorPositions(true);
  customType1->setStoreTermVectorOffsets(true);

  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<Field> storedField = newField(L"stored", L"stored", customType);
  document->push_back(storedField);
  shared_ptr<Field> termVectorField =
      newField(L"termVector", L"termVector", customType1);
  document->push_back(termVectorField);
  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::NO);
  document->push_back(idField);
  for (int i = 0; i < 98; i++) {
    idField->setStringValue(L"" + to_wstring(i));
    writer->addDocument(document);
  }
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  assertEquals(98, ir->maxDoc());
  assertEquals(98, ir->numDocs());
  delete ir;

  shared_ptr<IndexWriterConfig> dontMergeConfig =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMergePolicy(NoMergePolicy::INSTANCE);
  writer = make_shared<IndexWriter>(dir, dontMergeConfig);
  for (int i = 0; i < 98; i += 2) {
    writer->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(i))});
  }
  delete writer;
  ir = DirectoryReader::open(dir);
  assertEquals(49, ir->numDocs());
  delete ir;

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(newLogMergePolicy(3)));
  writer->forceMergeDeletes(false);
  delete writer;
  ir = DirectoryReader::open(dir);
  assertEquals(49, ir->maxDoc());
  assertEquals(49, ir->numDocs());
  delete ir;
  delete dir;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void TestIndexWriterMerging::MyMergeScheduler::merge(
    shared_ptr<IndexWriter> writer, MergeTrigger trigger,
    bool newMergesFound) 
{

  while (true) {
    shared_ptr<MergePolicy::OneMerge> merge = writer->getNextMerge();
    if (merge == nullptr) {
      break;
    }
    int numDocs = 0;
    for (int i = 0; i < merge->segments.size(); i++) {
      int maxDoc = merge->segments[i]->info.maxDoc();
      numDocs += maxDoc;
      assertTrue(maxDoc < 20);
    }
    writer->merge(merge);
    assertEquals(numDocs, merge->getMergeInfo()->info->maxDoc());
  }
}

TestIndexWriterMerging::MyMergeScheduler::~MyMergeScheduler() {}

void TestIndexWriterMerging::testSetMaxMergeDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergeScheduler(make_shared<MyMergeScheduler>())
          ->setMaxBufferedDocs(2)
          ->setMergePolicy(newLogMergePolicy());
  shared_ptr<LogMergePolicy> lmp =
      std::static_pointer_cast<LogMergePolicy>(conf->getMergePolicy());
  lmp->setMaxMergeDocs(20);
  lmp->setMergeFactor(2);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> document = make_shared<Document>();

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);

  document->push_back(newField(L"tvtest", L"a b c", customType));
  for (int i = 0; i < 177; i++) {
    iw->addDocument(document);
  }
  delete iw;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testNoWaitClose() throws Throwable
void TestIndexWriterMerging::testNoWaitClose() 
{
  shared_ptr<Directory> directory = newDirectory();

  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setTokenized(false);

  shared_ptr<Field> idField = newField(L"id", L"", customType);
  doc->push_back(idField);

  for (int pass = 0; pass < 2; pass++) {
    if (VERBOSE) {
      wcout << L"TEST: pass=" << pass << endl;
    }

    shared_ptr<IndexWriterConfig> conf =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
            ->setOpenMode(OpenMode::CREATE)
            ->setMaxBufferedDocs(2)
            ->setMergePolicy(newLogMergePolicy())
            ->setCommitOnClose(false);
    if (pass == 2) {
      conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
    }

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(directory, conf);
    (std::static_pointer_cast<LogMergePolicy>(
         writer->getConfig()->getMergePolicy()))
        ->setMergeFactor(100);

    for (int iter = 0; iter < 10; iter++) {
      if (VERBOSE) {
        wcout << L"TEST: iter=" << iter << endl;
      }
      for (int j = 0; j < 199; j++) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        idField->setStringValue(Integer::toString(iter * 201 + j));
        writer->addDocument(doc);
      }

      int delID = iter * 199;
      for (int j = 0; j < 20; j++) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        writer->deleteDocuments(
            {make_shared<Term>(L"id", Integer::toString(delID))});
        delID += 5;
      }

      writer->commit();

      // Force a bunch of merge threads to kick off so we
      // stress out aborting them on close:
      (std::static_pointer_cast<LogMergePolicy>(
           writer->getConfig()->getMergePolicy()))
          ->setMergeFactor(2);

      shared_ptr<IndexWriter> *const finalWriter = writer;
      shared_ptr<AtomicReference<runtime_error>> *const failure =
          make_shared<AtomicReference<runtime_error>>();
      shared_ptr<Thread> t1 = make_shared<ThreadAnonymousInnerClass>(
          shared_from_this(), doc, finalWriter, failure);

      t1->start();

      delete writer;
      t1->join();

      if (failure->get() != nullptr) {
        throw failure->get();
      }

      // Make sure reader can read
      shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
      delete reader;

      // Reopen
      writer = make_shared<IndexWriter>(
          directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                         ->setOpenMode(OpenMode::APPEND)
                         ->setMergePolicy(newLogMergePolicy())
                         ->setCommitOnClose(false));
    }
    delete writer;
  }

  delete directory;
}

TestIndexWriterMerging::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestIndexWriterMerging> outerInstance, shared_ptr<Document> doc,
    shared_ptr<org::apache::lucene::index::IndexWriter> finalWriter,
    shared_ptr<AtomicReference<runtime_error>> failure)
{
  this->outerInstance = outerInstance;
  this->doc = doc;
  this->finalWriter = finalWriter;
  this->failure = failure;
}

void TestIndexWriterMerging::ThreadAnonymousInnerClass::run()
{
  bool done = false;
  while (!done) {
    for (int i = 0; i < 100; i++) {
      try {
        finalWriter->addDocument(doc);
      } catch (const AlreadyClosedException &e) {
        done = true;
        break;
      } catch (const NullPointerException &e) {
        done = true;
        break;
      } catch (const runtime_error &e) {
        e.printStackTrace(System::out);
        failure->set(e);
        done = true;
        break;
      }
    }
    Thread::yield();
  }
}
} // namespace org::apache::lucene::index