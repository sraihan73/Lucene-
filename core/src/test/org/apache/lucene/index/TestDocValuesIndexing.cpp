using namespace std;

#include "TestDocValuesIndexing.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDocValuesIndexing::testAddIndexes() 
{
  shared_ptr<Directory> d1 = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), d1);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 1));
  w->addDocument(doc);
  shared_ptr<DirectoryReader> r1 = w->getReader();
  delete w;

  shared_ptr<Directory> d2 = newDirectory();
  w = make_shared<RandomIndexWriter>(random(), d2);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"2", Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 2));
  w->addDocument(doc);
  shared_ptr<DirectoryReader> r2 = w->getReader();
  delete w;

  shared_ptr<Directory> d3 = newDirectory();
  w = make_shared<RandomIndexWriter>(random(), d3);
  w->addIndexes({SlowCodecReaderWrapper::wrap(getOnlyLeafReader(r1)),
                 SlowCodecReaderWrapper::wrap(getOnlyLeafReader(r2))});
  r1->close();
  delete d1;
  r2->close();
  delete d2;

  w->forceMerge(1);
  shared_ptr<DirectoryReader> r3 = w->getReader();
  delete w;
  shared_ptr<LeafReader> sr = getOnlyLeafReader(r3);
  TestUtil::assertEquals(2, sr->numDocs());
  shared_ptr<NumericDocValues> docValues = sr->getNumericDocValues(L"dv");
  assertNotNull(docValues);
  r3->close();
  delete d3;
}

void TestDocValuesIndexing::testMultiValuedDocValuesField() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = make_shared<NumericDocValuesField>(L"field", 17);
  doc->push_back(f);

  // add the doc
  w->addDocument(doc);

  // Index doc values are single-valued so we should not
  // be able to add same field more than once:
  doc->push_back(f);
  expectThrows(invalid_argument::typeid, [&]() {
    w->addDocument(doc);
    fail(L"didn't hit expected exception");
  });

  shared_ptr<DirectoryReader> r = w->getReader();
  delete w;
  shared_ptr<NumericDocValues> values =
      DocValues::getNumeric(getOnlyLeafReader(r), L"field");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(17, values->longValue());
  r->close();
  delete d;
}

void TestDocValuesIndexing::testDifferentTypedDocValuesField() throw(
    runtime_error)
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"field", 17));
  w->addDocument(doc);

  // Index doc values are single-valued so we should not
  // be able to add same field more than once:
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"field", make_shared<BytesRef>(L"blah")));
  expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });

  shared_ptr<DirectoryReader> r = w->getReader();
  delete w;
  shared_ptr<NumericDocValues> values =
      DocValues::getNumeric(getOnlyLeafReader(r), L"field");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(17, values->longValue());
  r->close();
  delete d;
}

void TestDocValuesIndexing::testDifferentTypedDocValuesField2() throw(
    runtime_error)
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"field", 17));
  w->addDocument(doc);

  // Index doc values are single-valued so we should not
  // be able to add same field more than once:
  doc->push_back(make_shared<SortedDocValuesField>(
      L"field", make_shared<BytesRef>(L"hello")));
  expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });

  shared_ptr<DirectoryReader> r = w->getReader();
  shared_ptr<NumericDocValues> values =
      DocValues::getNumeric(getOnlyLeafReader(r), L"field");
  TestUtil::assertEquals(0, values->nextDoc());
  TestUtil::assertEquals(17, values->longValue());
  r->close();
  delete w;
  delete d;
}

void TestDocValuesIndexing::testLengthPrefixAcrossTwoPages() throw(
    runtime_error)
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      d, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  std::deque<char> bytes(32764);
  shared_ptr<BytesRef> b = make_shared<BytesRef>();
  b->bytes = bytes;
  b->length = bytes.size();
  doc->push_back(make_shared<SortedDocValuesField>(L"field", b));
  w->addDocument(doc);
  bytes[0] = 1;
  w->addDocument(doc);
  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = w->getReader();
  shared_ptr<BinaryDocValues> s =
      DocValues::getBinary(getOnlyLeafReader(r), L"field");
  TestUtil::assertEquals(0, s->nextDoc());
  shared_ptr<BytesRef> bytes1 = s->binaryValue();
  TestUtil::assertEquals(bytes.size(), bytes1->length);
  bytes[0] = 0;
  TestUtil::assertEquals(b, bytes1);

  TestUtil::assertEquals(1, s->nextDoc());
  bytes1 = s->binaryValue();
  TestUtil::assertEquals(bytes.size(), bytes1->length);
  bytes[0] = 1;
  TestUtil::assertEquals(b, bytes1);
  r->close();
  delete w;
  delete d;
}

