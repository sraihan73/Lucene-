using namespace std;

#include "BaseStoredFieldsFormatTestCase.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using SimpleTextCodec =
    org::apache::lucene::codecs::simpletext::SimpleTextCodec;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using Field = org::apache::lucene::document::Field;
using IntPoint = org::apache::lucene::document::IntPoint;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using MMapDirectory = org::apache::lucene::store::MMapDirectory;
using Throttling = org::apache::lucene::store::MockDirectoryWrapper::Throttling;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using com::carrotsearch::randomizedtesting::generators::RandomStrings;

void BaseStoredFieldsFormatTestCase::addRandomFields(shared_ptr<Document> d)
{
  constexpr int numValues = random()->nextInt(3);
  for (int i = 0; i < numValues; ++i) {
    d->push_back(make_shared<StoredField>(
        L"f", TestUtil::randomSimpleString(random(), 100)));
  }
}

void BaseStoredFieldsFormatTestCase::testRandomStoredFields() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> rand = random();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      rand, dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMaxBufferedDocs(TestUtil::nextInt(rand, 5, 20)));
  // w.w.setNoCFSRatio(0.0);
  constexpr int docCount = atLeast(200);
  constexpr int fieldCount = TestUtil::nextInt(rand, 1, 5);

  const deque<int> fieldIDs = deque<int>();

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setTokenized(false);
  shared_ptr<Field> idField = newField(L"id", L"", customType);

  for (int i = 0; i < fieldCount; i++) {
    fieldIDs.push_back(i);
  }

  const unordered_map<wstring, std::shared_ptr<Document>> docs =
      unordered_map<wstring, std::shared_ptr<Document>>();

  if (VERBOSE) {
    wcout << L"TEST: build index docCount=" << docCount << endl;
  }

  shared_ptr<FieldType> customType2 = make_shared<FieldType>();
  customType2->setStored(true);
  for (int i = 0; i < docCount; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(idField);
    const wstring id = L"" + to_wstring(i);
    idField->setStringValue(id);
    docs.emplace(id, doc);
    if (VERBOSE) {
      wcout << L"TEST: add doc id=" << id << endl;
    }

    for (auto field : fieldIDs) {
      const wstring s;
      if (rand->nextInt(4) != 3) {
        s = TestUtil::randomUnicodeString(rand, 1000);
        doc->push_back(newField(L"f" + to_wstring(field), s, customType2));
      } else {
        s = L"";
      }
    }
    w->addDocument(doc);
    if (rand->nextInt(50) == 17) {
      // mixup binding of field name -> Number every so often
      Collections::shuffle(fieldIDs, random());
    }
    if (rand->nextInt(5) == 3 && i > 0) {
      const wstring delID = L"" + rand->nextInt(i);
      if (VERBOSE) {
        wcout << L"TEST: delete doc id=" << delID << endl;
      }
      w->deleteDocuments(make_shared<Term>(L"id", delID));
      docs.erase(delID);
    }
  }

  if (VERBOSE) {
    wcout << L"TEST: " << docs.size() << L" docs in index; now load fields"
          << endl;
  }
  if (docs.size() > 0) {
    std::deque<wstring> idsList =
        docs.keySet().toArray(std::deque<wstring>(docs.size()));

    for (int x = 0; x < 2; x++) {
      shared_ptr<IndexReader> r = w->getReader();
      shared_ptr<IndexSearcher> s = newSearcher(r);

      if (VERBOSE) {
        wcout << L"TEST: cycle x=" << x << L" r=" << r << endl;
      }

      int num = atLeast(1000);
      for (int iter = 0; iter < num; iter++) {
        wstring testID = idsList[rand->nextInt(idsList.size())];
        if (VERBOSE) {
          wcout << L"TEST: test id=" << testID << endl;
        }
        shared_ptr<TopDocs> hits = s->search(
            make_shared<TermQuery>(make_shared<Term>(L"id", testID)), 1);
        TestUtil::assertEquals(1, hits->totalHits);
        shared_ptr<Document> doc = r->document(hits->scoreDocs[0]->doc);
        shared_ptr<Document> docExp = docs[testID];
        for (int i = 0; i < fieldCount; i++) {
          assertEquals(L"doc " + testID + L", field f" +
                           to_wstring(fieldCount) + L" is wrong",
                       docExp[L"f" + to_wstring(i)], doc[L"f" + to_wstring(i)]);
        }
      }
      delete r;
      w->forceMerge(1);
    }
  }
  delete w;
  delete dir;
}

