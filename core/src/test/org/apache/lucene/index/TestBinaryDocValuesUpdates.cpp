using namespace std;

#include "TestBinaryDocValuesUpdates.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using AssertingDocValuesFormat =
    org::apache::lucene::codecs::asserting::AssertingDocValuesFormat;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using Directory = org::apache::lucene::store::Directory;
using NRTCachingDirectory = org::apache::lucene::store::NRTCachingDirectory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

int64_t TestBinaryDocValuesUpdates::getValue(
    shared_ptr<BinaryDocValues> bdv) 
{
  shared_ptr<BytesRef> term = bdv->binaryValue();
  int idx = term->offset;
  assert(term->length > 0);
  char b = term->bytes[idx++];
  int64_t value = b & 0x7FLL;
  for (int shift = 7; (b & 0x80LL) != 0; shift += 7) {
    b = term->bytes[idx++];
    value |= (b & 0x7FLL) << shift;
  }
  return value;
}

shared_ptr<BytesRef> TestBinaryDocValuesUpdates::toBytes(int64_t value)
{
  //    long orig = value;
  shared_ptr<BytesRef> bytes =
      make_shared<BytesRef>(10); // negative longs may take 10 bytes
  while ((value & ~0x7FLL) != 0LL) {
    bytes->bytes[bytes->length++] =
        static_cast<char>((value & 0x7FLL) | 0x80LL);
    value = static_cast<int64_t>(static_cast<uint64_t>(value) >> 7);
  }
  bytes->bytes[bytes->length++] = static_cast<char>(value);
  //    System.err.println("[" + Thread.currentThread().getName() + "] value=" +
  //    orig + ", bytes=" + bytes);
  return bytes;
}

shared_ptr<Document> TestBinaryDocValuesUpdates::doc(int id)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"id", L"doc-" + to_wstring(id), Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"val", toBytes(id + 1)));
  return doc;
}

void TestBinaryDocValuesUpdates::testUpdatesAreFlushed() throw(
    IOException, InterruptedException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setRAMBufferSizeMB(0.00000001));
  writer->addDocument(doc(0)); // val=1
  writer->addDocument(doc(1)); // val=2
  writer->addDocument(doc(3)); // val=2
  writer->commit();
  TestUtil::assertEquals(1, writer->getFlushDeletesCount());
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-0"), L"val",
                               toBytes(5));
  TestUtil::assertEquals(2, writer->getFlushDeletesCount());
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-1"), L"val",
                               toBytes(6));
  TestUtil::assertEquals(3, writer->getFlushDeletesCount());
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-2"), L"val",
                               toBytes(7));
  TestUtil::assertEquals(4, writer->getFlushDeletesCount());
  writer->getConfig()->setRAMBufferSizeMB(1000);
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-2"), L"val",
                               toBytes(7));
  TestUtil::assertEquals(4, writer->getFlushDeletesCount());
  delete writer;
  delete dir;
}

void TestBinaryDocValuesUpdates::testSimple() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // make sure random config doesn't flush on us
  conf->setMaxBufferedDocs(10);
  conf->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  writer->addDocument(doc(0));   // val=1
  writer->addDocument(doc(1));   // val=2
  if (random()->nextBoolean()) { // randomly commit before the update is sent
    writer->commit();
  }
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-0"), L"val",
                               toBytes(2)); // doc=0, exp=2

  shared_ptr<DirectoryReader> *const reader;
  if (random()->nextBoolean()) { // not NRT
    delete writer;
    reader = DirectoryReader::open(dir);
  } else { // NRT
    reader = DirectoryReader::open(writer);
    delete writer;
  }

  TestUtil::assertEquals(1, reader->leaves()->size());
  shared_ptr<LeafReader> r = reader->leaves()->get(0).reader();
  shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"val");
  TestUtil::assertEquals(0, bdv->nextDoc());
  TestUtil::assertEquals(2, getValue(bdv));
  TestUtil::assertEquals(1, bdv->nextDoc());
  TestUtil::assertEquals(2, getValue(bdv));
  reader->close();

  delete dir;
}

void TestBinaryDocValuesUpdates::testUpdateFewSegments() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMaxBufferedDocs(2);                   // generate few segments
  conf->setMergePolicy(NoMergePolicy::INSTANCE); // prevent merges for this test
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  int numDocs = 10;
  std::deque<int64_t> expectedValues(numDocs);
  for (int i = 0; i < numDocs; i++) {
    writer->addDocument(doc(i));
    expectedValues[i] = i + 1;
  }
  writer->commit();

  // update few docs
  for (int i = 0; i < numDocs; i++) {
    if (random()->nextDouble() < 0.4) {
      int64_t value = (i + 1) * 2;
      writer->updateBinaryDocValue(
          make_shared<Term>(L"id", L"doc-" + to_wstring(i)), L"val",
          toBytes(value));
      expectedValues[i] = value;
    }
  }

  shared_ptr<DirectoryReader> *const reader;
  if (random()->nextBoolean()) { // not NRT
    delete writer;
    reader = DirectoryReader::open(dir);
  } else { // NRT
    reader = DirectoryReader::open(writer);
    delete writer;
  }

  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"val");
    assertNotNull(bdv);
    for (int i = 0; i < r->maxDoc(); i++) {
      TestUtil::assertEquals(i, bdv->nextDoc());
      int64_t expected = expectedValues[i + context->docBase];
      int64_t actual = getValue(bdv);
      TestUtil::assertEquals(expected, actual);
    }
  }

  reader->close();
  delete dir;
}

void TestBinaryDocValuesUpdates::testReopen() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  writer->addDocument(doc(0));
  writer->addDocument(doc(1));

  constexpr bool isNRT = random()->nextBoolean();
  shared_ptr<DirectoryReader> *const reader1;
  if (isNRT) {
    reader1 = DirectoryReader::open(writer);
  } else {
    writer->commit();
    reader1 = DirectoryReader::open(dir);
  }
  wcout << L"TEST: isNRT=" << isNRT << L" reader1=" << reader1 << endl;

  // update doc
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-0"), L"val",
                               toBytes(10)); // update doc-0's value to 10
  if (!isNRT) {
    writer->commit();
  }

  wcout << L"TEST: now reopen" << endl;

  // reopen reader and assert only it sees the update
  shared_ptr<DirectoryReader> *const reader2 =
      DirectoryReader::openIfChanged(reader1);
  assertNotNull(reader2);
  assertTrue(reader1 != reader2);

  shared_ptr<BinaryDocValues> bdv1 =
      reader1->leaves()->get(0).reader().getBinaryDocValues(L"val");
  shared_ptr<BinaryDocValues> bdv2 =
      reader2->leaves()->get(0).reader().getBinaryDocValues(L"val");
  TestUtil::assertEquals(0, bdv1->nextDoc());
  TestUtil::assertEquals(1, getValue(bdv1));
  TestUtil::assertEquals(0, bdv2->nextDoc());
  TestUtil::assertEquals(10, getValue(bdv2));

  delete writer;
  IOUtils::close({reader1, reader2, dir});
}

