using namespace std;

#include "TestNumericDocValuesUpdates.h"

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
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using Directory = org::apache::lucene::store::Directory;
using NRTCachingDirectory = org::apache::lucene::store::NRTCachingDirectory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;

shared_ptr<Document> TestNumericDocValuesUpdates::doc(int id)
{
  // make sure we don't set the doc's value to 0, to not confuse with a document
  // that's missing values
  return doc(id, id + 1);
}

shared_ptr<Document> TestNumericDocValuesUpdates::doc(int id, int64_t val)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<StringField>(L"id", L"doc-" + to_wstring(id), Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"val", val));
  return doc;
}

void TestNumericDocValuesUpdates::testMultipleUpdatesSameDoc() throw(
    runtime_error)
{

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));

  conf->setMaxBufferedDocs(
      3); // small number of docs, so use a tiny maxBufferedDocs

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  writer->updateDocument(make_shared<Term>(L"id", L"doc-1"),
                         doc(1, 1000000000LL));
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-1"), L"val",
                                1000001111LL);
  writer->updateDocument(make_shared<Term>(L"id", L"doc-2"),
                         doc(2, 2000000000LL));
  writer->updateDocument(make_shared<Term>(L"id", L"doc-2"),
                         doc(2, 2222222222LL));
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-1"), L"val",
                                1111111111LL);

  shared_ptr<DirectoryReader> *const reader;
  if (random()->nextBoolean()) {
    writer->commit();
    reader = DirectoryReader::open(dir);
  } else {
    reader = DirectoryReader::open(writer);
  }
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);
  shared_ptr<TopFieldDocs> td;

  td = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"id", L"doc-1")), 1,
      make_shared<Sort>(make_shared<SortField>(L"val", SortField::Type::LONG)));
  assertEquals(L"doc-1 missing?", 1, td->scoreDocs.size());
  assertEquals(
      L"doc-1 value", 1111111111LL,
      (std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]))->fields[0]);

  td = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"id", L"doc-2")), 1,
      make_shared<Sort>(make_shared<SortField>(L"val", SortField::Type::LONG)));
  assertEquals(L"doc-2 missing?", 1, td->scoreDocs.size());
  assertEquals(
      L"doc-2 value", 2222222222LL,
      (std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]))->fields[0]);

  IOUtils::close({reader, writer, dir});
}

void TestNumericDocValuesUpdates::testBiasedMixOfRandomUpdates() throw(
    runtime_error)
{
  // 3 types of operations: add, updated, updateDV.
  // rather then randomizing equally, we'll pick (random) cutoffs so each test
  // run is biased, in terms of some ops happen more often then others
  constexpr int ADD_CUTOFF = TestUtil::nextInt(random(), 1, 98);
  constexpr int UPD_CUTOFF = TestUtil::nextInt(random(), ADD_CUTOFF + 1, 99);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  constexpr int numOperations = atLeast(1000);
  const unordered_map<int, int64_t> expected =
      unordered_map<int, int64_t>(numOperations / 3);

  // start with at least one doc before any chance of updates
  constexpr int numSeedDocs = atLeast(1);
  for (int i = 0; i < numSeedDocs; i++) {
    constexpr int64_t val = random()->nextLong();
    expected.emplace(i, val);
    writer->addDocument(doc(i, val));
  }

  int numDocUpdates = 0;
  int numValueUpdates = 0;

  for (int i = 0; i < numOperations; i++) {
    constexpr int op = TestUtil::nextInt(random(), 1, 100);
    constexpr int64_t val = random()->nextLong();
    if (op <= ADD_CUTOFF) {
      constexpr int id = expected.size();
      expected.emplace(id, val);
      writer->addDocument(doc(id, val));
    } else {
      constexpr int id = TestUtil::nextInt(random(), 0, expected.size() - 1);
      expected.emplace(id, val);
      if (op <= UPD_CUTOFF) {
        numDocUpdates++;
        writer->updateDocument(
            make_shared<Term>(L"id", L"doc-" + to_wstring(id)), doc(id, val));
      } else {
        numValueUpdates++;
        writer->updateNumericDocValue(
            make_shared<Term>(L"id", L"doc-" + to_wstring(id)), L"val", val);
      }
    }
  }

  writer->commit();

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);

  // TODO: make more efficient if max numOperations is going to be increased
  // much
  for (auto expect : expected) {
    wstring id = L"doc-" + expect.first;
    shared_ptr<TopFieldDocs> td = searcher->search(
        make_shared<TermQuery>(make_shared<Term>(L"id", id)), 1,
        make_shared<Sort>(
            make_shared<SortField>(L"val", SortField::Type::LONG)));
    assertEquals(id + L" missing?", 1, td->totalHits);
    assertEquals(
        id + L" value", expect.second,
        (std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]))->fields[0]);
  }

  IOUtils::close({reader, writer, dir});
}

void TestNumericDocValuesUpdates::testUpdatesAreFlushed() 
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
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-0"), L"val",
                                5LL);
  TestUtil::assertEquals(2, writer->getFlushDeletesCount());
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-1"), L"val",
                                6LL);
  TestUtil::assertEquals(3, writer->getFlushDeletesCount());
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-2"), L"val",
                                7LL);
  TestUtil::assertEquals(4, writer->getFlushDeletesCount());
  writer->getConfig()->setRAMBufferSizeMB(1000);
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-2"), L"val",
                                7LL);
  TestUtil::assertEquals(4, writer->getFlushDeletesCount());
  delete writer;
  delete dir;
}