void BaseStoredFieldsFormatTestCase::testStoredFieldsOrder() throw(
    runtime_error)
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> customType = make_shared<FieldType>();
  customType->setStored(true);
  doc->push_back(newField(L"zzz", L"a b c", customType));
  doc->push_back(newField(L"aaa", L"a b c", customType));
  doc->push_back(newField(L"zzz", L"1 2 3", customType));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<Document> doc2 = r->document(0);
  Iterator<std::shared_ptr<IndexableField>> it = doc2->getFields().begin();
  assertTrue(it->hasNext());
  shared_ptr<Field> f = std::static_pointer_cast<Field>(it->next());
  TestUtil::assertEquals(f->name(), L"zzz");
  TestUtil::assertEquals(f->stringValue(), L"a b c");

  assertTrue(it->hasNext());
  f = std::static_pointer_cast<Field>(it->next());
  TestUtil::assertEquals(f->name(), L"aaa");
  TestUtil::assertEquals(f->stringValue(), L"a b c");

  assertTrue(it->hasNext());
  f = std::static_pointer_cast<Field>(it->next());
  TestUtil::assertEquals(f->name(), L"zzz");
  TestUtil::assertEquals(f->stringValue(), L"1 2 3");
  assertFalse(it->hasNext());
  delete r;
  delete w;
  delete d;
}

void BaseStoredFieldsFormatTestCase::testBinaryFieldOffsetLength() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  std::deque<char> b(50);
  for (int i = 0; i < 50; i++) {
    b[i] = static_cast<char>(i + 77);
  }

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = make_shared<StoredField>(L"binary", b, 10, 17);
  std::deque<char> bx = f->binaryValue()->bytes;
  assertTrue(bx.size() > 0);
  TestUtil::assertEquals(50, bx.size());
  TestUtil::assertEquals(10, f->binaryValue()->offset);
  TestUtil::assertEquals(17, f->binaryValue()->length);
  doc->push_back(f);
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Document> doc2 = ir->document(0);
  shared_ptr<IndexableField> f2 = doc2->getField(L"binary");
  b = f2->binaryValue()->bytes;
  assertTrue(b.size() > 0);
  assertEquals(17, b.size(), 17);
  TestUtil::assertEquals(87, b[0]);
  delete ir;
  delete dir;
}