void TestBinaryDocValuesUpdates::testUpdatesAndDeletes() 
{
  // create an index with a segment with only deletes, a segment with both
  // deletes and updates and a segment with only updates
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMaxBufferedDocs(10);                  // control segment flushing
  conf->setMergePolicy(NoMergePolicy::INSTANCE); // prevent merges for this test
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  for (int i = 0; i < 6; i++) {
    writer->addDocument(doc(i));
    if (i % 2 == 1) {
      writer->commit(); // create 2-docs segments
    }
  }

  // delete doc-1 and doc-2
  writer->deleteDocuments(
      {make_shared<Term>(L"id", L"doc-1"),
       make_shared<Term>(L"id", L"doc-2")}); // 1st and 2nd segments

  // update docs 3 and 5
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-3"), L"val",
                               toBytes(17LL));
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-5"), L"val",
                               toBytes(17LL));

  shared_ptr<DirectoryReader> *const reader;
  if (random()->nextBoolean()) { // not NRT
    delete writer;
    reader = DirectoryReader::open(dir);
  } else { // NRT
    reader = DirectoryReader::open(writer);
    delete writer;
  }

  shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(reader);
  std::deque<bool> expectedLiveDocs = {true, false, false, true, true, true};
  for (int i = 0; i < expectedLiveDocs.size(); i++) {
    TestUtil::assertEquals(expectedLiveDocs[i], liveDocs->get(i));
  }

  std::deque<int64_t> expectedValues = {1, 2, 3, 17, 5, 17};
  shared_ptr<BinaryDocValues> bdv =
      MultiDocValues::getBinaryValues(reader, L"val");
  for (int i = 0; i < expectedValues.size(); i++) {
    TestUtil::assertEquals(i, bdv->nextDoc());
    TestUtil::assertEquals(expectedValues[i], getValue(bdv));
  }

  reader->close();
  delete dir;
}

void TestBinaryDocValuesUpdates::testUpdatesWithDeletes() 
{
  // update and delete different documents in the same commit session
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMaxBufferedDocs(10); // control segment flushing
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  writer->addDocument(doc(0));
  writer->addDocument(doc(1));

  if (random()->nextBoolean()) {
    writer->commit();
  }

  writer->deleteDocuments({make_shared<Term>(L"id", L"doc-0")});
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-1"), L"val",
                               toBytes(17LL));

  shared_ptr<DirectoryReader> *const reader;
  if (random()->nextBoolean()) { // not NRT
    delete writer;
    reader = DirectoryReader::open(dir);
  } else { // NRT
    reader = DirectoryReader::open(writer);
    delete writer;
  }

  shared_ptr<LeafReader> r = reader->leaves()->get(0).reader();
  assertFalse(r->getLiveDocs()->get(0));
  shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"val");
  TestUtil::assertEquals(1, bdv->advance(1));
  TestUtil::assertEquals(17, getValue(bdv));

  reader->close();
  delete dir;
}

void TestBinaryDocValuesUpdates::testMultipleDocValuesTypes() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMaxBufferedDocs(10); // prevent merges
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  for (int i = 0; i < 4; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"dvUpdateKey", L"dv", Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv", i));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<BinaryDocValuesField>(
        L"bdv", make_shared<BytesRef>(Integer::toString(i))));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<SortedDocValuesField>(
        L"sdv", make_shared<BytesRef>(Integer::toString(i))));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<SortedSetDocValuesField>(
        L"ssdv", make_shared<BytesRef>(Integer::toString(i))));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<SortedSetDocValuesField>(
        L"ssdv", make_shared<BytesRef>(Integer::toString(i * 2))));
    writer->addDocument(doc);
  }
  writer->commit();

  // update all docs' bdv field
  writer->updateBinaryDocValue(make_shared<Term>(L"dvUpdateKey", L"dv"), L"bdv",
                               toBytes(17LL));
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> r = reader->leaves()->get(0).reader();
  shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"ndv");
  shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"bdv");
  shared_ptr<SortedDocValues> sdv = r->getSortedDocValues(L"sdv");
  shared_ptr<SortedSetDocValues> ssdv = r->getSortedSetDocValues(L"ssdv");
  for (int i = 0; i < r->maxDoc(); i++) {
    TestUtil::assertEquals(i, ndv->nextDoc());
    TestUtil::assertEquals(i, ndv->longValue());
    TestUtil::assertEquals(i, bdv->nextDoc());
    TestUtil::assertEquals(17, getValue(bdv));
    TestUtil::assertEquals(i, sdv->nextDoc());
    shared_ptr<BytesRef> term = sdv->binaryValue();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(make_shared<BytesRef>(Integer::toString(i)), term);
    TestUtil::assertEquals(i, ssdv->nextDoc());
    int64_t ord = ssdv->nextOrd();
    term = ssdv->lookupOrd(ord);
    TestUtil::assertEquals(i, stoi(term->utf8ToString()));
    // For the i=0 case, we added the same value twice, which was dedup'd by
    // IndexWriter so it has only one value:
    if (i != 0) {
      ord = ssdv->nextOrd();
      term = ssdv->lookupOrd(ord);
      TestUtil::assertEquals(i * 2, stoi(term->utf8ToString()));
    }
    TestUtil::assertEquals(SortedSetDocValues::NO_MORE_ORDS, ssdv->nextOrd());
  }

  reader->close();
  delete dir;
}

void TestBinaryDocValuesUpdates::testMultipleBinaryDocValues() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMaxBufferedDocs(10); // prevent merges
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"dvUpdateKey", L"dv", Store::NO));
    doc->push_back(make_shared<BinaryDocValuesField>(L"bdv1", toBytes(i)));
    doc->push_back(make_shared<BinaryDocValuesField>(L"bdv2", toBytes(i)));
    writer->addDocument(doc);
  }
  writer->commit();

  // update all docs' bdv1 field
  writer->updateBinaryDocValue(make_shared<Term>(L"dvUpdateKey", L"dv"),
                               L"bdv1", toBytes(17LL));
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> r = reader->leaves()->get(0).reader();

  shared_ptr<BinaryDocValues> bdv1 = r->getBinaryDocValues(L"bdv1");
  shared_ptr<BinaryDocValues> bdv2 = r->getBinaryDocValues(L"bdv2");
  for (int i = 0; i < r->maxDoc(); i++) {
    TestUtil::assertEquals(i, bdv1->nextDoc());
    TestUtil::assertEquals(17, getValue(bdv1));
    TestUtil::assertEquals(i, bdv2->nextDoc());
    TestUtil::assertEquals(i, getValue(bdv2));
  }

  reader->close();
  delete dir;
}