void TestDocValuesIndexing::testDocValuesUnstored() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwconfig =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwconfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwconfig);
  for (int i = 0; i < 50; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
    doc->push_back(
        make_shared<TextField>(L"docId", L"" + to_wstring(i), Store::YES));
    writer->addDocument(doc);
  }
  shared_ptr<DirectoryReader> r = writer->getReader();
  shared_ptr<FieldInfos> fi = MultiFields::getMergedFieldInfos(r);
  shared_ptr<FieldInfo> dvInfo = fi->fieldInfo(L"dv");
  assertTrue(dvInfo->getDocValuesType() != DocValuesType::NONE);
  shared_ptr<NumericDocValues> dv = MultiDocValues::getNumericValues(r, L"dv");
  for (int i = 0; i < 50; i++) {
    TestUtil::assertEquals(i, dv->nextDoc());
    TestUtil::assertEquals(i, dv->longValue());
    shared_ptr<Document> d = r->document(i);
    // cannot use d.get("dv") due to another bug!
    assertNull(d->getField(L"dv"));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(Integer::toString(i), d[L"docId"]);
  }
  r->close();
  delete writer;
  delete dir;
}

void TestDocValuesIndexing::testMixedTypesSameDocument() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  w->addDocument(make_shared<Document>());

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 0));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"foo", make_shared<BytesRef>(L"hello")));
  expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });

  shared_ptr<IndexReader> ir = w->getReader();
  TestUtil::assertEquals(1, ir->numDocs());
  delete ir;
  delete w;
  delete dir;
}

void TestDocValuesIndexing::testMixedTypesDifferentDocuments() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 0));
  w->addDocument(doc);

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<SortedDocValuesField>(
      L"foo", make_shared<BytesRef>(L"hello")));
  expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc2); });

  shared_ptr<IndexReader> ir = w->getReader();
  TestUtil::assertEquals(1, ir->numDocs());
  delete ir;
  delete w;
  delete dir;
}

void TestDocValuesIndexing::testAddSortedTwice() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  // we don't use RandomIndexWriter because it might add more docvalues than we
  // expect !!!!1
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> iwriter = make_shared<IndexWriter>(directory, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo!")));
  iwriter->addDocument(doc);

  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"bar!")));
  expectThrows(invalid_argument::typeid, [&]() { iwriter->addDocument(doc); });

  shared_ptr<IndexReader> ir = iwriter->getReader();
  TestUtil::assertEquals(1, ir->numDocs());
  delete ir;
  delete iwriter;
  delete directory;
}

void TestDocValuesIndexing::testAddBinaryTwice() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  // we don't use RandomIndexWriter because it might add more docvalues than we
  // expect !!!!1
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> iwriter = make_shared<IndexWriter>(directory, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv", make_shared<BytesRef>(L"foo!")));
  iwriter->addDocument(doc);

  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dv", make_shared<BytesRef>(L"bar!")));
  expectThrows(invalid_argument::typeid, [&]() { iwriter->addDocument(doc); });

  shared_ptr<IndexReader> ir = iwriter->getReader();
  TestUtil::assertEquals(1, ir->numDocs());
  delete ir;

  delete iwriter;
  delete directory;
}

void TestDocValuesIndexing::testAddNumericTwice() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  // we don't use RandomIndexWriter because it might add more docvalues than we
  // expect !!!!1
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> iwriter = make_shared<IndexWriter>(directory, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 1));
  iwriter->addDocument(doc);

  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 2));
  expectThrows(invalid_argument::typeid, [&]() { iwriter->addDocument(doc); });

  shared_ptr<IndexReader> ir = iwriter->getReader();
  TestUtil::assertEquals(1, ir->numDocs());
  delete ir;
  delete iwriter;
  delete directory;
}