void TestNumericDocValuesUpdates::testSimple() 
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
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-0"), L"val",
                                2LL); // doc=0, exp=2

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
  shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"val");
  TestUtil::assertEquals(0, ndv->nextDoc());
  TestUtil::assertEquals(2, ndv->longValue());
  TestUtil::assertEquals(1, ndv->nextDoc());
  TestUtil::assertEquals(2, ndv->longValue());
  reader->close();

  delete dir;
}

void TestNumericDocValuesUpdates::testUpdateFewSegments() 
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
      writer->updateNumericDocValue(
          make_shared<Term>(L"id", L"doc-" + to_wstring(i)), L"val", value);
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
    shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"val");
    assertNotNull(ndv);
    for (int i = 0; i < r->maxDoc(); i++) {
      int64_t expected = expectedValues[i + context->docBase];
      TestUtil::assertEquals(i, ndv->nextDoc());
      int64_t actual = ndv->longValue();
      TestUtil::assertEquals(expected, actual);
    }
  }

  reader->close();
  delete dir;
}

void TestNumericDocValuesUpdates::testReopen() 
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
  if (VERBOSE) {
    wcout << L"TEST: isNRT=" << isNRT << endl;
  }

  // update doc
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-0"), L"val",
                                10LL); // update doc-0's value to 10
  if (!isNRT) {
    writer->commit();
  }

  // reopen reader and assert only it sees the update
  if (VERBOSE) {
    wcout << L"TEST: openIfChanged" << endl;
  }
  shared_ptr<DirectoryReader> *const reader2 =
      DirectoryReader::openIfChanged(reader1);
  assertNotNull(reader2);
  assertTrue(reader1 != reader2);
  shared_ptr<NumericDocValues> dvs1 =
      reader1->leaves()->get(0).reader().getNumericDocValues(L"val");
  TestUtil::assertEquals(0, dvs1->nextDoc());
  TestUtil::assertEquals(1, dvs1->longValue());

  shared_ptr<NumericDocValues> dvs2 =
      reader2->leaves()->get(0).reader().getNumericDocValues(L"val");
  TestUtil::assertEquals(0, dvs2->nextDoc());
  TestUtil::assertEquals(10, dvs2->longValue());

  delete writer;
  IOUtils::close({reader1, reader2, dir});
}

void TestNumericDocValuesUpdates::testUpdatesAndDeletes() 
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
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-3"), L"val",
                                17LL);
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-5"), L"val",
                                17LL);

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
  shared_ptr<NumericDocValues> ndv =
      MultiDocValues::getNumericValues(reader, L"val");
  for (int i = 0; i < expectedValues.size(); i++) {
    TestUtil::assertEquals(i, ndv->nextDoc());
    TestUtil::assertEquals(expectedValues[i], ndv->longValue());
  }

  reader->close();
  delete dir;
}

void TestNumericDocValuesUpdates::testUpdatesWithDeletes() 
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
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-1"), L"val",
                                17LL);

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
  shared_ptr<NumericDocValues> values = r->getNumericDocValues(L"val");
  TestUtil::assertEquals(1, values->advance(1));
  TestUtil::assertEquals(17, values->longValue());

  reader->close();
  delete dir;
}

void TestNumericDocValuesUpdates::testMultipleDocValuesTypes() throw(
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

  // update all docs' ndv field
  writer->updateNumericDocValue(make_shared<Term>(L"dvUpdateKey", L"dv"),
                                L"ndv", 17LL);
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> r = reader->leaves()->get(0).reader();
  shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"ndv");
  shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"bdv");
  shared_ptr<SortedDocValues> sdv = r->getSortedDocValues(L"sdv");
  shared_ptr<SortedSetDocValues> ssdv = r->getSortedSetDocValues(L"ssdv");
  for (int i = 0; i < r->maxDoc(); i++) {
    TestUtil::assertEquals(i, ndv->nextDoc());
    TestUtil::assertEquals(17, ndv->longValue());
    TestUtil::assertEquals(i, bdv->nextDoc());
    shared_ptr<BytesRef> term = bdv->binaryValue();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(make_shared<BytesRef>(Integer::toString(i)), term);
    TestUtil::assertEquals(i, sdv->nextDoc());
    term = sdv->binaryValue();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(make_shared<BytesRef>(Integer::toString(i)), term);
    TestUtil::assertEquals(i, ssdv->nextDoc());

    int64_t ord = ssdv->nextOrd();
    term = ssdv->lookupOrd(ord);
    TestUtil::assertEquals(i, stoi(term->utf8ToString()));
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

void TestNumericDocValuesUpdates::testMultipleNumericDocValues() throw(
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
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv1", i));
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv2", i));
    writer->addDocument(doc);
  }
  writer->commit();

  // update all docs' ndv1 field
  writer->updateNumericDocValue(make_shared<Term>(L"dvUpdateKey", L"dv"),
                                L"ndv1", 17LL);
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> r = reader->leaves()->get(0).reader();
  shared_ptr<NumericDocValues> ndv1 = r->getNumericDocValues(L"ndv1");
  shared_ptr<NumericDocValues> ndv2 = r->getNumericDocValues(L"ndv2");
  for (int i = 0; i < r->maxDoc(); i++) {
    TestUtil::assertEquals(i, ndv1->nextDoc());
    TestUtil::assertEquals(17, ndv1->longValue());
    TestUtil::assertEquals(i, ndv2->nextDoc());
    TestUtil::assertEquals(i, ndv2->longValue());
  }

  reader->close();
  delete dir;
}