void TestBinaryDocValuesUpdates::testDocumentWithNoValue() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"dvUpdateKey", L"dv", Store::NO));
    if (i == 0) { // index only one document with value
      doc->push_back(make_shared<BinaryDocValuesField>(L"bdv", toBytes(5LL)));
    }
    writer->addDocument(doc);
  }
  writer->commit();

  // update all docs' bdv field
  writer->updateBinaryDocValue(make_shared<Term>(L"dvUpdateKey", L"dv"), L"bdv",
                               toBytes(17LL));
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> r = reader->leaves()->get(0).reader();
  shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"bdv");
  for (int i = 0; i < r->maxDoc(); i++) {
    TestUtil::assertEquals(i, bdv->nextDoc());
    TestUtil::assertEquals(17, getValue(bdv));
  }

  reader->close();
  delete dir;
}

void TestBinaryDocValuesUpdates::testUpdateNonBinaryDocValuesField() throw(
    runtime_error)
{
  // we don't support adding new fields or updating existing non-binary-dv
  // fields through binary updates
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"key", L"doc", Store::NO));
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Store::NO));
  writer->addDocument(doc); // flushed document
  writer->commit();
  writer->addDocument(doc); // in-memory document

  expectThrows(invalid_argument::typeid, [&]() {
    writer->updateBinaryDocValue(make_shared<Term>(L"key", L"doc"), L"bdv",
                                 toBytes(17LL));
  });

  expectThrows(invalid_argument::typeid, [&]() {
    writer->updateBinaryDocValue(make_shared<Term>(L"key", L"doc"), L"foo",
                                 toBytes(17LL));
  });

  delete writer;
  delete dir;
}

void TestBinaryDocValuesUpdates::testDifferentDVFormatPerField() throw(
    runtime_error)
{
  // test relies on separate instances of "same thing"
  assert(TestUtil::getDefaultDocValuesFormat() !=
         TestUtil::getDefaultDocValuesFormat());
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setCodec(
      make_shared<AssertingCodecAnonymousInnerClass>(shared_from_this()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"key", L"doc", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"bdv", toBytes(5LL)));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"sorted", make_shared<BytesRef>(L"value")));
  writer->addDocument(doc); // flushed document
  writer->commit();
  writer->addDocument(doc); // in-memory document

  writer->updateBinaryDocValue(make_shared<Term>(L"key", L"doc"), L"bdv",
                               toBytes(17LL));
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);

  shared_ptr<BinaryDocValues> bdv =
      MultiDocValues::getBinaryValues(reader, L"bdv");
  shared_ptr<SortedDocValues> sdv =
      MultiDocValues::getSortedValues(reader, L"sorted");
  for (int i = 0; i < reader->maxDoc(); i++) {
    TestUtil::assertEquals(i, bdv->nextDoc());
    TestUtil::assertEquals(17, getValue(bdv));
    TestUtil::assertEquals(i, sdv->nextDoc());
    shared_ptr<BytesRef> term = sdv->binaryValue();
    TestUtil::assertEquals(make_shared<BytesRef>(L"value"), term);
  }

  reader->close();
  delete dir;
}

TestBinaryDocValuesUpdates::AssertingCodecAnonymousInnerClass::
    AssertingCodecAnonymousInnerClass(
        shared_ptr<TestBinaryDocValuesUpdates> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<DocValuesFormat>
TestBinaryDocValuesUpdates::AssertingCodecAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return TestUtil::getDefaultDocValuesFormat();
}

void TestBinaryDocValuesUpdates::testUpdateSameDocMultipleTimes() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"key", L"doc", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"bdv", toBytes(5LL)));
  writer->addDocument(doc); // flushed document
  writer->commit();
  writer->addDocument(doc); // in-memory document

  writer->updateBinaryDocValue(make_shared<Term>(L"key", L"doc"), L"bdv",
                               toBytes(17LL)); // update existing field
  writer->updateBinaryDocValue(
      make_shared<Term>(L"key", L"doc"), L"bdv",
      toBytes(3LL)); // update existing field 2nd time in this commit
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<BinaryDocValues> bdv =
      MultiDocValues::getBinaryValues(reader, L"bdv");
  for (int i = 0; i < reader->maxDoc(); i++) {
    TestUtil::assertEquals(i, bdv->nextDoc());
    TestUtil::assertEquals(3, getValue(bdv));
  }
  reader->close();
  delete dir;
}

void TestBinaryDocValuesUpdates::testSegmentMerges() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> random = TestBinaryDocValuesUpdates::random();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  int docid = 0;
  int numRounds = atLeast(10);
  for (int rnd = 0; rnd < numRounds; rnd++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"key", L"doc", Store::NO));
    doc->push_back(make_shared<BinaryDocValuesField>(L"bdv", toBytes(-1)));
    int numDocs = atLeast(30);
    for (int i = 0; i < numDocs; i++) {
      doc->removeField(L"id");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<StringField>(L"id", Integer::toString(docid++),
                                              Store::NO));
      writer->addDocument(doc);
    }

    int64_t value = rnd + 1;
    writer->updateBinaryDocValue(make_shared<Term>(L"key", L"doc"), L"bdv",
                                 toBytes(value));

    if (random->nextDouble() < 0.2) { // randomly delete one doc
      // C++ TODO: There is no native C++ equivalent to 'toString':
      writer->deleteDocuments({make_shared<Term>(
          L"id", Integer::toString(random->nextInt(docid)))});
    }

    // randomly commit or reopen-IW (or nothing), before forceMerge
    if (random->nextDouble() < 0.4) {
      writer->commit();
    } else if (random->nextDouble() < 0.1) {
      delete writer;
      conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random));
      writer = make_shared<IndexWriter>(dir, conf);
    }

    // add another document with the current value, to be sure forceMerge has
    // something to merge (for instance, it could be that CMS finished merging
    // all segments down to 1 before the delete was applied, so when
    // forceMerge is called, the index will be with one segment and deletes
    // and some MPs might now merge it, thereby invalidating test's
    // assumption that the reader has no deletes).
    doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        make_shared<StringField>(L"id", Integer::toString(docid++), Store::NO));
    doc->push_back(make_shared<StringField>(L"key", L"doc", Store::NO));
    doc->push_back(make_shared<BinaryDocValuesField>(L"bdv", toBytes(value)));
    writer->addDocument(doc);

    writer->forceMerge(1, true);
    shared_ptr<DirectoryReader> *const reader;
    if (random->nextBoolean()) {
      writer->commit();
      reader = DirectoryReader::open(dir);
    } else {
      reader = DirectoryReader::open(writer);
    }

    TestUtil::assertEquals(1, reader->leaves()->size());
    shared_ptr<LeafReader> *const r = reader->leaves()->get(0).reader();
    assertNull(L"index should have no deletes after forceMerge",
               r->getLiveDocs());
    shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"bdv");
    assertNotNull(bdv);
    for (int i = 0; i < r->maxDoc(); i++) {
      TestUtil::assertEquals(i, bdv->nextDoc());
      TestUtil::assertEquals(value, getValue(bdv));
    }
    reader->close();
  }

  delete writer;
  delete dir;
}