void BaseStoredFieldsFormatTestCase::testNumericField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  constexpr int numDocs = atLeast(500);
  std::deque<std::shared_ptr<Number>> answers(numDocs);
  const std::deque<type_info> typeAnswers = std::deque<type_info>(numDocs);
  for (int id = 0; id < numDocs; id++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> *const nf;
    shared_ptr<Number> *const answer;
    constexpr type_info typeAnswer;
    if (random()->nextBoolean()) {
      // float/double
      if (random()->nextBoolean()) {
        constexpr float f = random()->nextFloat();
        answer = static_cast<Float>(f);
        nf = make_shared<StoredField>(L"nf", f);
        typeAnswer = Float::typeid;
      } else {
        constexpr double d = random()->nextDouble();
        answer = static_cast<Double>(d);
        nf = make_shared<StoredField>(L"nf", d);
        typeAnswer = Double::typeid;
      }
    } else {
      // int/long
      if (random()->nextBoolean()) {
        constexpr int i = random()->nextInt();
        answer = static_cast<Integer>(i);
        nf = make_shared<StoredField>(L"nf", i);
        typeAnswer = Integer::typeid;
      } else {
        constexpr int64_t l = random()->nextLong();
        answer = static_cast<int64_t>(l);
        nf = make_shared<StoredField>(L"nf", l);
        typeAnswer = Long::typeid;
      }
    }
    doc->push_back(nf);
    answers[id] = answer;
    typeAnswers[id] = typeAnswer;
    doc->push_back(make_shared<StoredField>(L"id", id));
    doc->push_back(make_shared<IntPoint>(L"id", id));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
    w->addDocument(doc);
  }
  shared_ptr<DirectoryReader> *const r = w->getReader();
  delete w;

  TestUtil::assertEquals(numDocs, r->numDocs());

  for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
    shared_ptr<LeafReader> *const sub = ctx->reader();
    shared_ptr<NumericDocValues> *const ids = DocValues::getNumeric(sub, L"id");
    for (int docID = 0; docID < sub->numDocs(); docID++) {
      shared_ptr<Document> *const doc = sub->document(docID);
      shared_ptr<Field> *const f =
          std::static_pointer_cast<Field>(doc->getField(L"nf"));
      assertTrue(L"got f=" + f,
                 std::dynamic_pointer_cast<StoredField>(f) != nullptr);
      TestUtil::assertEquals(docID, ids->nextDoc());
      TestUtil::assertEquals(answers[static_cast<int>(ids->longValue())],
                             f->numericValue());
    }
  }
  r->close();
  delete dir;
}

void BaseStoredFieldsFormatTestCase::testIndexedBit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> onlyStored = make_shared<FieldType>();
  onlyStored->setStored(true);
  doc->push_back(make_shared<Field>(L"field", L"value", onlyStored));
  doc->push_back(make_shared<StringField>(L"field2", L"value", Store::YES));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  TestUtil::assertEquals(
      IndexOptions::NONE,
      r->document(0)->getField(L"field")->fieldType()->indexOptions());
  assertNotNull(
      r->document(0)->getField(L"field2")->fieldType()->indexOptions());
  delete r;
  delete dir;
}

void BaseStoredFieldsFormatTestCase::testReadSkip() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwConf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwConf->setMaxBufferedDocs(RandomNumbers::randomIntBetween(random(), 2, 30));
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConf);

  shared_ptr<FieldType> ft = make_shared<FieldType>();
  ft->setStored(true);
  ft->freeze();

  const wstring string = TestUtil::randomSimpleString(random(), 50);
  const std::deque<char> bytes = string.getBytes(StandardCharsets::UTF_8);
  constexpr int64_t l =
      random()->nextBoolean() ? random()->nextInt(42) : random()->nextLong();
  constexpr int i =
      random()->nextBoolean() ? random()->nextInt(42) : random()->nextInt();
  constexpr float f = random()->nextFloat();
  constexpr double d = random()->nextDouble();

  deque<std::shared_ptr<Field>> fields = Arrays::asList(
      make_shared<Field>(L"bytes", bytes, ft),
      make_shared<Field>(L"string", string, ft),
      make_shared<StoredField>(L"long", l), make_shared<StoredField>(L"int", i),
      make_shared<StoredField>(L"float", f),
      make_shared<StoredField>(L"double", d));

  for (int k = 0; k < 100; ++k) {
    shared_ptr<Document> doc = make_shared<Document>();
    for (auto fld : fields) {
      doc->push_back(fld);
    }
    iw->w->addDocument(doc);
  }
  iw->commit();

  shared_ptr<DirectoryReader> *const reader = DirectoryReader::open(dir);
  constexpr int docID = random()->nextInt(100);
  for (auto fld : fields) {
    wstring fldName = fld->name();
    shared_ptr<Document> *const sDoc =
        reader->document(docID, Collections::singleton(fldName));
    shared_ptr<IndexableField> *const sField = sDoc->getField(fldName);
    if (Field::typeid->equals(fld->getClass())) {
      TestUtil::assertEquals(fld->binaryValue(), sField->binaryValue());
      TestUtil::assertEquals(fld->stringValue(), sField->stringValue());
    } else {
      TestUtil::assertEquals(fld->numericValue(), sField->numericValue());
    }
  }
  reader->close();
  delete iw;
  delete dir;
}