void TestNumericDocValuesUpdates::testDocumentWithNoValue() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"dvUpdateKey", L"dv", Store::NO));
    if (i == 0) { // index only one document with value
      doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 5));
    }
    writer->addDocument(doc);
  }
  writer->commit();
  if (VERBOSE) {
    wcout << L"TEST: first commit" << endl;
  }

  // update all docs' ndv field
  writer->updateNumericDocValue(make_shared<Term>(L"dvUpdateKey", L"dv"),
                                L"ndv", 17LL);
  if (VERBOSE) {
    wcout << L"TEST: first close" << endl;
  }
  delete writer;
  if (VERBOSE) {
    wcout << L"TEST: done close" << endl;
  }

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  if (VERBOSE) {
    wcout << L"TEST: got reader=reader" << endl;
  }
  shared_ptr<LeafReader> r = reader->leaves()->get(0).reader();
  shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"ndv");
  for (int i = 0; i < r->maxDoc(); i++) {
    TestUtil::assertEquals(i, ndv->nextDoc());
    assertEquals(L"doc=" + to_wstring(i) + L" has wrong numeric doc value", 17,
                 ndv->longValue());
  }

  reader->close();
  delete dir;
}

void TestNumericDocValuesUpdates::testUpdateNonNumericDocValuesField() throw(
    runtime_error)
{
  // we don't support adding new fields or updating existing non-numeric-dv
  // fields through numeric updates
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
    writer->updateNumericDocValue(make_shared<Term>(L"key", L"doc"), L"ndv",
                                  17LL);
  });

  expectThrows(invalid_argument::typeid, [&]() {
    writer->updateNumericDocValue(make_shared<Term>(L"key", L"doc"), L"foo",
                                  17LL);
  });

  delete writer;
  delete dir;
}

void TestNumericDocValuesUpdates::testDifferentDVFormatPerField() throw(
    runtime_error)
{
  // test relies on separate instances of the "same thing"
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
  doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 5));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"sorted", make_shared<BytesRef>(L"value")));
  writer->addDocument(doc); // flushed document
  writer->commit();
  writer->addDocument(doc); // in-memory document

  writer->updateNumericDocValue(make_shared<Term>(L"key", L"doc"), L"ndv",
                                17LL);
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);

  shared_ptr<NumericDocValues> ndv =
      MultiDocValues::getNumericValues(reader, L"ndv");
  shared_ptr<SortedDocValues> sdv =
      MultiDocValues::getSortedValues(reader, L"sorted");
  for (int i = 0; i < reader->maxDoc(); i++) {
    TestUtil::assertEquals(i, ndv->nextDoc());
    TestUtil::assertEquals(17, ndv->longValue());
    TestUtil::assertEquals(i, sdv->nextDoc());
    shared_ptr<BytesRef> *const term = sdv->binaryValue();
    TestUtil::assertEquals(make_shared<BytesRef>(L"value"), term);
  }

  reader->close();
  delete dir;
}

TestNumericDocValuesUpdates::AssertingCodecAnonymousInnerClass::
    AssertingCodecAnonymousInnerClass(
        shared_ptr<TestNumericDocValuesUpdates> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<DocValuesFormat>
TestNumericDocValuesUpdates::AssertingCodecAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return TestUtil::getDefaultDocValuesFormat();
}

void TestNumericDocValuesUpdates::testUpdateSameDocMultipleTimes() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"key", L"doc", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 5));
  writer->addDocument(doc); // flushed document
  writer->commit();
  writer->addDocument(doc); // in-memory document

  writer->updateNumericDocValue(make_shared<Term>(L"key", L"doc"), L"ndv",
                                17LL); // update existing field
  writer->updateNumericDocValue(
      make_shared<Term>(L"key", L"doc"), L"ndv",
      3LL); // update existing field 2nd time in this commit
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<NumericDocValues> ndv =
      MultiDocValues::getNumericValues(reader, L"ndv");
  for (int i = 0; i < reader->maxDoc(); i++) {
    TestUtil::assertEquals(i, ndv->nextDoc());
    TestUtil::assertEquals(3, ndv->longValue());
  }
  reader->close();
  delete dir;
}