void TestBinaryDocValuesUpdates::testUpdateDocumentByMultipleTerms() throw(
    runtime_error)
{
  // make sure the order of updates is respected, even when multiple terms
  // affect same document
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"k1", L"v1", Store::NO));
  doc->push_back(make_shared<StringField>(L"k2", L"v2", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"bdv", toBytes(5LL)));
  writer->addDocument(doc); // flushed document
  writer->commit();
  writer->addDocument(doc); // in-memory document

  writer->updateBinaryDocValue(make_shared<Term>(L"k1", L"v1"), L"bdv",
                               toBytes(17LL));
  writer->updateBinaryDocValue(make_shared<Term>(L"k2", L"v2"), L"bdv",
                               toBytes(3LL));
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<BinaryDocValues> bdv =
      MultiDocValues::getBinaryValues(reader, L"bdv");
  for (int i = 0; i < reader->maxDoc(); i++) {
    TestUtil::assertEquals(i, bdv->nextDoc());
    TestUtil::assertEquals(3, getValue(bdv));
  }
  reader->close();
  delete dir;
}

TestBinaryDocValuesUpdates::OneSortDoc::OneSortDoc(int id,
                                                   shared_ptr<BytesRef> value,
                                                   int64_t sortValue)
    : sortValue(sortValue), id(id)
{
  this->value = value;
}

int TestBinaryDocValuesUpdates::OneSortDoc::compareTo(
    shared_ptr<OneSortDoc> other)
{
  int cmp = Long::compare(sortValue, other->sortValue);
  if (cmp == 0) {
    cmp = Integer::compare(id, other->id);
    assert(cmp != 0);
  }
  return cmp;
}

void TestBinaryDocValuesUpdates::testSortedIndex() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexSort(make_shared<Sort>(
      make_shared<SortField>(L"sort", SortField::Type::LONG)));
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int valueRange = TestUtil::nextInt(random(), 1, 1000);
  int sortValueRange = TestUtil::nextInt(random(), 1, 1000);

  int refreshChance = TestUtil::nextInt(random(), 5, 200);
  int deleteChance = TestUtil::nextInt(random(), 2, 100);

  int idUpto = 0;
  int deletedCount = 0;

  deque<std::shared_ptr<OneSortDoc>> docs =
      deque<std::shared_ptr<OneSortDoc>>();
  shared_ptr<DirectoryReader> r = w->getReader();

  int numIters = atLeast(1000);
  for (int iter = 0; iter < numIters; iter++) {
    shared_ptr<BytesRef> value =
        toBytes(static_cast<int64_t>(random()->nextInt(valueRange)));
    if (docs.empty() || random()->nextInt(3) == 1) {
      int id = docs.size();
      // add new doc
      shared_ptr<Document> doc = make_shared<Document>();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(newStringField(L"id", Integer::toString(id), Store::YES));
      doc->push_back(make_shared<BinaryDocValuesField>(L"number", value));
      int sortValue = random()->nextInt(sortValueRange);
      doc->push_back(make_shared<NumericDocValuesField>(L"sort", sortValue));
      if (VERBOSE) {
        wcout << L"TEST: iter=" << iter << L" add doc id=" << id
              << L" sortValue=" << sortValue << L" value=" << value << endl;
      }
      w->addDocument(doc);

      docs.push_back(make_shared<OneSortDoc>(id, value, sortValue));
    } else {
      // update existing doc value
      int idToUpdate = random()->nextInt(docs.size());
      if (VERBOSE) {
        wcout << L"TEST: iter=" << iter << L" update doc id=" << idToUpdate
              << L" new value=" << value << endl;
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      w->updateBinaryDocValue(
          make_shared<Term>(L"id", Integer::toString(idToUpdate)), L"number",
          value);

      docs[idToUpdate]->value = value;
    }

    if (random()->nextInt(deleteChance) == 0) {
      int idToDelete = random()->nextInt(docs.size());
      if (VERBOSE) {
        wcout << L"TEST: delete doc id=" << idToDelete << endl;
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      w->deleteDocuments(
          make_shared<Term>(L"id", Integer::toString(idToDelete)));
      if (docs[idToDelete]->deleted == false) {
        docs[idToDelete]->deleted = true;
        deletedCount++;
      }
    }

    if (random()->nextInt(refreshChance) == 0) {
      if (VERBOSE) {
        wcout << L"TEST: now get reader; old reader=" << r << endl;
      }
      shared_ptr<DirectoryReader> r2 = w->getReader();
      r->close();
      r = r2;

      if (VERBOSE) {
        wcout << L"TEST: got reader=" << r << endl;
      }

      int liveCount = 0;

      for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
        shared_ptr<LeafReader> leafReader = ctx->reader();
        shared_ptr<BinaryDocValues> values =
            leafReader->getBinaryDocValues(L"number");
        shared_ptr<NumericDocValues> sortValues =
            leafReader->getNumericDocValues(L"sort");
        shared_ptr<Bits> liveDocs = leafReader->getLiveDocs();

        int64_t lastSortValue = numeric_limits<int64_t>::min();
        for (int i = 0; i < leafReader->maxDoc(); i++) {

          shared_ptr<Document> doc = leafReader->document(i);
          shared_ptr<OneSortDoc> sortDoc =
              docs[static_cast<Integer>(doc[L"id"])];

          TestUtil::assertEquals(i, values->nextDoc());
          TestUtil::assertEquals(i, sortValues->nextDoc());

          if (liveDocs != nullptr && liveDocs->get(i) == false) {
            assertTrue(sortDoc->deleted);
            continue;
          }
          assertFalse(sortDoc->deleted);

          TestUtil::assertEquals(sortDoc->value, values->binaryValue());

          int64_t sortValue = sortValues->longValue();
          TestUtil::assertEquals(sortDoc->sortValue, sortValue);

          assertTrue(sortValue >= lastSortValue);
          lastSortValue = sortValue;
          liveCount++;
        }
      }

      TestUtil::assertEquals(docs.size() - deletedCount, liveCount);
    }
  }

  IOUtils::close({r, w, dir});
}