void BaseStoredFieldsFormatTestCase::testEmptyDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwConf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwConf->setMaxBufferedDocs(RandomNumbers::randomIntBetween(random(), 2, 30));
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConf);

  // make sure that the fact that documents might be empty is not a problem
  shared_ptr<Document> *const emptyDoc = make_shared<Document>();
  constexpr int numDocs = random()->nextBoolean() ? 1 : atLeast(1000);
  for (int i = 0; i < numDocs; ++i) {
    iw->addDocument(emptyDoc);
  }
  iw->commit();
  shared_ptr<DirectoryReader> *const rd = DirectoryReader::open(dir);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> *const doc = rd->document(i);
    assertNotNull(doc);
    assertTrue(doc->getFields().empty());
  }
  rd->close();

  delete iw;
  delete dir;
}

void BaseStoredFieldsFormatTestCase::testConcurrentReads() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwConf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwConf->setMaxBufferedDocs(RandomNumbers::randomIntBetween(random(), 2, 30));
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConf);

  // make sure the readers are properly cloned
  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<Field> *const field =
      make_shared<StringField>(L"fld", L"", Store::YES);
  doc->push_back(field);
  constexpr int numDocs = atLeast(1000);
  for (int i = 0; i < numDocs; ++i) {
    field->setStringValue(L"" + to_wstring(i));
    iw->addDocument(doc);
  }
  iw->commit();

  shared_ptr<DirectoryReader> *const rd = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> *const searcher = make_shared<IndexSearcher>(rd);
  constexpr int concurrentReads = atLeast(5);
  constexpr int readsPerThread = atLeast(50);
  const deque<std::shared_ptr<Thread>> readThreads =
      deque<std::shared_ptr<Thread>>();
  shared_ptr<AtomicReference<runtime_error>> *const ex =
      make_shared<AtomicReference<runtime_error>>();
  for (int i = 0; i < concurrentReads; ++i) {
    readThreads.push_back(make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), numDocs, rd, searcher, readsPerThread, ex, i));
  }
  for (auto thread : readThreads) {
    thread->start();
  }
  for (auto thread : readThreads) {
    thread->join();
  }
  rd->close();
  if (ex->get() != nullptr) {
    throw ex->get();
  }

  delete iw;
  delete dir;
}

BaseStoredFieldsFormatTestCase::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<BaseStoredFieldsFormatTestCase> outerInstance, int numDocs,
        shared_ptr<org::apache::lucene::index::DirectoryReader> rd,
        shared_ptr<IndexSearcher> searcher, int readsPerThread,
        shared_ptr<AtomicReference<runtime_error>> ex, int i)
{
  this->outerInstance = outerInstance;
  this->numDocs = numDocs;
  this->rd = rd;
  this->searcher = searcher;
  this->readsPerThread = readsPerThread;
  this->ex = ex;
  this->i = i;

  queries = std::deque<int>(readsPerThread);
  for (int i = 0; i < queries->length; ++i) {
    queries[i] = random()->nextInt(numDocs);
  }
}