void TestDocValuesIndexing::testTooLargeSortedBytes() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  // we don't use RandomIndexWriter because it might add more docvalues than we
  // expect !!!!1
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> iwriter = make_shared<IndexWriter>(directory, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"dv", make_shared<BytesRef>(L"just fine")));
  iwriter->addDocument(doc);

  shared_ptr<Document> hugeDoc = make_shared<Document>();
  std::deque<char> bytes(100000);
  shared_ptr<BytesRef> b = make_shared<BytesRef>(bytes);
  random()->nextBytes(bytes);
  hugeDoc->push_back(make_shared<SortedDocValuesField>(L"dv", b));
  expectThrows(invalid_argument::typeid,
               [&]() { iwriter->addDocument(hugeDoc); });

  shared_ptr<IndexReader> ir = iwriter->getReader();
  TestUtil::assertEquals(1, ir->numDocs());
  delete ir;
  delete iwriter;
  delete directory;
}

void TestDocValuesIndexing::testTooLargeTermSortedSetBytes() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<Directory> directory = newDirectory();
  // we don't use RandomIndexWriter because it might add more docvalues than we
  // expect !!!!1
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> iwriter = make_shared<IndexWriter>(directory, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"dv", make_shared<BytesRef>(L"just fine")));
  iwriter->addDocument(doc);

  shared_ptr<Document> hugeDoc = make_shared<Document>();
  std::deque<char> bytes(100000);
  shared_ptr<BytesRef> b = make_shared<BytesRef>(bytes);
  random()->nextBytes(bytes);
  hugeDoc->push_back(make_shared<SortedSetDocValuesField>(L"dv", b));
  expectThrows(invalid_argument::typeid,
               [&]() { iwriter->addDocument(hugeDoc); });

  shared_ptr<IndexReader> ir = iwriter->getReader();
  TestUtil::assertEquals(1, ir->numDocs());
  delete ir;
  delete iwriter;
  delete directory;
}

void TestDocValuesIndexing::testMixedTypesDifferentSegments() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 0));
  w->addDocument(doc);
  w->commit();

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<SortedDocValuesField>(
      L"foo", make_shared<BytesRef>(L"hello")));
  expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc2); });

  delete w;
  delete dir;
}

void TestDocValuesIndexing::testMixedTypesAfterDeleteAll() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 0));
  w->addDocument(doc);
  w->deleteAll();

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"foo", make_shared<BytesRef>(L"hello")));
  w->addDocument(doc);
  delete w;
  delete dir;
}

void TestDocValuesIndexing::testMixedTypesAfterReopenCreate() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 0));
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setOpenMode(IndexWriterConfig::OpenMode::CREATE);
  w = make_shared<IndexWriter>(dir, iwc);
  doc = make_shared<Document>();
  w->addDocument(doc);
  delete w;
  delete dir;
}

void TestDocValuesIndexing::testMixedTypesAfterReopenAppend1() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 0));
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(make_shared<SortedDocValuesField>(
      L"foo", make_shared<BytesRef>(L"hello")));
  expectThrows(invalid_argument::typeid, [&]() { w2->addDocument(doc2); });

  delete w2;
  delete dir;
}

void TestDocValuesIndexing::testMixedTypesAfterReopenAppend2() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"foo")));
  w->addDocument(doc);
  delete w;

  shared_ptr<Document> doc2 = make_shared<Document>();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  doc2->push_back(make_shared<StringField>(L"foo", L"bar", Store::NO));
  doc2->push_back(
      make_shared<BinaryDocValuesField>(L"foo", make_shared<BytesRef>(L"foo")));
  // NOTE: this case follows a different code path inside
  // DefaultIndexingChain/FieldInfos, because the field (foo)
  // is first added without DocValues:
  expectThrows(invalid_argument::typeid, [&]() { w2->addDocument(doc2); });

  w2->forceMerge(1);
  delete w2;
  delete dir;
}