void TestBinaryDocValuesUpdates::testManyReopensAndFields() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> *const random = TestBinaryDocValuesUpdates::random();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random));
  shared_ptr<LogMergePolicy> lmp = newLogMergePolicy();
  lmp->setMergeFactor(3); // merge often
  conf->setMergePolicy(lmp);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  constexpr bool isNRT = random->nextBoolean();
  shared_ptr<DirectoryReader> reader;
  if (isNRT) {
    reader = DirectoryReader::open(writer);
  } else {
    writer->commit();
    reader = DirectoryReader::open(dir);
  }
  // System.out.println("TEST: isNRT=" + isNRT);

  constexpr int numFields = random->nextInt(4) + 3; // 3-7
  const std::deque<int64_t> fieldValues = std::deque<int64_t>(numFields);
  for (int i = 0; i < fieldValues.size(); i++) {
    fieldValues[i] = 1;
  }

  int numRounds = atLeast(15);
  int docID = 0;
  for (int i = 0; i < numRounds; i++) {
    int numDocs = atLeast(5);
    // System.out.println("[" + Thread.currentThread().getName() + "]: round=" +
    // i + ", numDocs=" + numDocs);
    for (int j = 0; j < numDocs; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<StringField>(
          L"id", L"doc-" + to_wstring(docID), Store::NO));
      doc->push_back(
          make_shared<StringField>(L"key", L"all", Store::NO)); // update key
      // add all fields with their current value
      for (int f = 0; f < fieldValues.size(); f++) {
        doc->push_back(make_shared<BinaryDocValuesField>(
            L"f" + to_wstring(f), toBytes(fieldValues[f])));
      }
      writer->addDocument(doc);
      ++docID;
    }

    int fieldIdx = random->nextInt(fieldValues.size());
    wstring updateField = L"f" + to_wstring(fieldIdx);
    //      System.out.println("[" + Thread.currentThread().getName() + "]:
    //      updated field '" + updateField + "' to value " +
    //      fieldValues[fieldIdx]);
    writer->updateBinaryDocValue(make_shared<Term>(L"key", L"all"), updateField,
                                 toBytes(++fieldValues[fieldIdx]));

    if (random->nextDouble() < 0.2) {
      int deleteDoc = random->nextInt(
          docID); // might also delete an already deleted document, ok!
      writer->deleteDocuments(
          {make_shared<Term>(L"id", L"doc-" + to_wstring(deleteDoc))});
      //        System.out.println("[" + Thread.currentThread().getName() + "]:
      //        deleted document: doc-" + deleteDoc);
    }

    // verify reader
    if (!isNRT) {
      writer->commit();
    }

    //      System.out.println("[" + Thread.currentThread().getName() + "]:
    //      reopen reader: " + reader);
    shared_ptr<DirectoryReader> newReader =
        DirectoryReader::openIfChanged(reader);
    assertNotNull(newReader);
    reader->close();
    reader = newReader;
    //      System.out.println("[" + Thread.currentThread().getName() + "]:
    //      reopened reader: " + reader);
    assertTrue(reader->numDocs() >
               0); // we delete at most one document per round
    for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
      shared_ptr<LeafReader> r = context->reader();
      //        System.out.println(((SegmentReader) r).getSegmentName());
      shared_ptr<Bits> liveDocs = r->getLiveDocs();
      for (int field = 0; field < fieldValues.size(); field++) {
        wstring f = L"f" + to_wstring(field);
        shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(f);
        assertNotNull(bdv);
        int maxDoc = r->maxDoc();
        for (int doc = 0; doc < maxDoc; doc++) {
          if (liveDocs == nullptr || liveDocs->get(doc)) {
            TestUtil::assertEquals(doc, bdv->advance(doc));
            assertEquals(L"invalid value for doc=" + to_wstring(doc) +
                             L", field=" + f + L", reader=" + r,
                         fieldValues[field], getValue(bdv));
          }
        }
      }
    }
    //      System.out.println();
  }

  delete writer;
  IOUtils::close({reader, dir});
}

void TestBinaryDocValuesUpdates::testUpdateSegmentWithNoDocValues() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // prevent merges, otherwise by the time updates are applied
  // (writer.close()), the segments might have merged and that update becomes
  // legit.
  conf->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  // first segment with BDV
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc0", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"bdv", toBytes(3LL)));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(
      L"id", L"doc4", Store::NO)); // document without 'bdv' field
  writer->addDocument(doc);
  writer->commit();

  // second segment with no BDV
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc1", Store::NO));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(
      L"id", L"doc2", Store::NO)); // document that isn't updated
  writer->addDocument(doc);
  writer->commit();

  // update document in the first segment - should not affect docsWithField of
  // the document without BDV field
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc0"), L"bdv",
                               toBytes(5LL));

  // update document in the second segment - field should be added and we should
  // be able to handle the other document correctly (e.g. no NPE)
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc1"), L"bdv",
                               toBytes(5LL));
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"bdv");
    TestUtil::assertEquals(0, bdv->nextDoc());
    TestUtil::assertEquals(5LL, getValue(bdv));
    TestUtil::assertEquals(NO_MORE_DOCS, bdv->nextDoc());
  }
  reader->close();

  delete dir;
}

void TestBinaryDocValuesUpdates::
    testUpdateSegmentWithPostingButNoDocValues() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // prevent merges, otherwise by the time updates are applied
  // (writer.close()), the segments might have merged and that update becomes
  // legit.
  conf->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  // first segment with BDV
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc0", Store::NO));
  doc->push_back(make_shared<StringField>(L"bdv", L"mock-value", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"bdv", toBytes(5LL)));
  writer->addDocument(doc);
  writer->commit();

  // second segment with no BDV
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc1", Store::NO));
  doc->push_back(make_shared<StringField>(L"bdv", L"mock-value", Store::NO));
  writer->addDocument(doc);
  writer->commit();

  // update document in the second segment
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc1"), L"bdv",
                               toBytes(5LL));
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"bdv");
    for (int i = 0; i < r->maxDoc(); i++) {
      TestUtil::assertEquals(i, bdv->nextDoc());
      TestUtil::assertEquals(5LL, getValue(bdv));
    }
  }
  reader->close();

  delete dir;
}

void TestBinaryDocValuesUpdates::
    testUpdateBinaryDVFieldWithSameNameAsPostingField() 
{
  // this used to fail because FieldInfos.Builder neglected to update
  // globalFieldMaps.docValuesTypes map_obj
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"f", L"mock-value", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f", toBytes(5LL)));
  writer->addDocument(doc);
  writer->commit();
  writer->updateBinaryDocValue(make_shared<Term>(L"f", L"mock-value"), L"f",
                               toBytes(17LL));
  delete writer;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  shared_ptr<BinaryDocValues> bdv =
      r->leaves()->get(0).reader().getBinaryDocValues(L"f");
  TestUtil::assertEquals(0, bdv->nextDoc());
  TestUtil::assertEquals(17, getValue(bdv));
  r->close();

  delete dir;
}