void BaseStoredFieldsFormatTestCase::ThreadAnonymousInnerClass::run()
{
  for (int q : queries) {
    shared_ptr<Query> *const query =
        make_shared<TermQuery>(make_shared<Term>(L"fld", L"" + to_wstring(q)));
    try {
      shared_ptr<TopDocs> *const topDocs = searcher->search(query, 1);
      if (topDocs->totalHits != 1) {
        throw make_shared<IllegalStateException>(
            L"Expected 1 hit, got " + to_wstring(topDocs->totalHits));
      }
      shared_ptr<Document> *const sdoc =
          rd->document(topDocs->scoreDocs[0]->doc);
      if (sdoc->empty() || sdoc[L"fld"] == nullptr) {
        throw make_shared<IllegalStateException>(L"Could not find document " +
                                                 to_wstring(q));
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      if (!Integer::toString(q).equals(sdoc[L"fld"])) {
        throw make_shared<IllegalStateException>(L"Expected " + to_wstring(q) +
                                                 L", but got " + sdoc[L"fld"]);
      }
    } catch (const runtime_error &e) {
      ex->compareAndSet(nullptr, e);
    }
  }
}

std::deque<char> BaseStoredFieldsFormatTestCase::randomByteArray(int length,
                                                                  int max)
{
  const std::deque<char> result = std::deque<char>(length);
  for (int i = 0; i < length; ++i) {
    result[i] = static_cast<char>(random()->nextInt(max));
  }
  return result;
}

void BaseStoredFieldsFormatTestCase::testWriteReadMerge() 
{
  // get another codec, other than the default: so we are merging segments
  // across different codecs
  shared_ptr<Codec> *const otherCodec;
  if (L"SimpleText" == Codec::getDefault()->getName()) {
    otherCodec = TestUtil::getDefaultCodec();
  } else {
    otherCodec = make_shared<SimpleTextCodec>();
  }
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwConf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwConf->setMaxBufferedDocs(RandomNumbers::randomIntBetween(random(), 2, 30));
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConf);

  constexpr int docCount = atLeast(200);
  const std::deque<std::deque<std::deque<char>>> data =
      std::deque<std::deque<std::deque<char>>>(docCount);
  for (int i = 0; i < docCount; ++i) {
    constexpr int fieldCount =
        rarely() ? RandomNumbers::randomIntBetween(random(), 1, 500)
                 : RandomNumbers::randomIntBetween(random(), 1, 5);
    data[i] = std::deque<std::deque<char>>(fieldCount);
    for (int j = 0; j < fieldCount; ++j) {
      constexpr int length =
          rarely() ? random()->nextInt(1000) : random()->nextInt(10);
      constexpr int max = rarely() ? 256 : 2;
      data[i][j] = randomByteArray(length, max);
    }
  }

  shared_ptr<FieldType> *const type =
      make_shared<FieldType>(StringField::TYPE_STORED);
  type->setIndexOptions(IndexOptions::NONE);
  type->freeze();
  shared_ptr<IntPoint> id = make_shared<IntPoint>(L"id", 0);
  shared_ptr<StoredField> idStored = make_shared<StoredField>(L"id", 0);
  for (int i = 0; i < data.size(); ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(id);
    doc->push_back(idStored);
    id->setIntValue(i);
    idStored->setIntValue(i);
    for (int j = 0; j < data[i].size(); ++j) {
      shared_ptr<Field> f =
          make_shared<Field>(L"bytes" + to_wstring(j), data[i][j], type);
      doc->push_back(f);
    }
    iw->w->addDocument(doc);
    if (random()->nextBoolean() && (i % (data.size() / 10) == 0)) {
      delete iw->w;
      shared_ptr<IndexWriterConfig> iwConfNew =
          newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
      // test merging against a non-compressing codec
      if (iwConf->getCodec() == otherCodec) {
        iwConfNew->setCodec(Codec::getDefault());
      } else {
        iwConfNew->setCodec(otherCodec);
      }
      iwConf = iwConfNew;
      iw = make_shared<RandomIndexWriter>(random(), dir, iwConf);
    }
  }

  for (int i = 0; i < 10; ++i) {
    constexpr int min = random()->nextInt(data.size());
    constexpr int max = min + random()->nextInt(20);
    iw->deleteDocuments(IntPoint::newRangeQuery(L"id", min, max - 1));
  }

  iw->forceMerge(2); // force merges with deletions

  iw->commit();

  shared_ptr<DirectoryReader> *const ir = DirectoryReader::open(dir);
  assertTrue(ir->numDocs() > 0);
  int numDocs = 0;
  for (int i = 0; i < ir->maxDoc(); ++i) {
    shared_ptr<Document> *const doc = ir->document(i);
    if (doc->empty()) {
      continue;
    }
    ++numDocs;
    constexpr int docId = doc->getField(L"id")->numericValue()->intValue();
    TestUtil::assertEquals(data[docId].size() + 1, doc->getFields().size());
    for (int j = 0; j < data[docId].size(); ++j) {
      const std::deque<char> arr = data[docId][j];
      shared_ptr<BytesRef> *const arr2Ref =
          doc->getBinaryValue(L"bytes" + to_wstring(j));
      const std::deque<char> arr2 = Arrays::copyOfRange(
          arr2Ref->bytes, arr2Ref->offset, arr2Ref->offset + arr2Ref->length);
      assertArrayEquals(arr, arr2);
    }
  }
  assertTrue(ir->numDocs() <= numDocs);
  ir->close();

  iw->deleteAll();
  iw->commit();
  iw->forceMerge(1);

  delete iw;
  delete dir;
}