void TestDocValuesIndexing::testMixedTypesAfterReopenAppend3() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"foo", make_shared<BytesRef>(L"foo")));
  w->addDocument(doc);
  delete w;

  shared_ptr<Document> doc2 = make_shared<Document>();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  doc2->push_back(make_shared<StringField>(L"foo", L"bar", Store::NO));
  doc2->push_back(
      make_shared<BinaryDocValuesField>(L"foo", make_shared<BytesRef>(L"foo")));
  // NOTE: this case follows a different code path inside
  // DefaultIndexingChain/FieldInfos, because the field (foo)
  // is first added without DocValues:
  expectThrows(invalid_argument::typeid, [&]() { w2->addDocument(doc2); });

  // Also add another document so there is a segment to write here:
  w2->addDocument(make_shared<Document>());
  w2->forceMerge(1);
  delete w2;
  delete dir;
}

void TestDocValuesIndexing::testMixedTypesDifferentThreads() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  shared_ptr<AtomicBoolean> *const hitExc = make_shared<AtomicBoolean>();
  std::deque<std::shared_ptr<Thread>> threads(3);
  for (int i = 0; i < 3; i++) {
    shared_ptr<Field> field;
    if (i == 0) {
      field = make_shared<SortedDocValuesField>(
          L"foo", make_shared<BytesRef>(L"hello"));
    } else if (i == 1) {
      field = make_shared<NumericDocValuesField>(L"foo", 0);
    } else {
      field = make_shared<BinaryDocValuesField>(L"foo",
                                                make_shared<BytesRef>(L"bazz"));
    }
    shared_ptr<Document> *const doc = make_shared<Document>();
    doc->push_back(field);

    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), w, startingGun, hitExc, doc);
    threads[i]->start();
  }

  startingGun->countDown();

  for (auto t : threads) {
    t->join();
  }
  assertTrue(hitExc->get());
  delete w;
  delete dir;
}

TestDocValuesIndexing::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestDocValuesIndexing> outerInstance,
    shared_ptr<org::apache::lucene::index::IndexWriter> w,
    shared_ptr<CountDownLatch> startingGun, shared_ptr<AtomicBoolean> hitExc,
    shared_ptr<Document> doc)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->startingGun = startingGun;
  this->hitExc = hitExc;
  this->doc = doc;
}

void TestDocValuesIndexing::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    w->addDocument(doc);
  } catch (const invalid_argument &iae) {
    // expected
    hitExc->set(true);
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void TestDocValuesIndexing::testMixedTypesViaAddIndexes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"foo", 0));
  w->addDocument(doc);

  // Make 2nd index w/ inconsistent field
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"foo", make_shared<BytesRef>(L"hello")));
  w2->addDocument(doc);
  delete w2;

  expectThrows(invalid_argument::typeid, [&]() {
    w->addIndexes(std::deque<std::shared_ptr<Directory>>{dir2});
  });

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir2);
  expectThrows(invalid_argument::typeid,
               [&]() { TestUtil::addIndexesSlowly(w, {r}); });

  r->close();
  delete dir2;
  delete w;
  delete dir;
}

void TestDocValuesIndexing::testIllegalTypeChange() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  expectThrows(invalid_argument::typeid, [&]() { writer->addDocument(doc2); });

  shared_ptr<IndexReader> ir = writer->getReader();
  TestUtil::assertEquals(1, ir->numDocs());
  delete ir;
  delete writer;
  delete dir;
}

void TestDocValuesIndexing::testIllegalTypeChangeAcrossSegments() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  delete writer;

  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer2 = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  expectThrows(invalid_argument::typeid, [&]() { writer2->addDocument(doc2); });

  delete writer2;
  delete dir;
}

void TestDocValuesIndexing::testTypeChangeAfterCloseAndDeleteAll() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  delete writer;

  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  writer = make_shared<IndexWriter>(dir, conf);
  writer->deleteAll();
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  writer->addDocument(doc);
  delete writer;
  delete dir;
}

void TestDocValuesIndexing::testTypeChangeAfterDeleteAll() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  writer->deleteAll();
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  writer->addDocument(doc);
  delete writer;
  delete dir;
}