void TestNumericDocValuesUpdates::testSegmentMerges() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> random = TestNumericDocValuesUpdates::random();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  int docid = 0;
  int numRounds = atLeast(10);
  if (VERBOSE) {
    wcout << L"TEST: " << numRounds << L" rounds" << endl;
  }
  for (int rnd = 0; rnd < numRounds; rnd++) {
    if (VERBOSE) {
      wcout << L"\nTEST: round=" << rnd << endl;
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"key", L"doc", Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv", -1));
    int numDocs = atLeast(30);
    if (VERBOSE) {
      wcout << L"TEST: " << numDocs << L" docs" << endl;
    }
    for (int i = 0; i < numDocs; i++) {
      doc->removeField(L"id");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<StringField>(L"id", Integer::toString(docid),
                                              Store::YES));
      if (VERBOSE) {
        wcout << L"TEST: add doc id=" << docid << endl;
      }
      writer->addDocument(doc);
      docid++;
    }

    int64_t value = rnd + 1;
    if (VERBOSE) {
      wcout << L"TEST: update all ndv values to " << value << endl;
    }
    writer->updateNumericDocValue(make_shared<Term>(L"key", L"doc"), L"ndv",
                                  value);

    if (random->nextDouble() < 0.2) { // randomly delete one doc
      int delID = random->nextInt(docid);
      if (VERBOSE) {
        wcout << L"TEST: delete random doc id=" << delID << endl;
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      writer->deleteDocuments(
          {make_shared<Term>(L"id", Integer::toString(delID))});
    }

    // randomly commit or reopen-IW (or nothing), before forceMerge
    if (random->nextDouble() < 0.4) {
      if (VERBOSE) {
        wcout << L"\nTEST: commit writer" << endl;
      }
      writer->commit();
    } else if (random->nextDouble() < 0.1) {
      if (VERBOSE) {
        wcout << L"\nTEST: close writer" << endl;
      }
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
        make_shared<StringField>(L"id", Integer::toString(docid), Store::YES));
    doc->push_back(make_shared<StringField>(L"key", L"doc", Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv", value));
    if (VERBOSE) {
      wcout << L"\nTEST: add one more doc id=" << docid << endl;
    }
    writer->addDocument(doc);
    docid++;

    if (VERBOSE) {
      wcout << L"\nTEST: force merge" << endl;
    }
    writer->forceMerge(1, true);

    shared_ptr<DirectoryReader> *const reader;
    if (random->nextBoolean()) {
      if (VERBOSE) {
        wcout << L"\nTEST: commit and open non-NRT reader" << endl;
      }
      writer->commit();
      reader = DirectoryReader::open(dir);
    } else {
      if (VERBOSE) {
        wcout << L"\nTEST: open NRT reader" << endl;
      }
      reader = DirectoryReader::open(writer);
    }
    if (VERBOSE) {
      wcout << L"TEST: got reader=" << reader << endl;
    }

    TestUtil::assertEquals(1, reader->leaves()->size());
    shared_ptr<LeafReader> *const r = reader->leaves()->get(0).reader();
    assertNull(L"index should have no deletes after forceMerge",
               r->getLiveDocs());
    shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"ndv");
    assertNotNull(ndv);
    if (VERBOSE) {
      wcout << L"TEST: maxDoc=" << r->maxDoc() << endl;
    }
    for (int i = 0; i < r->maxDoc(); i++) {
      shared_ptr<Document> rdoc = r->document(i);
      TestUtil::assertEquals(i, ndv->nextDoc());
      assertEquals(L"docid=" + to_wstring(i) + L" has wrong ndv value; doc=" +
                       rdoc,
                   value, ndv->longValue());
    }
    reader->close();
  }

  delete writer;
  delete dir;
}

void TestNumericDocValuesUpdates::testUpdateDocumentByMultipleTerms() throw(
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
  doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 5));
  writer->addDocument(doc); // flushed document
  writer->commit();
  writer->addDocument(doc); // in-memory document

  writer->updateNumericDocValue(make_shared<Term>(L"k1", L"v1"), L"ndv", 17LL);
  writer->updateNumericDocValue(make_shared<Term>(L"k2", L"v2"), L"ndv", 3LL);
  delete writer;

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  shared_ptr<NumericDocValues> ndv =
      MultiDocValues::getNumericValues(reader, L"ndv");
  for (int i = 0; i < reader->maxDoc(); i++) {
    TestUtil::assertEquals(i, ndv->nextDoc());
    TestUtil::assertEquals(3, ndv->longValue());
  }
  reader->close();
  delete dir;
}

TestNumericDocValuesUpdates::OneSortDoc::OneSortDoc(int id, int64_t value,
                                                    int64_t sortValue)
    : sortValue(sortValue), id(id)
{
  this->value = value;
}

int TestNumericDocValuesUpdates::OneSortDoc::compareTo(
    shared_ptr<OneSortDoc> other)
{
  int cmp = Long::compare(sortValue, other->sortValue);
  if (cmp == 0) {
    cmp = Integer::compare(id, other->id);
    assert(cmp != 0);
  }
  return cmp;
}