BaseStoredFieldsFormatTestCase::DummyFilterLeafReader::DummyFilterLeafReader(
    shared_ptr<LeafReader> in_)
    : FilterLeafReader(in_)
{
}

void BaseStoredFieldsFormatTestCase::DummyFilterLeafReader::document(
    int docID, shared_ptr<StoredFieldVisitor> visitor) 
{
  FilterLeafReader::document(maxDoc() - 1 - docID, visitor);
}

shared_ptr<CacheHelper>
BaseStoredFieldsFormatTestCase::DummyFilterLeafReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper>
BaseStoredFieldsFormatTestCase::DummyFilterLeafReader::getReaderCacheHelper()
{
  return nullptr;
}

BaseStoredFieldsFormatTestCase::DummyFilterDirectoryReader::
    DummyFilterDirectoryReader(shared_ptr<DirectoryReader> in_) throw(
        IOException)
{
  FilterDirectoryReader(in_,
                        make_shared<SubReaderWrapperAnonymousInnerClass>());
}

BaseStoredFieldsFormatTestCase::DummyFilterDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::SubReaderWrapperAnonymousInnerClass()
{
}

shared_ptr<LeafReader>
BaseStoredFieldsFormatTestCase::DummyFilterDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::wrap(shared_ptr<LeafReader> reader)
{
  return make_shared<DummyFilterLeafReader>(reader);
}

shared_ptr<DirectoryReader>
BaseStoredFieldsFormatTestCase::DummyFilterDirectoryReader::
    doWrapDirectoryReader(shared_ptr<DirectoryReader> in_) 
{
  return make_shared<DummyFilterDirectoryReader>(in_);
}

shared_ptr<CacheHelper> BaseStoredFieldsFormatTestCase::
    DummyFilterDirectoryReader::getReaderCacheHelper()
{
  return nullptr;
}

void BaseStoredFieldsFormatTestCase::testMergeFilterReader() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  constexpr int numDocs = atLeast(200);
  const std::deque<wstring> stringValues = std::deque<wstring>(10);
  for (int i = 0; i < stringValues.size(); ++i) {
    stringValues[i] =
        RandomStrings::randomRealisticUnicodeOfLength(random(), 10);
  }
  std::deque<std::shared_ptr<Document>> docs(numDocs);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(
        L"to_delete", random()->nextBoolean() ? L"yes" : L"no", Store::NO));
    doc->push_back(make_shared<StoredField>(L"id", i));
    doc->push_back(make_shared<StoredField>(L"i", random()->nextInt(50)));
    doc->push_back(make_shared<StoredField>(L"l", random()->nextLong()));
    doc->push_back(make_shared<StoredField>(L"d", random()->nextDouble()));
    doc->push_back(make_shared<StoredField>(L"f", random()->nextFloat()));
    doc->push_back(make_shared<StoredField>(
        L"s", RandomPicks::randomFrom(random(), stringValues)));
    doc->push_back(make_shared<StoredField>(
        L"b", make_shared<BytesRef>(
                  RandomPicks::randomFrom(random(), stringValues))));
    docs[i] = doc;
    w->addDocument(doc);
  }
  if (random()->nextBoolean()) {
    w->deleteDocuments(make_shared<Term>(L"to_delete", L"yes"));
  }
  w->commit();
  delete w;

  shared_ptr<DirectoryReader> reader =
      make_shared<DummyFilterDirectoryReader>(DirectoryReader::open(dir));

  shared_ptr<Directory> dir2 = newDirectory();
  w = make_shared<RandomIndexWriter>(random(), dir2);
  TestUtil::addIndexesSlowly(w->w, {reader});
  reader->close();
  delete dir;

  reader = w->getReader();
  for (int i = 0; i < reader->maxDoc(); ++i) {
    shared_ptr<Document> *const doc = reader->document(i);
    constexpr int id = doc->getField(L"id")->numericValue()->intValue();
    shared_ptr<Document> *const expected = docs[id];
    TestUtil::assertEquals(expected[L"s"], doc[L"s"]);
    TestUtil::assertEquals(expected->getField(L"i")->numericValue(),
                           doc->getField(L"i")->numericValue());
    TestUtil::assertEquals(expected->getField(L"l")->numericValue(),
                           doc->getField(L"l")->numericValue());
    TestUtil::assertEquals(expected->getField(L"d")->numericValue(),
                           doc->getField(L"d")->numericValue());
    TestUtil::assertEquals(expected->getField(L"f")->numericValue(),
                           doc->getField(L"f")->numericValue());
    TestUtil::assertEquals(expected->getField(L"b")->binaryValue(),
                           doc->getField(L"b")->binaryValue());
  }

  reader->close();
  delete w;
  TestUtil::checkIndex(dir2);
  delete dir2;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testBigDocuments() throws
