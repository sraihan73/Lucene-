using namespace std;

#include "TestSoftDeletesDirectoryReaderWrapper.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSoftDeletesDirectoryReaderWrapper::
    testReuseUnchangedLeafReader() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> indexWriterConfig = newIndexWriterConfig();
  wstring softDeletesField = L"soft_delete";
  indexWriterConfig->setSoftDeletesField(softDeletesField);
  indexWriterConfig->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, indexWriterConfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  writer->addDocument(doc);
  writer->commit();
  shared_ptr<DirectoryReader> reader =
      make_shared<SoftDeletesDirectoryReaderWrapper>(DirectoryReader::open(dir),
                                                     softDeletesField);
  assertEquals(2, reader->numDocs());
  assertEquals(2, reader->maxDoc());
  assertEquals(0, reader->numDeletedDocs());

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"3", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  writer->addDocument(doc);
  writer->commit();

  shared_ptr<DirectoryReader> newReader =
      DirectoryReader::openIfChanged(reader);
  assertNotSame(newReader, reader);
  reader->close();
  reader = newReader;
  assertEquals(3, reader->numDocs());
  assertEquals(4, reader->maxDoc());
  assertEquals(1, reader->numDeletedDocs());

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"3", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
  writer->commit();

  newReader = DirectoryReader::openIfChanged(reader);
  assertNotSame(newReader, reader);
  assertEquals(3, newReader->getSequentialSubReaders().size());
  assertEquals(2, reader->getSequentialSubReaders().size());
  assertSame(reader->getSequentialSubReaders()[0],
             newReader->getSequentialSubReaders()[0]);
  assertNotSame(reader->getSequentialSubReaders()[1],
                newReader->getSequentialSubReaders()[1]);
  assertTrue(isWrapped(reader->getSequentialSubReaders()[0]));
  // last one has no soft deletes
  assertFalse(isWrapped(reader->getSequentialSubReaders()[1]));

  assertTrue(isWrapped(newReader->getSequentialSubReaders()[0]));
  assertTrue(isWrapped(newReader->getSequentialSubReaders()[1]));
  // last one has no soft deletes
  assertFalse(isWrapped(newReader->getSequentialSubReaders()[2]));
  reader->close();
  reader = newReader;
  assertEquals(3, reader->numDocs());
  assertEquals(5, reader->maxDoc());
  assertEquals(2, reader->numDeletedDocs());
  IOUtils::close({reader, writer, dir});
}

bool TestSoftDeletesDirectoryReaderWrapper::isWrapped(
    shared_ptr<LeafReader> reader)
{
  return std::dynamic_pointer_cast<
             SoftDeletesDirectoryReaderWrapper::SoftDeletesFilterLeafReader>(
             reader) != nullptr ||
         std::dynamic_pointer_cast<
             SoftDeletesDirectoryReaderWrapper::SoftDeletesFilterCodecReader>(
             reader) != nullptr;
}

void TestSoftDeletesDirectoryReaderWrapper::testMixSoftAndHardDeletes() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> indexWriterConfig = newIndexWriterConfig();
  wstring softDeletesField = L"soft_delete";
  indexWriterConfig->setSoftDeletesField(softDeletesField);
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, indexWriterConfig);
  shared_ptr<Set<int>> uniqueDocs = unordered_set<int>();
  for (int i = 0; i < 100; i++) {
    int docId = random()->nextInt(5);
    uniqueDocs->add(docId);
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<StringField>(L"id", to_wstring(docId), Field::Store::YES));
    if (docId % 2 == 0) {
      writer->updateDocument(make_shared<Term>(L"id", to_wstring(docId)), doc);
    } else {
      writer->softUpdateDocument(
          make_shared<Term>(L"id", to_wstring(docId)), doc,
          {make_shared<NumericDocValuesField>(softDeletesField, 0)});
    }
  }

  writer->commit();
  delete writer;
  shared_ptr<DirectoryReader> reader =
      make_shared<SoftDeletesDirectoryReaderWrapper>(DirectoryReader::open(dir),
                                                     softDeletesField);
  assertEquals(uniqueDocs->size(), reader->numDocs());
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  for (shared_ptr<> : : optional<int> docId : uniqueDocs) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(1, searcher
                        ->search(make_shared<TermQuery>(make_shared<Term>(
                                     L"id", docId->toString())),
                                 1)
                        ->totalHits);
  }

  IOUtils::close({reader, dir});
}

void TestSoftDeletesDirectoryReaderWrapper::testReaderCacheKey() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> indexWriterConfig = newIndexWriterConfig();
  wstring softDeletesField = L"soft_delete";
  indexWriterConfig->setSoftDeletesField(softDeletesField);
  indexWriterConfig->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, indexWriterConfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  writer->addDocument(doc);
  writer->commit();
  shared_ptr<DirectoryReader> reader =
      make_shared<SoftDeletesDirectoryReaderWrapper>(DirectoryReader::open(dir),
                                                     softDeletesField);
  shared_ptr<IndexReader::CacheHelper> readerCacheHelper =
      reader->leaves()->get(0).reader().getReaderCacheHelper();
  shared_ptr<AtomicInteger> leafCalled = make_shared<AtomicInteger>(0);
  shared_ptr<AtomicInteger> dirCalled = make_shared<AtomicInteger>(0);
  readerCacheHelper->addClosedListener([&](any key) {
    leafCalled->incrementAndGet();
    assertSame(key, readerCacheHelper->getKey());
  });
  shared_ptr<IndexReader::CacheHelper> dirReaderCacheHelper =
      reader->getReaderCacheHelper();
  dirReaderCacheHelper->addClosedListener([&](any key) {
    dirCalled->incrementAndGet();
    assertSame(key, dirReaderCacheHelper->getKey());
  });
  assertEquals(2, reader->numDocs());
  assertEquals(2, reader->maxDoc());
  assertEquals(0, reader->numDeletedDocs());

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"2", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"3", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  writer->addDocument(doc);
  writer->commit();
  assertEquals(0, leafCalled->get());
  assertEquals(0, dirCalled->get());
  shared_ptr<DirectoryReader> newReader =
      DirectoryReader::openIfChanged(reader);
  assertEquals(0, leafCalled->get());
  assertEquals(0, dirCalled->get());
  assertNotSame(newReader->getReaderCacheHelper().getKey(),
                reader->getReaderCacheHelper().getKey());
  assertNotSame(newReader, reader);
  reader->close();
  reader = newReader;
  assertEquals(1, dirCalled->get());
  assertEquals(1, leafCalled->get());
  IOUtils::close({reader, writer, dir});
}
} // namespace org::apache::lucene::index