void TestNumericDocValuesUpdates::testSortedIndex() 
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
    int value = random()->nextInt(valueRange);
    if (docs.empty() || random()->nextInt(3) == 1) {
      int id = docs.size();
      // add new doc
      shared_ptr<Document> doc = make_shared<Document>();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(newStringField(L"id", Integer::toString(id), Store::YES));
      doc->push_back(make_shared<NumericDocValuesField>(L"number", value));
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
      w->updateNumericDocValue(
          make_shared<Term>(L"id", Integer::toString(idToUpdate)), L"number",
          static_cast<int64_t>(value));

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
        shared_ptr<NumericDocValues> values =
            leafReader->getNumericDocValues(L"number");
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

          TestUtil::assertEquals(sortDoc->value, values->longValue());

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

void TestNumericDocValuesUpdates::testManyReopensAndFields() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> *const random = TestNumericDocValuesUpdates::random();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random));
  shared_ptr<LogMergePolicy> lmp = newLogMergePolicy();
  lmp->setMergeFactor(3); // merge often
  conf->setMergePolicy(lmp);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  constexpr bool isNRT = random->nextBoolean();
  if (VERBOSE) {
    wcout << L"TEST: isNRT=" << isNRT << endl;
  }
  shared_ptr<DirectoryReader> reader;
  if (isNRT) {
    reader = DirectoryReader::open(writer);
  } else {
    writer->commit();
    reader = DirectoryReader::open(dir);
  }

  constexpr int numFields = random->nextInt(4) + 3; // 3-7
  const std::deque<int64_t> fieldValues = std::deque<int64_t>(numFields);
  for (int i = 0; i < fieldValues.size(); i++) {
    fieldValues[i] = 1;
  }

  int numRounds = atLeast(15);
  int docID = 0;
  for (int i = 0; i < numRounds; i++) {
    int numDocs = atLeast(5);
    if (VERBOSE) {
      wcout << L"TEST: round=" << i << L", numDocs=" << numDocs << endl;
    }
    for (int j = 0; j < numDocs; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<StringField>(
          L"id", L"doc-" + to_wstring(docID), Store::YES));
      doc->push_back(
          make_shared<StringField>(L"key", L"all", Store::NO)); // update key
      // add all fields with their current value
      for (int f = 0; f < fieldValues.size(); f++) {
        doc->push_back(make_shared<NumericDocValuesField>(L"f" + to_wstring(f),
                                                          fieldValues[f]));
      }
      writer->addDocument(doc);
      if (VERBOSE) {
        wcout << L"TEST add doc id=" << docID << endl;
      }
      ++docID;
    }

    int fieldIdx = random->nextInt(fieldValues.size());

    wstring updateField = L"f" + to_wstring(fieldIdx);
    if (VERBOSE) {
      wcout << L"TEST: update field=" << updateField
            << L" for all docs to value=" << (fieldValues[fieldIdx] << 1)
            << endl;
    }
    writer->updateNumericDocValue(make_shared<Term>(L"key", L"all"),
                                  updateField, ++fieldValues[fieldIdx]);

    if (random->nextDouble() < 0.2) {
      int deleteDoc = random->nextInt(
          numDocs); // might also delete an already deleted document, ok!
      if (VERBOSE) {
        wcout << L"TEST: delete doc id=" << deleteDoc << endl;
      }
      writer->deleteDocuments(
          {make_shared<Term>(L"id", L"doc-" + to_wstring(deleteDoc))});
    }

    // verify reader
    if (isNRT == false) {
      if (VERBOSE) {
        wcout << L"TEST: now commit" << endl;
      }
      writer->commit();
    }

    shared_ptr<DirectoryReader> newReader =
        DirectoryReader::openIfChanged(reader);
    assertNotNull(newReader);
    reader->close();
    reader = newReader;
    if (VERBOSE) {
      wcout << L"TEST: got reader maxDoc=" << reader->maxDoc() << L" " << reader
            << endl;
    }
    assertTrue(reader->numDocs() >
               0); // we delete at most one document per round
    for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
      shared_ptr<LeafReader> r = context->reader();
      shared_ptr<Bits> liveDocs = r->getLiveDocs();
      for (int field = 0; field < fieldValues.size(); field++) {
        wstring f = L"f" + to_wstring(field);
        shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(f);
        assertNotNull(ndv);
        int maxDoc = r->maxDoc();
        for (int doc = 0; doc < maxDoc; doc++) {
          if (liveDocs == nullptr || liveDocs->get(doc)) {
            assertEquals(L"advanced to wrong doc in seg=" + r, doc,
                         ndv->advance(doc));
            assertEquals(L"invalid value for docID=" + to_wstring(doc) +
                             L" id=" + r->document(doc)[L"id"] + L", field=" +
                             f + L", reader=" + r + L" doc=" + r->document(doc),
                         fieldValues[field], ndv->longValue());
          }
        }
      }
    }
  }

  delete writer;
  IOUtils::close({reader, dir});
}

void TestNumericDocValuesUpdates::testUpdateSegmentWithNoDocValues() throw(
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

  // first segment with NDV
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc0", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 3));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(
      L"id", L"doc4", Store::NO)); // document without 'ndv' field
  writer->addDocument(doc);
  writer->commit();

  // second segment with no NDV
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc1", Store::NO));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(
      L"id", L"doc2", Store::NO)); // document that isn't updated
  writer->addDocument(doc);
  writer->commit();

  // update document in the first segment - should not affect docsWithField of
  // the document without NDV field
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc0"), L"ndv", 5LL);

  // update document in the second segment - field should be added and we should
  // be able to handle the other document correctly (e.g. no NPE)
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc1"), L"ndv", 5LL);
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"ndv");
    TestUtil::assertEquals(0, ndv->nextDoc());
    TestUtil::assertEquals(5LL, ndv->longValue());
    // docID 1 has no ndv value
    assertTrue(ndv->nextDoc() > 1);
  }
  reader->close();

  delete dir;
}

void TestNumericDocValuesUpdates::testUpdateSegmentWithNoDocValues2() throw(
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

  // first segment with NDV
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc0", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 3));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(
      L"id", L"doc4", Store::NO)); // document without 'ndv' field
  writer->addDocument(doc);
  writer->commit();

  // second segment with no NDV, but another dv field "foo"
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc1", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 3));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(
      L"id", L"doc2", Store::NO)); // document that isn't updated
  writer->addDocument(doc);
  writer->commit();

  // update document in the first segment - should not affect docsWithField of
  // the document without NDV field
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc0"), L"ndv", 5LL);

  // update document in the second segment - field should be added and we should
  // be able to handle the other document correctly (e.g. no NPE)
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc1"), L"ndv", 5LL);
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"ndv");
    TestUtil::assertEquals(0, ndv->nextDoc());
    TestUtil::assertEquals(5LL, ndv->longValue());
    assertTrue(ndv->nextDoc() > 1);
  }
  reader->close();

  TestUtil::checkIndex(dir);

  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  writer = make_shared<IndexWriter>(dir, conf);
  writer->forceMerge(1);
  delete writer;

  reader = DirectoryReader::open(dir);
  shared_ptr<LeafReader> ar = getOnlyLeafReader(reader);
  TestUtil::assertEquals(
      DocValuesType::NUMERIC,
      ar->getFieldInfos()->fieldInfo(L"foo")->getDocValuesType());
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  shared_ptr<TopFieldDocs> td;
  // doc0
  td = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"id", L"doc0")), 1,
      make_shared<Sort>(make_shared<SortField>(L"ndv", SortField::Type::LONG)));
  TestUtil::assertEquals(
      5LL, (std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]))->fields[0]);
  // doc1
  td = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"id", L"doc1")), 1,
      make_shared<Sort>(make_shared<SortField>(L"ndv", SortField::Type::LONG),
                        make_shared<SortField>(L"foo", SortField::Type::LONG)));
  TestUtil::assertEquals(
      5LL, (std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]))->fields[0]);
  TestUtil::assertEquals(
      3LL, (std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]))->fields[1]);
  // doc2
  td = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"id", L"doc2")), 1,
      make_shared<Sort>(make_shared<SortField>(L"ndv", SortField::Type::LONG)));
  TestUtil::assertEquals(
      0LL, (std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]))->fields[0]);
  // doc4
  td = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"id", L"doc4")), 1,
      make_shared<Sort>(make_shared<SortField>(L"ndv", SortField::Type::LONG)));
  TestUtil::assertEquals(
      0LL, (std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]))->fields[0]);
  reader->close();

  delete dir;
}