void TestDocValuesIndexing::testTypeChangeAfterCommitAndDeleteAll() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  writer->commit();
  writer->deleteAll();
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  writer->addDocument(doc);
  delete writer;
  delete dir;
}

void TestDocValuesIndexing::testTypeChangeAfterOpenCreate() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  delete writer;
  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setOpenMode(IndexWriterConfig::OpenMode::CREATE);
  writer = make_shared<IndexWriter>(dir, conf);
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  writer->addDocument(doc);
  delete writer;
  delete dir;
}

void TestDocValuesIndexing::testTypeChangeViaAddIndexes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<Directory> dir2 = newDirectory();
  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer2 = make_shared<IndexWriter>(dir2, conf);
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  writer2->addDocument(doc);
  expectThrows(invalid_argument::typeid, [&]() { writer2->addIndexes({dir}); });
  delete writer2;

  delete dir;
  delete dir2;
}

void TestDocValuesIndexing::testTypeChangeViaAddIndexesIR() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<Directory> dir2 = newDirectory();
  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer2 = make_shared<IndexWriter>(dir2, conf);
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  writer2->addDocument(doc);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  expectThrows(invalid_argument::typeid,
               [&]() { TestUtil::addIndexesSlowly(writer2, {reader}); });

  reader->close();
  delete writer2;

  delete dir;
  delete dir2;
}

void TestDocValuesIndexing::testTypeChangeViaAddIndexes2() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<Directory> dir2 = newDirectory();
  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer2 = make_shared<IndexWriter>(dir2, conf);
  writer2->addIndexes({dir});
  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  expectThrows(invalid_argument::typeid, [&]() { writer2->addDocument(doc2); });

  delete writer2;
  delete dir2;
  delete dir;
}

void TestDocValuesIndexing::testTypeChangeViaAddIndexesIR2() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<Directory> dir2 = newDirectory();
  conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer2 = make_shared<IndexWriter>(dir2, conf);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  TestUtil::addIndexesSlowly(writer2, {reader});
  reader->close();
  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(
      make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>(L"foo")));
  expectThrows(invalid_argument::typeid, [&]() { writer2->addDocument(doc2); });

  delete writer2;
  delete dir2;
  delete dir;
}

void TestDocValuesIndexing::testDocsWithField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"dv", L"some text", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 0LL));
  writer->addDocument(doc);

  shared_ptr<DirectoryReader> r = writer->getReader();
  delete writer;

  shared_ptr<LeafReader> subR = r->leaves()->get(0).reader();
  TestUtil::assertEquals(2, subR->numDocs());

  r->close();
  delete dir;
}

void TestDocValuesIndexing::testSameFieldNameForPostingAndDocValue() throw(
    runtime_error)
{
  // LUCENE-5192: FieldInfos.Builder neglected to update
  // globalFieldNumbers.docValuesType map_obj if the field existed, resulting in
  // potentially adding the same field with different DV types.
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"f", L"mock-value", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"f", 5));
  writer->addDocument(doc);
  writer->commit();

  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(
      make_shared<BinaryDocValuesField>(L"f", make_shared<BytesRef>(L"mock")));
  expectThrows(invalid_argument::typeid, [&]() { writer->addDocument(doc2); });
  writer->rollback();

  delete dir;
}

void TestDocValuesIndexing::testExcIndexingDocBeforeDocValues() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setDocValuesType(DocValuesType::SORTED);
  ft->freeze();
  shared_ptr<Field> field = make_shared<Field>(L"test", L"value", ft);
  field->setTokenStream(
      make_shared<TokenStreamAnonymousInnerClass>(shared_from_this()));
  doc->push_back(field);
  expectThrows(runtime_error::typeid, [&]() { w->addDocument(doc); });

  w->addDocument(make_shared<Document>());
  delete w;
  delete dir;
}

TestDocValuesIndexing::TokenStreamAnonymousInnerClass::
    TokenStreamAnonymousInnerClass(
        shared_ptr<TestDocValuesIndexing> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestDocValuesIndexing::TokenStreamAnonymousInnerClass::incrementToken()
{
  throw runtime_error(L"no");
}
} // namespace org::apache::lucene::index