void TestBinaryDocValuesUpdates::testStressMultiThreading() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(dir, conf);

  // create index
  constexpr int numFields = TestUtil::nextInt(random(), 1, 4);
  constexpr int numDocs = atLeast(2000);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<StringField>(L"id", L"doc" + to_wstring(i), Store::NO));
    double group = random()->nextDouble();
    wstring g;
    if (group < 0.1) {
      g = L"g0";
    } else if (group < 0.5) {
      g = L"g1";
    } else if (group < 0.8) {
      g = L"g2";
    } else {
      g = L"g3";
    }
    doc->push_back(make_shared<StringField>(L"updKey", g, Store::NO));
    for (int j = 0; j < numFields; j++) {
      int64_t value = random()->nextInt();
      doc->push_back(make_shared<BinaryDocValuesField>(L"f" + to_wstring(j),
                                                       toBytes(value)));
      doc->push_back(make_shared<BinaryDocValuesField>(
          L"cf" + to_wstring(j),
          toBytes(value * 2))); // control, always updated to f * 2
    }
    writer->addDocument(doc);
  }

  constexpr int numThreads = TestUtil::nextInt(random(), 3, 6);
  shared_ptr<CountDownLatch> *const done =
      make_shared<CountDownLatch>(numThreads);
  shared_ptr<AtomicInteger> *const numUpdates =
      make_shared<AtomicInteger>(atLeast(100));

  // same thread updates a field as well as reopens
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), L"UpdateThread-" + to_wstring(i), writer, numFields,
        numDocs, done, numUpdates);
  }

  for (auto t : threads) {
    t->start();
  }
  done->await();
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    for (int i = 0; i < numFields; i++) {
      shared_ptr<BinaryDocValues> bdv =
          r->getBinaryDocValues(L"f" + to_wstring(i));
      shared_ptr<BinaryDocValues> control =
          r->getBinaryDocValues(L"cf" + to_wstring(i));
      shared_ptr<Bits> liveDocs = r->getLiveDocs();
      for (int j = 0; j < r->maxDoc(); j++) {
        if (liveDocs == nullptr || liveDocs->get(j)) {
          TestUtil::assertEquals(j, bdv->advance(j));
          TestUtil::assertEquals(j, control->advance(j));
          TestUtil::assertEquals(getValue(control), getValue(bdv) * 2);
        }
      }
    }
  }
  reader->close();

  delete dir;
}

TestBinaryDocValuesUpdates::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestBinaryDocValuesUpdates> outerInstance,
        wstring L"UpdateThread-" + i,
        shared_ptr<org::apache::lucene::index::IndexWriter> writer,
        int numFields, int numDocs, shared_ptr<CountDownLatch> done,
        shared_ptr<AtomicInteger> numUpdates)
    : Thread(L"UpdateThread-" + i)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
  this->numFields = numFields;
  this->numDocs = numDocs;
  this->done = done;
  this->numUpdates = numUpdates;
}

void TestBinaryDocValuesUpdates::ThreadAnonymousInnerClass::run()
{
  shared_ptr<DirectoryReader> reader = nullptr;
  bool success = false;
  try {
    shared_ptr<Random> random = TestBinaryDocValuesUpdates::random();
    while (numUpdates->getAndDecrement() > 0) {
      double group = random->nextDouble();
      shared_ptr<Term> t;
      if (group < 0.1) {
        t = make_shared<Term>(L"updKey", L"g0");
      } else if (group < 0.5) {
        t = make_shared<Term>(L"updKey", L"g1");
      } else if (group < 0.8) {
        t = make_shared<Term>(L"updKey", L"g2");
      } else {
        t = make_shared<Term>(L"updKey", L"g3");
      }

      constexpr int field =
          TestBinaryDocValuesUpdates::random()->nextInt(numFields);
      const wstring f = L"f" + to_wstring(field);
      const wstring cf = L"cf" + to_wstring(field);
      //              System.out.println("[" + Thread.currentThread().getName()
      //              + "] numUpdates=" + numUpdates + " updateTerm=" + t + "
      //              field=" + field);
      int64_t updValue = random->nextInt();
      writer->updateDocValues(
          t, {make_shared<BinaryDocValuesField>(f, toBytes(updValue)),
              make_shared<BinaryDocValuesField>(cf, toBytes(updValue * 2))});

      if (random->nextDouble() < 0.2) {
        // delete a random document
        int doc = random->nextInt(numDocs);
        //                System.out.println("[" +
        //                Thread.currentThread().getName() + "] deleteDoc=doc" +
        //                doc);
        writer->deleteDocuments(
            {make_shared<Term>(L"id", L"doc" + to_wstring(doc))});
      }

      if (random->nextDouble() < 0.05) { // commit every 20 updates on average
        //                  System.out.println("[" +
        //                  Thread.currentThread().getName() + "] commit");
        writer->commit();
      }

      if (random->nextDouble() < 0.1) { // reopen NRT reader (apply updates), on
                                        // average once every 10 updates
        if (reader == nullptr) {
          //                  System.out.println("[" +
          //                  Thread.currentThread().getName() + "] open NRT");
          reader = DirectoryReader::open(writer);
        } else {
          //                  System.out.println("[" +
          //                  Thread.currentThread().getName() + "] reopen
          //                  NRT");
          shared_ptr<DirectoryReader> r2 =
              DirectoryReader::openIfChanged(reader, writer);
          if (r2 != nullptr) {
            reader->close();
            reader = r2;
          }
        }
      }
    }
    //            System.out.println("[" + Thread.currentThread().getName() + "]
    //            DONE");
    success = true;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      try {
        reader->close();
      } catch (const IOException &e) {
        if (success) { // suppress this exception only if there was another
                       // exception
          throw runtime_error(e);
        }
      }
    }
    done->countDown();
  }
}

void TestBinaryDocValuesUpdates::testUpdateDifferentDocsInDifferentGens() throw(
    runtime_error)
{
  // update same document multiple times across generations
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMaxBufferedDocs(4);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  constexpr int numDocs = atLeast(10);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<StringField>(L"id", L"doc" + to_wstring(i), Store::NO));
    int64_t value = random()->nextInt();
    doc->push_back(make_shared<BinaryDocValuesField>(L"f", toBytes(value)));
    doc->push_back(
        make_shared<BinaryDocValuesField>(L"cf", toBytes(value * 2)));
    writer->addDocument(doc);
  }

  int numGens = atLeast(5);
  for (int i = 0; i < numGens; i++) {
    int doc = random()->nextInt(numDocs);
    shared_ptr<Term> t = make_shared<Term>(L"id", L"doc" + to_wstring(doc));
    int64_t value = random()->nextLong();
    writer->updateDocValues(
        t, {make_shared<BinaryDocValuesField>(L"f", toBytes(value)),
            make_shared<BinaryDocValuesField>(L"cf", toBytes(value * 2))});
    shared_ptr<DirectoryReader> reader = DirectoryReader::open(writer);
    for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
      shared_ptr<LeafReader> r = context->reader();
      shared_ptr<BinaryDocValues> fbdv = r->getBinaryDocValues(L"f");
      shared_ptr<BinaryDocValues> cfbdv = r->getBinaryDocValues(L"cf");
      for (int j = 0; j < r->maxDoc(); j++) {
        TestUtil::assertEquals(j, fbdv->nextDoc());
        TestUtil::assertEquals(j, cfbdv->nextDoc());
        TestUtil::assertEquals(getValue(cfbdv), getValue(fbdv) * 2);
      }
    }
    reader->close();
  }
  delete writer;
  delete dir;
}