void TestNumericDocValuesUpdates::
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

  // first segment with NDV
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc0", Store::NO));
  doc->push_back(make_shared<StringField>(L"ndv", L"mock-value", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 5));
  writer->addDocument(doc);
  writer->commit();

  // second segment with no NDV
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc1", Store::NO));
  doc->push_back(make_shared<StringField>(L"ndv", L"mock-value", Store::NO));
  writer->addDocument(doc);
  writer->commit();

  // update document in the second segment
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc1"), L"ndv", 5LL);
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"ndv");
    for (int i = 0; i < r->maxDoc(); i++) {
      TestUtil::assertEquals(i, ndv->nextDoc());
      TestUtil::assertEquals(5LL, ndv->longValue());
    }
  }
  reader->close();

  delete dir;
}

void TestNumericDocValuesUpdates::
    testUpdateNumericDVFieldWithSameNameAsPostingField() 
{
  // this used to fail because FieldInfos.Builder neglected to update
  // globalFieldMaps.docValuesTypes map_obj
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"f", L"mock-value", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"f", 5));
  writer->addDocument(doc);
  writer->commit();
  writer->updateNumericDocValue(make_shared<Term>(L"f", L"mock-value"), L"f",
                                17LL);
  delete writer;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  shared_ptr<NumericDocValues> ndv =
      r->leaves()->get(0).reader().getNumericDocValues(L"f");
  TestUtil::assertEquals(0, ndv->nextDoc());
  TestUtil::assertEquals(17, ndv->longValue());
  r->close();

  delete dir;
}

void TestNumericDocValuesUpdates::testStressMultiThreading() throw(
    runtime_error)
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
      doc->push_back(
          make_shared<NumericDocValuesField>(L"f" + to_wstring(j), value));
      doc->push_back(make_shared<NumericDocValuesField>(
          L"cf" + to_wstring(j),
          value * 2)); // control, always updated to f * 2
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
      shared_ptr<NumericDocValues> ndv =
          r->getNumericDocValues(L"f" + to_wstring(i));
      shared_ptr<NumericDocValues> control =
          r->getNumericDocValues(L"cf" + to_wstring(i));
      shared_ptr<Bits> liveDocs = r->getLiveDocs();
      for (int j = 0; j < r->maxDoc(); j++) {
        if (liveDocs == nullptr || liveDocs->get(j)) {
          TestUtil::assertEquals(j, ndv->advance(j));
          TestUtil::assertEquals(j, control->advance(j));
          TestUtil::assertEquals(control->longValue(), ndv->longValue() * 2);
        }
      }
    }
  }
  reader->close();

  delete dir;
}

TestNumericDocValuesUpdates::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestNumericDocValuesUpdates> outerInstance,
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

void TestNumericDocValuesUpdates::ThreadAnonymousInnerClass::run()
{
  shared_ptr<DirectoryReader> reader = nullptr;
  bool success = false;
  try {
    shared_ptr<Random> random = TestNumericDocValuesUpdates::random();
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
          TestNumericDocValuesUpdates::random()->nextInt(numFields);
      const wstring f = L"f" + to_wstring(field);
      const wstring cf = L"cf" + to_wstring(field);
      int64_t updValue = random->nextInt();
      writer->updateDocValues(
          t, {make_shared<NumericDocValuesField>(f, updValue),
              make_shared<NumericDocValuesField>(cf, updValue * 2)});

      if (random->nextDouble() < 0.2) {
        // delete a random document
        int doc = random->nextInt(numDocs);
        writer->deleteDocuments(
            {make_shared<Term>(L"id", L"doc" + to_wstring(doc))});
      }

      if (random->nextDouble() < 0.05) { // commit every 20 updates on average
        writer->commit();
      }

      if (random->nextDouble() < 0.1) { // reopen NRT reader (apply updates), on
                                        // average once every 10 updates
        if (reader == nullptr) {
          reader = DirectoryReader::open(writer);
        } else {
          shared_ptr<DirectoryReader> r2 =
              DirectoryReader::openIfChanged(reader, writer);
          if (r2 != nullptr) {
            reader->close();
            reader = r2;
          }
        }
      }
    }
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

void TestNumericDocValuesUpdates::
    testUpdateDifferentDocsInDifferentGens() 
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
    doc->push_back(make_shared<NumericDocValuesField>(L"f", value));
    doc->push_back(make_shared<NumericDocValuesField>(L"cf", value * 2));
    writer->addDocument(doc);
  }

  int numGens = atLeast(5);
  for (int i = 0; i < numGens; i++) {
    int doc = random()->nextInt(numDocs);
    shared_ptr<Term> t = make_shared<Term>(L"id", L"doc" + to_wstring(doc));
    int64_t value = random()->nextLong();
    writer->updateDocValues(
        t, {make_shared<NumericDocValuesField>(L"f", value),
            make_shared<NumericDocValuesField>(L"cf", value * 2)});
    shared_ptr<DirectoryReader> reader = DirectoryReader::open(writer);
    for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
      shared_ptr<LeafReader> r = context->reader();
      shared_ptr<NumericDocValues> fndv = r->getNumericDocValues(L"f");
      shared_ptr<NumericDocValues> cfndv = r->getNumericDocValues(L"cf");
      for (int j = 0; j < r->maxDoc(); j++) {
        TestUtil::assertEquals(j, fndv->nextDoc());
        TestUtil::assertEquals(j, cfndv->nextDoc());
        TestUtil::assertEquals(cfndv->longValue(), fndv->longValue() * 2);
      }
    }
    reader->close();
  }
  delete writer;
  delete dir;
}