// java.io.IOException
void BaseStoredFieldsFormatTestCase::testBigDocuments() 
{
  assumeWorkingMMapOnWindows();

  // "big" as "much bigger than the chunk size"
  // for this test we force a FS dir
  // we can't just use newFSDirectory, because this test doesn't really index
  // anything. so if we get NRTCachingDir+SimpleText, we make massive stored
  // fields and OOM (LUCENE-4484)
  shared_ptr<Directory> dir = make_shared<MockDirectoryWrapper>(
      random(), make_shared<MMapDirectory>(createTempDir(L"testBigDocuments")));
  shared_ptr<IndexWriterConfig> iwConf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwConf->setMaxBufferedDocs(RandomNumbers::randomIntBetween(random(), 2, 30));
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConf);

  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(Throttling::NEVER);
  }

  shared_ptr<Document> *const emptyDoc = make_shared<Document>(); // emptyDoc
  shared_ptr<Document> *const bigDoc1 =
      make_shared<Document>(); // lot of small fields
  shared_ptr<Document> *const bigDoc2 =
      make_shared<Document>(); // 1 very big field

  shared_ptr<Field> *const idField =
      make_shared<StringField>(L"id", L"", Store::NO);
  emptyDoc->push_back(idField);
  bigDoc1->push_back(idField);
  bigDoc2->push_back(idField);

  shared_ptr<FieldType> *const onlyStored =
      make_shared<FieldType>(StringField::TYPE_STORED);
  onlyStored->setIndexOptions(IndexOptions::NONE);

  shared_ptr<Field> *const smallField = make_shared<Field>(
      L"fld", randomByteArray(random()->nextInt(10), 256), onlyStored);
  constexpr int numFields =
      RandomNumbers::randomIntBetween(random(), 500000, 1000000);
  for (int i = 0; i < numFields; ++i) {
    bigDoc1->push_back(smallField);
  }

  shared_ptr<Field> *const bigField = make_shared<Field>(
      L"fld",
      randomByteArray(
          RandomNumbers::randomIntBetween(random(), 1000000, 5000000), 2),
      onlyStored);
  bigDoc2->push_back(bigField);

  constexpr int numDocs = atLeast(5);
  std::deque<std::shared_ptr<Document>> docs(numDocs);
  for (int i = 0; i < numDocs; ++i) {
    docs[i] = RandomPicks::randomFrom(
        random(), Arrays::asList(emptyDoc, bigDoc1, bigDoc2));
  }
  for (int i = 0; i < numDocs; ++i) {
    idField->setStringValue(L"" + to_wstring(i));
    iw->addDocument(docs[i]);
    if (random()->nextInt(numDocs) == 0) {
      iw->commit();
    }
  }
  iw->commit();
  iw->forceMerge(1); // look at what happens when big docs are merged
  shared_ptr<DirectoryReader> *const rd = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> *const searcher = make_shared<IndexSearcher>(rd);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Query> *const query =
        make_shared<TermQuery>(make_shared<Term>(L"id", L"" + to_wstring(i)));
    shared_ptr<TopDocs> *const topDocs = searcher->search(query, 1);
    assertEquals(L"" + to_wstring(i), 1, topDocs->totalHits);
    shared_ptr<Document> *const doc = rd->document(topDocs->scoreDocs[0]->doc);
    assertNotNull(doc);
    std::deque<std::shared_ptr<IndexableField>> fieldValues =
        doc->getFields(L"fld");
    TestUtil::assertEquals(docs[i]->getFields(L"fld")->size(),
                           fieldValues.size());
    if (fieldValues.size() > 0) {
      TestUtil::assertEquals(docs[i]->getFields(L"fld")[0].binaryValue(),
                             fieldValues[0]->binaryValue());
    }
  }
  rd->close();
  delete iw;
  delete dir;
}