void TestBinaryDocValuesUpdates::testChangeCodec() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMergePolicy(
      NoMergePolicy::INSTANCE); // disable merges to simplify test assertions.
  conf->setCodec(
      make_shared<AssertingCodecAnonymousInnerClass>(shared_from_this()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"d0", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f1", toBytes(5LL)));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f2", toBytes(13LL)));
  writer->addDocument(doc);
  delete writer;

  // change format
  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMergePolicy(
      NoMergePolicy::INSTANCE); // disable merges to simplify test assertions.
  conf->setCodec(
      make_shared<AssertingCodecAnonymousInnerClass2>(shared_from_this()));
  writer = make_shared<IndexWriter>(dir, conf);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"d1", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f1", toBytes(17LL)));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f2", toBytes(2LL)));
  writer->addDocument(doc);
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"d0"), L"f1",
                               toBytes(12LL));
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  shared_ptr<BinaryDocValues> f1 =
      MultiDocValues::getBinaryValues(reader, L"f1");
  shared_ptr<BinaryDocValues> f2 =
      MultiDocValues::getBinaryValues(reader, L"f2");
  TestUtil::assertEquals(0, f1->nextDoc());
  TestUtil::assertEquals(0, f2->nextDoc());
  TestUtil::assertEquals(12LL, getValue(f1));
  TestUtil::assertEquals(13LL, getValue(f2));
  TestUtil::assertEquals(1, f1->nextDoc());
  TestUtil::assertEquals(1, f2->nextDoc());
  TestUtil::assertEquals(17LL, getValue(f1));
  TestUtil::assertEquals(2LL, getValue(f2));
  reader->close();
  delete dir;
}

TestBinaryDocValuesUpdates::AssertingCodecAnonymousInnerClass::
    AssertingCodecAnonymousInnerClass(
        shared_ptr<TestBinaryDocValuesUpdates> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<DocValuesFormat>
TestBinaryDocValuesUpdates::AssertingCodecAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return TestUtil::getDefaultDocValuesFormat();
}

TestBinaryDocValuesUpdates::AssertingCodecAnonymousInnerClass2::
    AssertingCodecAnonymousInnerClass2(
        shared_ptr<TestBinaryDocValuesUpdates> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<DocValuesFormat>
TestBinaryDocValuesUpdates::AssertingCodecAnonymousInnerClass2::
    getDocValuesFormatForField(const wstring &field)
{
  return make_shared<AssertingDocValuesFormat>();
}

void TestBinaryDocValuesUpdates::testAddIndexes() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir1, conf);

  constexpr int numDocs = atLeast(50);
  constexpr int numTerms = TestUtil::nextInt(random(), 1, numDocs / 5);
  shared_ptr<Set<wstring>> randomTerms = unordered_set<wstring>();
  while (randomTerms->size() < numTerms) {
    randomTerms->add(TestUtil::randomSimpleString(random()));
  }

  // create first index
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(
        L"id", RandomPicks::randomFrom(random(), randomTerms), Store::NO));
    doc->push_back(make_shared<BinaryDocValuesField>(L"bdv", toBytes(4LL)));
    doc->push_back(make_shared<BinaryDocValuesField>(L"control", toBytes(8LL)));
    writer->addDocument(doc);
  }

  if (random()->nextBoolean()) {
    writer->commit();
  }

  // update some docs to a random value
  int64_t value = random()->nextInt();
  shared_ptr<Term> term =
      make_shared<Term>(L"id", RandomPicks::randomFrom(random(), randomTerms));
  writer->updateDocValues(
      term,
      {make_shared<BinaryDocValuesField>(L"bdv", toBytes(value)),
       make_shared<BinaryDocValuesField>(L"control", toBytes(value * 2))});
  delete writer;

  shared_ptr<Directory> dir2 = newDirectory();
  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  writer = make_shared<IndexWriter>(dir2, conf);
  if (random()->nextBoolean()) {
    writer->addIndexes({dir1});
  } else {
    shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir1);
    TestUtil::addIndexesSlowly(writer, {reader});
    reader->close();
  }
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir2);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"bdv");
    shared_ptr<BinaryDocValues> control = r->getBinaryDocValues(L"control");
    for (int i = 0; i < r->maxDoc(); i++) {
      TestUtil::assertEquals(i, bdv->nextDoc());
      TestUtil::assertEquals(i, control->nextDoc());
      TestUtil::assertEquals(getValue(bdv) * 2, getValue(control));
    }
  }
  reader->close();

  IOUtils::close({dir1, dir2});
}

void TestBinaryDocValuesUpdates::testDeleteUnusedUpdatesFiles() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"d0", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f1", toBytes(1LL)));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f2", toBytes(1LL)));
  writer->addDocument(doc);

  // update each field twice to make sure all unneeded files are deleted
  for (auto f : std::deque<wstring>{L"f1", L"f2"}) {
    writer->updateBinaryDocValue(make_shared<Term>(L"id", L"d0"), f,
                                 toBytes(2LL));
    writer->commit();
    int numFiles = dir->listAll().size();

    // update again, number of files shouldn't change (old field's gen is
    // removed)
    writer->updateBinaryDocValue(make_shared<Term>(L"id", L"d0"), f,
                                 toBytes(3LL));
    writer->commit();

    TestUtil::assertEquals(numFiles, dir->listAll().size());
  }

  delete writer;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testTonsOfUpdates() throws Exception