void TestNumericDocValuesUpdates::testChangeCodec() 
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
  doc->push_back(make_shared<NumericDocValuesField>(L"f1", 5LL));
  doc->push_back(make_shared<NumericDocValuesField>(L"f2", 13LL));
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
  doc->push_back(make_shared<NumericDocValuesField>(L"f1", 17LL));
  doc->push_back(make_shared<NumericDocValuesField>(L"f2", 2LL));
  writer->addDocument(doc);
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"d0"), L"f1", 12LL);
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  shared_ptr<NumericDocValues> f1 =
      MultiDocValues::getNumericValues(reader, L"f1");
  shared_ptr<NumericDocValues> f2 =
      MultiDocValues::getNumericValues(reader, L"f2");
  TestUtil::assertEquals(0, f1->nextDoc());
  TestUtil::assertEquals(12LL, f1->longValue());
  TestUtil::assertEquals(0, f2->nextDoc());
  TestUtil::assertEquals(13LL, f2->longValue());
  TestUtil::assertEquals(1, f1->nextDoc());
  TestUtil::assertEquals(17LL, f1->longValue());
  TestUtil::assertEquals(1, f2->nextDoc());
  TestUtil::assertEquals(2LL, f2->longValue());
  reader->close();
  delete dir;
}

TestNumericDocValuesUpdates::AssertingCodecAnonymousInnerClass::
    AssertingCodecAnonymousInnerClass(
        shared_ptr<TestNumericDocValuesUpdates> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<DocValuesFormat>
TestNumericDocValuesUpdates::AssertingCodecAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return TestUtil::getDefaultDocValuesFormat();
}

TestNumericDocValuesUpdates::AssertingCodecAnonymousInnerClass2::
    AssertingCodecAnonymousInnerClass2(
        shared_ptr<TestNumericDocValuesUpdates> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<DocValuesFormat>
TestNumericDocValuesUpdates::AssertingCodecAnonymousInnerClass2::
    getDocValuesFormatForField(const wstring &field)
{
  return make_shared<AssertingDocValuesFormat>();
}

void TestNumericDocValuesUpdates::testAddIndexes() 
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
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv", 4LL));
    doc->push_back(make_shared<NumericDocValuesField>(L"control", 8LL));
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
      term, {make_shared<NumericDocValuesField>(L"ndv", value),
             make_shared<NumericDocValuesField>(L"control", value * 2)});
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
    shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"ndv");
    shared_ptr<NumericDocValues> control = r->getNumericDocValues(L"control");
    for (int i = 0; i < r->maxDoc(); i++) {
      TestUtil::assertEquals(i, ndv->nextDoc());
      TestUtil::assertEquals(i, control->nextDoc());
      TestUtil::assertEquals(ndv->longValue() * 2, control->longValue());
    }
  }
  reader->close();

  IOUtils::close({dir1, dir2});
}

void TestNumericDocValuesUpdates::testDeleteUnusedUpdatesFiles() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"d0", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"f1", 1LL));
  doc->push_back(make_shared<NumericDocValuesField>(L"f2", 1LL));
  writer->addDocument(doc);

  // update each field twice to make sure all unneeded files are deleted
  for (auto f : std::deque<wstring>{L"f1", L"f2"}) {
    writer->updateNumericDocValue(make_shared<Term>(L"id", L"d0"), f, 2LL);
    writer->commit();
    int numFiles = dir->listAll().size();

    // update again, number of files shouldn't change (old field's gen is
    // removed)
    writer->updateNumericDocValue(make_shared<Term>(L"id", L"d0"), f, 3LL);
    writer->commit();

    TestUtil::assertEquals(numFiles, dir->listAll().size());
  }

  delete writer;
  delete dir;
}