void BaseStoredFieldsFormatTestCase::testBulkMergeWithDeletes() throw(
    IOException)
{
  constexpr int numDocs = atLeast(200);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(NoMergePolicy::INSTANCE));
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        make_shared<StringField>(L"id", Integer::toString(i), Store::YES));
    doc->push_back(
        make_shared<StoredField>(L"f", TestUtil::randomSimpleString(random())));
    w->addDocument(doc);
  }
  constexpr int deleteCount = TestUtil::nextInt(random(), 5, numDocs);
  for (int i = 0; i < deleteCount; ++i) {
    constexpr int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    w->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }
  w->commit();
  delete w;
  w = make_shared<RandomIndexWriter>(random(), dir);
  w->forceMerge(TestUtil::nextInt(random(), 1, 3));
  w->commit();
  delete w;
  TestUtil::checkIndex(dir);
  delete dir;
}

void BaseStoredFieldsFormatTestCase::testMismatchedFields() 
{
  std::deque<std::shared_ptr<Directory>> dirs(10);
  for (int i = 0; i < dirs.size(); i++) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(nullptr);
    shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
    shared_ptr<Document> doc = make_shared<Document>();
    for (int j = 0; j < 10; j++) {
      // add fields where name=value (e.g. 3=3) so we can detect if stuff gets
      // screwed up.
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<StringField>(
          Integer::toString(j), Integer::toString(j), Store::YES));
    }
    for (int j = 0; j < 10; j++) {
      iw->addDocument(doc);
    }

    shared_ptr<DirectoryReader> reader = DirectoryReader::open(iw);
    // mix up fields explicitly
    if (random()->nextBoolean()) {
      reader = make_shared<MismatchedDirectoryReader>(reader, random());
    }
    dirs[i] = newDirectory();
    shared_ptr<IndexWriter> adder = make_shared<IndexWriter>(
        dirs[i], make_shared<IndexWriterConfig>(nullptr));
    TestUtil::addIndexesSlowly(adder, {reader});
    adder->commit();
    delete adder;

    IOUtils::close({reader, iw, dir});
  }

  shared_ptr<Directory> everything = newDirectory();
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      everything, make_shared<IndexWriterConfig>(nullptr));
  iw->addIndexes(dirs);
  iw->forceMerge(1);

  shared_ptr<LeafReader> ir = getOnlyLeafReader(DirectoryReader::open(iw));
  for (int i = 0; i < ir->maxDoc(); i++) {
    shared_ptr<Document> doc = ir->document(i);
    TestUtil::assertEquals(10, doc->getFields().size());
    for (int j = 0; j < 10; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      TestUtil::assertEquals(Integer::toString(j), doc[Integer::toString(j)]);
    }
  }

  IOUtils::close({iw, ir, everything});
  IOUtils::close(dirs);
}
} // namespace org::apache::lucene::index