void TestBinaryDocValuesUpdates::testTonsOfUpdates() 
{
  // LUCENE-5248: make sure that when there are many updates, we don't use too
  // much RAM
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> *const random = TestBinaryDocValuesUpdates::random();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random));
  conf->setRAMBufferSizeMB(IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB);
  conf->setMaxBufferedDocs(
      IndexWriterConfig::DISABLE_AUTO_FLUSH); // don't flush by doc
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  // test data: lots of documents (few 10Ks) and lots of update terms (few
  // hundreds)
  constexpr int numDocs = atLeast(20000);
  constexpr int numBinaryFields = atLeast(5);
  constexpr int numTerms =
      TestUtil::nextInt(random, 10, 100); // terms should affect many docs
  shared_ptr<Set<wstring>> updateTerms = unordered_set<wstring>();
  while (updateTerms->size() < numTerms) {
    updateTerms->add(TestUtil::randomSimpleString(random));
  }

  //    System.out.println("numDocs=" + numDocs + " numBinaryFields=" +
  //    numBinaryFields + " numTerms=" + numTerms);

  // build a large index with many BDV fields and update terms
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int numUpdateTerms = TestUtil::nextInt(random, 1, numTerms / 10);
    for (int j = 0; j < numUpdateTerms; j++) {
      doc->push_back(make_shared<StringField>(
          L"upd", RandomPicks::randomFrom(random, updateTerms), Store::NO));
    }
    for (int j = 0; j < numBinaryFields; j++) {
      int64_t val = random->nextInt();
      doc->push_back(make_shared<BinaryDocValuesField>(L"f" + to_wstring(j),
                                                       toBytes(val)));
      doc->push_back(make_shared<BinaryDocValuesField>(L"cf" + to_wstring(j),
                                                       toBytes(val * 2)));
    }
    writer->addDocument(doc);
  }

  writer->commit(); // commit so there's something to apply to

  // set to flush every 2048 bytes (approximately every 12 updates), so we get
  // many flushes during binary updates
  writer->getConfig()->setRAMBufferSizeMB(2048.0 / 1024 / 1024);
  constexpr int numUpdates = atLeast(100);
  //    System.out.println("numUpdates=" + numUpdates);
  for (int i = 0; i < numUpdates; i++) {
    int field = random->nextInt(numBinaryFields);
    shared_ptr<Term> updateTerm =
        make_shared<Term>(L"upd", RandomPicks::randomFrom(random, updateTerms));
    int64_t value = random->nextInt();
    writer->updateDocValues(
        updateTerm, {make_shared<BinaryDocValuesField>(L"f" + to_wstring(field),
                                                       toBytes(value)),
                     make_shared<BinaryDocValuesField>(
                         L"cf" + to_wstring(field), toBytes(value * 2))});
  }

  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    for (int i = 0; i < numBinaryFields; i++) {
      shared_ptr<LeafReader> r = context->reader();
      shared_ptr<BinaryDocValues> f =
          r->getBinaryDocValues(L"f" + to_wstring(i));
      shared_ptr<BinaryDocValues> cf =
          r->getBinaryDocValues(L"cf" + to_wstring(i));
      for (int j = 0; j < r->maxDoc(); j++) {
        TestUtil::assertEquals(j, f->nextDoc());
        TestUtil::assertEquals(j, cf->nextDoc());
        assertEquals(L"reader=" + r + L", field=f" + to_wstring(i) + L", doc=" +
                         to_wstring(j),
                     getValue(cf), getValue(f) * 2);
      }
    }
  }
  reader->close();

  delete dir;
}

void TestBinaryDocValuesUpdates::testUpdatesOrder() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"upd", L"t1", Store::NO));
  doc->push_back(make_shared<StringField>(L"upd", L"t2", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f1", toBytes(1LL)));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f2", toBytes(1LL)));
  writer->addDocument(doc);
  writer->updateBinaryDocValue(make_shared<Term>(L"upd", L"t1"), L"f1",
                               toBytes(2LL)); // update f1 to 2
  writer->updateBinaryDocValue(make_shared<Term>(L"upd", L"t1"), L"f2",
                               toBytes(2LL)); // update f2 to 2
  writer->updateBinaryDocValue(make_shared<Term>(L"upd", L"t2"), L"f1",
                               toBytes(3LL)); // update f1 to 3
  writer->updateBinaryDocValue(make_shared<Term>(L"upd", L"t2"), L"f2",
                               toBytes(3LL)); // update f2 to 3
  writer->updateBinaryDocValue(make_shared<Term>(L"upd", L"t1"), L"f1",
                               toBytes(4LL)); // update f1 to 4 (but not f2)
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  shared_ptr<BinaryDocValues> bdv =
      reader->leaves()->get(0).reader().getBinaryDocValues(L"f1");
  TestUtil::assertEquals(0, bdv->nextDoc());
  TestUtil::assertEquals(4, getValue(bdv));
  bdv = reader->leaves()->get(0).reader().getBinaryDocValues(L"f2");
  TestUtil::assertEquals(0, bdv->nextDoc());
  TestUtil::assertEquals(3, getValue(bdv));
  reader->close();

  delete dir;
}

void TestBinaryDocValuesUpdates::testUpdateAllDeletedSegment() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f1", toBytes(1LL)));
  writer->addDocument(doc);
  writer->addDocument(doc);
  writer->commit();
  writer->deleteDocuments({make_shared<Term>(
      L"id", L"doc")}); // delete all docs in the first segment
  writer->addDocument(doc);
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc"), L"f1",
                               toBytes(2LL));
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, reader->leaves()->size());
  shared_ptr<BinaryDocValues> bdv =
      reader->leaves()->get(0).reader().getBinaryDocValues(L"f1");
  TestUtil::assertEquals(0, bdv->nextDoc());
  TestUtil::assertEquals(2LL, getValue(bdv));
  reader->close();

  delete dir;
}

void TestBinaryDocValuesUpdates::testUpdateTwoNonexistingTerms() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc", Store::NO));
  doc->push_back(make_shared<BinaryDocValuesField>(L"f1", toBytes(1LL)));
  writer->addDocument(doc);
  // update w/ multiple nonexisting terms in same field
  writer->updateBinaryDocValue(make_shared<Term>(L"c", L"foo"), L"f1",
                               toBytes(2LL));
  writer->updateBinaryDocValue(make_shared<Term>(L"c", L"bar"), L"f1",
                               toBytes(2LL));
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, reader->leaves()->size());
  shared_ptr<BinaryDocValues> bdv =
      reader->leaves()->get(0).reader().getBinaryDocValues(L"f1");
  TestUtil::assertEquals(0, bdv->nextDoc());
  TestUtil::assertEquals(1LL, getValue(bdv));
  reader->close();

  delete dir;
}

void TestBinaryDocValuesUpdates::testIOContext() 
{
  // LUCENE-5591: make sure we pass an IOContext with an approximate
  // segmentSize in FlushInfo
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // we want a single large enough segment so that a doc-values update writes a
  // large file
  conf->setMergePolicy(NoMergePolicy::INSTANCE);
  conf->setMaxBufferedDocs(numeric_limits<int>::max()); // manually flush
  conf->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  for (int i = 0; i < 100; i++) {
    writer->addDocument(doc(i));
  }
  writer->commit();
  delete writer;

  shared_ptr<NRTCachingDirectory> cachingDir =
      make_shared<NRTCachingDirectory>(dir, 100, 1 / (1024.0 * 1024.0));
  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // we want a single large enough segment so that a doc-values update writes a
  // large file
  conf->setMergePolicy(NoMergePolicy::INSTANCE);
  conf->setMaxBufferedDocs(numeric_limits<int>::max()); // manually flush
  conf->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH);
  writer = make_shared<IndexWriter>(cachingDir, conf);
  writer->updateBinaryDocValue(make_shared<Term>(L"id", L"doc-0"), L"val",
                               toBytes(100LL));
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(writer); // flush
  TestUtil::assertEquals(0, cachingDir->listCachedFiles().size());

  IOUtils::close({reader, writer, cachingDir});
}
} // namespace org::apache::lucene::index