void TestNumericDocValuesUpdates::testTonsOfUpdates() 
{
  // LUCENE-5248: make sure that when there are many updates, we don't use too
  // much RAM
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> *const random = TestNumericDocValuesUpdates::random();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random));
  conf->setRAMBufferSizeMB(IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB);
  conf->setMaxBufferedDocs(
      IndexWriterConfig::DISABLE_AUTO_FLUSH); // don't flush by doc
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  // test data: lots of documents (few 10Ks) and lots of update terms (few
  // hundreds)
  constexpr int numDocs = atLeast(20000);
  constexpr int numNumericFields = atLeast(5);
  constexpr int numTerms =
      TestUtil::nextInt(random, 10, 100); // terms should affect many docs
  shared_ptr<Set<wstring>> updateTerms = unordered_set<wstring>();
  while (updateTerms->size() < numTerms) {
    updateTerms->add(TestUtil::randomSimpleString(random));
  }

  // build a large index with many NDV fields and update terms
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int numUpdateTerms = TestUtil::nextInt(random, 1, numTerms / 10);
    for (int j = 0; j < numUpdateTerms; j++) {
      doc->push_back(make_shared<StringField>(
          L"upd", RandomPicks::randomFrom(random, updateTerms), Store::NO));
    }
    for (int j = 0; j < numNumericFields; j++) {
      int64_t val = random->nextInt();
      doc->push_back(
          make_shared<NumericDocValuesField>(L"f" + to_wstring(j), val));
      doc->push_back(
          make_shared<NumericDocValuesField>(L"cf" + to_wstring(j), val * 2));
    }
    writer->addDocument(doc);
  }

  writer->commit(); // commit so there's something to apply to

  // set to flush every 2048 bytes (approximately every 12 updates), so we get
  // many flushes during numeric updates
  writer->getConfig()->setRAMBufferSizeMB(2048.0 / 1024 / 1024);
  constexpr int numUpdates = atLeast(100);
  for (int i = 0; i < numUpdates; i++) {
    int field = random->nextInt(numNumericFields);
    shared_ptr<Term> updateTerm =
        make_shared<Term>(L"upd", RandomPicks::randomFrom(random, updateTerms));
    int64_t value = random->nextInt();
    writer->updateDocValues(
        updateTerm,
        {make_shared<NumericDocValuesField>(L"f" + to_wstring(field), value),
         make_shared<NumericDocValuesField>(L"cf" + to_wstring(field),
                                            value * 2)});
  }

  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    for (int i = 0; i < numNumericFields; i++) {
      shared_ptr<LeafReader> r = context->reader();
      shared_ptr<NumericDocValues> f =
          r->getNumericDocValues(L"f" + to_wstring(i));
      shared_ptr<NumericDocValues> cf =
          r->getNumericDocValues(L"cf" + to_wstring(i));
      for (int j = 0; j < r->maxDoc(); j++) {
        TestUtil::assertEquals(j, f->nextDoc());
        TestUtil::assertEquals(j, cf->nextDoc());
        assertEquals(L"reader=" + r + L", field=f" + to_wstring(i) + L", doc=" +
                         to_wstring(j),
                     cf->longValue(), f->longValue() * 2);
      }
    }
  }
  reader->close();

  delete dir;
}

void TestNumericDocValuesUpdates::testUpdatesOrder() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"upd", L"t1", Store::NO));
  doc->push_back(make_shared<StringField>(L"upd", L"t2", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"f1", 1LL));
  doc->push_back(make_shared<NumericDocValuesField>(L"f2", 1LL));
  writer->addDocument(doc);
  writer->updateNumericDocValue(make_shared<Term>(L"upd", L"t1"), L"f1",
                                2LL); // update f1 to 2
  writer->updateNumericDocValue(make_shared<Term>(L"upd", L"t1"), L"f2",
                                2LL); // update f2 to 2
  writer->updateNumericDocValue(make_shared<Term>(L"upd", L"t2"), L"f1",
                                3LL); // update f1 to 3
  writer->updateNumericDocValue(make_shared<Term>(L"upd", L"t2"), L"f2",
                                3LL); // update f2 to 3
  writer->updateNumericDocValue(make_shared<Term>(L"upd", L"t1"), L"f1",
                                4LL); // update f1 to 4 (but not f2)
  if (VERBOSE) {
    wcout << L"TEST: now close" << endl;
  }
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  shared_ptr<NumericDocValues> dvs =
      reader->leaves()->get(0).reader().getNumericDocValues(L"f1");
  TestUtil::assertEquals(0, dvs->nextDoc());
  TestUtil::assertEquals(4, dvs->longValue());
  dvs = reader->leaves()->get(0).reader().getNumericDocValues(L"f2");
  TestUtil::assertEquals(0, dvs->nextDoc());
  TestUtil::assertEquals(3, dvs->longValue());
  reader->close();

  delete dir;
}

void TestNumericDocValuesUpdates::testUpdateAllDeletedSegment() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"f1", 1LL));
  writer->addDocument(doc);
  writer->addDocument(doc);
  writer->commit();
  writer->deleteDocuments({make_shared<Term>(
      L"id", L"doc")}); // delete all docs in the first segment
  writer->addDocument(doc);
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc"), L"f1", 2LL);
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, reader->leaves()->size());
  shared_ptr<NumericDocValues> dvs =
      reader->leaves()->get(0).reader().getNumericDocValues(L"f1");
  TestUtil::assertEquals(0, dvs->nextDoc());
  TestUtil::assertEquals(2, dvs->longValue());

  reader->close();

  delete dir;
}

void TestNumericDocValuesUpdates::testUpdateTwoNonexistingTerms() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"doc", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"f1", 1LL));
  writer->addDocument(doc);
  // update w/ multiple nonexisting terms in same field
  writer->updateNumericDocValue(make_shared<Term>(L"c", L"foo"), L"f1", 2LL);
  writer->updateNumericDocValue(make_shared<Term>(L"c", L"bar"), L"f1", 2LL);
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, reader->leaves()->size());
  shared_ptr<NumericDocValues> dvs =
      reader->leaves()->get(0).reader().getNumericDocValues(L"f1");
  TestUtil::assertEquals(0, dvs->nextDoc());
  TestUtil::assertEquals(1, dvs->longValue());
  reader->close();

  delete dir;
}

void TestNumericDocValuesUpdates::testIOContext() 
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
  writer->updateNumericDocValue(make_shared<Term>(L"id", L"doc-0"), L"val",
                                100LL);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(writer); // flush
  TestUtil::assertEquals(0, cachingDir->listCachedFiles().size());

  IOUtils::close({reader, writer, cachingDir});
}
} // namespace org::apache::lucene::index