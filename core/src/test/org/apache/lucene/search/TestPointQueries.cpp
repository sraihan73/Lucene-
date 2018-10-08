using namespace std;

#include "TestPointQueries.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using Lucene60PointsReader =
    org::apache::lucene::codecs::lucene60::Lucene60PointsReader;
using Lucene60PointsWriter =
    org::apache::lucene::codecs::lucene60::Lucene60PointsWriter;
using BinaryPoint = org::apache::lucene::document::BinaryPoint;
using Document = org::apache::lucene::document::Document;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using PointValues = org::apache::lucene::index::PointValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;
using org::junit::BeforeClass;
int64_t TestPointQueries::valueMid = 0;
int TestPointQueries::valueRange = 0;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass()
void TestPointQueries::beforeClass()
{
  if (random()->nextBoolean()) {
    valueMid = random()->nextLong();
    if (random()->nextBoolean()) {
      // Wide range
      valueRange = TestUtil::nextInt(random(), 1, numeric_limits<int>::max());
    } else {
      // Narrow range
      valueRange = TestUtil::nextInt(random(), 1, 100000);
    }
    if (VERBOSE) {
      wcout << L"TEST: will generate long values " << valueMid << L" +/- "
            << valueRange << endl;
    }
  } else {
    // All longs
    valueRange = 0;
    if (VERBOSE) {
      wcout << L"TEST: will generate all long values" << endl;
    }
  }
}

void TestPointQueries::testBasicInts() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"point", -7));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"point", 0));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"point", 3));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  TestUtil::assertEquals(2, s->count(IntPoint::newRangeQuery(L"point", -8, 1)));
  TestUtil::assertEquals(3, s->count(IntPoint::newRangeQuery(L"point", -7, 3)));
  TestUtil::assertEquals(1, s->count(IntPoint::newExactQuery(L"point", -7)));
  TestUtil::assertEquals(0, s->count(IntPoint::newExactQuery(L"point", -6)));
  delete w;
  r->close();
  delete dir;
}

void TestPointQueries::testBasicFloats() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", -7.0f));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", 0.0f));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", 3.0f));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  TestUtil::assertEquals(
      2, s->count(FloatPoint::newRangeQuery(L"point", -8.0f, 1.0f)));
  TestUtil::assertEquals(
      3, s->count(FloatPoint::newRangeQuery(L"point", -7.0f, 3.0f)));
  TestUtil::assertEquals(1,
                         s->count(FloatPoint::newExactQuery(L"point", -7.0f)));
  TestUtil::assertEquals(0,
                         s->count(FloatPoint::newExactQuery(L"point", -6.0f)));
  delete w;
  r->close();
  delete dir;
}

void TestPointQueries::testBasicLongs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<LongPoint>(L"point", -7));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<LongPoint>(L"point", 0));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<LongPoint>(L"point", 3));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  TestUtil::assertEquals(
      2, s->count(LongPoint::newRangeQuery(L"point", -8LL, 1LL)));
  TestUtil::assertEquals(
      3, s->count(LongPoint::newRangeQuery(L"point", -7LL, 3LL)));
  TestUtil::assertEquals(1, s->count(LongPoint::newExactQuery(L"point", -7LL)));
  TestUtil::assertEquals(0, s->count(LongPoint::newExactQuery(L"point", -6LL)));
  delete w;
  r->close();
  delete dir;
}

void TestPointQueries::testBasicDoubles() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"point", -7.0));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"point", 0.0));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"point", 3.0));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  TestUtil::assertEquals(
      2, s->count(DoublePoint::newRangeQuery(L"point", -8.0, 1.0)));
  TestUtil::assertEquals(
      3, s->count(DoublePoint::newRangeQuery(L"point", -7.0, 3.0)));
  TestUtil::assertEquals(1,
                         s->count(DoublePoint::newExactQuery(L"point", -7.0)));
  TestUtil::assertEquals(0,
                         s->count(DoublePoint::newExactQuery(L"point", -6.0)));
  delete w;
  r->close();
  delete dir;
}

void TestPointQueries::testCrazyDoubles() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<DoublePoint>(L"point", -numeric_limits<double>::infinity()));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"point", -0.0));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"point", +0.0));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"point", Double::MIN_VALUE));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<DoublePoint>(L"point", numeric_limits<double>::max()));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<DoublePoint>(L"point", numeric_limits<double>::infinity()));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"point", NAN));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);

  // exact queries
  TestUtil::assertEquals(1,
                         s->count(DoublePoint::newExactQuery(
                             L"point", -numeric_limits<double>::infinity())));
  TestUtil::assertEquals(1,
                         s->count(DoublePoint::newExactQuery(L"point", -0.0)));
  TestUtil::assertEquals(1,
                         s->count(DoublePoint::newExactQuery(L"point", +0.0)));
  TestUtil::assertEquals(
      1, s->count(DoublePoint::newExactQuery(L"point", Double::MIN_VALUE)));
  TestUtil::assertEquals(1, s->count(DoublePoint::newExactQuery(
                                L"point", numeric_limits<double>::max())));
  TestUtil::assertEquals(1, s->count(DoublePoint::newExactQuery(
                                L"point", numeric_limits<double>::infinity())));
  TestUtil::assertEquals(1,
                         s->count(DoublePoint::newExactQuery(L"point", NAN)));

  // set query
  std::deque<double> set = {numeric_limits<double>::max(),
                             NAN,
                             +0.0,
                             -numeric_limits<double>::infinity(),
                             Double::MIN_VALUE,
                             -0.0,
                             numeric_limits<double>::infinity()};
  TestUtil::assertEquals(7, s->count(DoublePoint::newSetQuery(L"point", set)));

  // ranges
  TestUtil::assertEquals(
      2, s->count(DoublePoint::newRangeQuery(
             L"point", -numeric_limits<double>::infinity(), -0.0)));
  TestUtil::assertEquals(
      2, s->count(DoublePoint::newRangeQuery(L"point", -0.0, 0.0)));
  TestUtil::assertEquals(2, s->count(DoublePoint::newRangeQuery(
                                L"point", 0.0, Double::MIN_VALUE)));
  TestUtil::assertEquals(
      2, s->count(DoublePoint::newRangeQuery(L"point", Double::MIN_VALUE,
                                             numeric_limits<double>::max())));
  TestUtil::assertEquals(2, s->count(DoublePoint::newRangeQuery(
                                L"point", numeric_limits<double>::max(),
                                numeric_limits<double>::infinity())));
  TestUtil::assertEquals(
      2, s->count(DoublePoint::newRangeQuery(
             L"point", numeric_limits<double>::infinity(), NAN)));

  delete w;
  r->close();
  delete dir;
}

void TestPointQueries::testCrazyFloats() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<FloatPoint>(L"point", -numeric_limits<float>::infinity()));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", -0.0F));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", +0.0F));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", Float::MIN_VALUE));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<FloatPoint>(L"point", numeric_limits<float>::max()));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<FloatPoint>(L"point", numeric_limits<float>::infinity()));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", NAN));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);

  // exact queries
  TestUtil::assertEquals(1, s->count(FloatPoint::newExactQuery(
                                L"point", -numeric_limits<float>::infinity())));
  TestUtil::assertEquals(1,
                         s->count(FloatPoint::newExactQuery(L"point", -0.0F)));
  TestUtil::assertEquals(1,
                         s->count(FloatPoint::newExactQuery(L"point", +0.0F)));
  TestUtil::assertEquals(
      1, s->count(FloatPoint::newExactQuery(L"point", Float::MIN_VALUE)));
  TestUtil::assertEquals(1, s->count(FloatPoint::newExactQuery(
                                L"point", numeric_limits<float>::max())));
  TestUtil::assertEquals(1, s->count(FloatPoint::newExactQuery(
                                L"point", numeric_limits<float>::infinity())));
  TestUtil::assertEquals(1, s->count(FloatPoint::newExactQuery(L"point", NAN)));

  // set query
  std::deque<float> set = {numeric_limits<float>::max(),
                            NAN,
                            +0.0F,
                            -numeric_limits<float>::infinity(),
                            Float::MIN_VALUE,
                            -0.0F,
                            numeric_limits<float>::infinity()};
  TestUtil::assertEquals(7, s->count(FloatPoint::newSetQuery(L"point", set)));

  // ranges
  TestUtil::assertEquals(
      2, s->count(FloatPoint::newRangeQuery(
             L"point", -numeric_limits<float>::infinity(), -0.0F)));
  TestUtil::assertEquals(
      2, s->count(FloatPoint::newRangeQuery(L"point", -0.0F, 0.0F)));
  TestUtil::assertEquals(
      2, s->count(FloatPoint::newRangeQuery(L"point", 0.0F, Float::MIN_VALUE)));
  TestUtil::assertEquals(
      2, s->count(FloatPoint::newRangeQuery(L"point", Float::MIN_VALUE,
                                            numeric_limits<float>::max())));
  TestUtil::assertEquals(2, s->count(FloatPoint::newRangeQuery(
                                L"point", numeric_limits<float>::max(),
                                numeric_limits<float>::infinity())));
  TestUtil::assertEquals(
      2, s->count(FloatPoint::newRangeQuery(
             L"point", numeric_limits<float>::infinity(), NAN)));

  delete w;
  r->close();
  delete dir;
}

void TestPointQueries::testAllEqual() 
{
  int numValues = atLeast(10000);
  int64_t value = randomValue();
  std::deque<int64_t> values(numValues);

  if (VERBOSE) {
    wcout << L"TEST: use same value=" << value << endl;
  }
  Arrays::fill(values, value);

  verifyLongs(values, nullptr);
}

void TestPointQueries::testRandomLongsTiny() 
{
  // Make sure single-leaf-node case is OK:
  doTestRandomLongs(10);
}

void TestPointQueries::testRandomLongsMedium() 
{
  doTestRandomLongs(10000);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testRandomLongsBig() throws Exception
void TestPointQueries::testRandomLongsBig() 
{
  doTestRandomLongs(100000);
}

void TestPointQueries::doTestRandomLongs(int count) 
{

  int numValues = TestUtil::nextInt(random(), count, count * 2);

  if (VERBOSE) {
    wcout << L"TEST: numValues=" << numValues << endl;
  }

  std::deque<int64_t> values(numValues);
  std::deque<int> ids(numValues);

  bool singleValued = random()->nextBoolean();

  int sameValuePct = random()->nextInt(100);

  int id = 0;
  for (int ord = 0; ord < numValues; ord++) {
    if (ord > 0 && random()->nextInt(100) < sameValuePct) {
      // Identical to old value
      values[ord] = values[random()->nextInt(ord)];
    } else {
      values[ord] = randomValue();
    }

    ids[ord] = id;
    if (singleValued || random()->nextInt(2) == 1) {
      id++;
    }
  }

  verifyLongs(values, ids);
}

void TestPointQueries::testLongEncode()
{
  for (int i = 0; i < 10000; i++) {
    int64_t v = random()->nextLong();
    std::deque<char> tmp(8);
    NumericUtils::longToSortableBytes(v, tmp, 0);
    int64_t v2 = NumericUtils::sortableBytesToLong(tmp, 0);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"got bytes=" + Arrays->toString(tmp), v, v2);
  }
}

void TestPointQueries::verifyLongs(std::deque<int64_t> &values,
                                   std::deque<int> &ids) 
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();

  // Else we can get O(N^2) merging:
  int mbd = iwc->getMaxBufferedDocs();
  if (mbd != -1 && mbd < values.size() / 100) {
    iwc->setMaxBufferedDocs(values.size() / 100);
  }
  iwc->setCodec(getCodec());
  shared_ptr<Directory> dir;
  if (values.size() > 100000) {
    dir = newMaybeVirusCheckingFSDirectory(createTempDir(L"TestRangeTree"));
  } else {
    dir = newMaybeVirusCheckingDirectory();
  }

  int missingPct = random()->nextInt(100);
  int deletedPct = random()->nextInt(100);
  if (VERBOSE) {
    wcout << L"  missingPct=" << missingPct << endl;
    wcout << L"  deletedPct=" << deletedPct << endl;
  }

  shared_ptr<BitSet> missing = make_shared<BitSet>();
  shared_ptr<BitSet> deleted = make_shared<BitSet>();

  shared_ptr<Document> doc;
  int lastID = -1;

  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  for (int ord = 0; ord < values.size(); ord++) {

    int id;
    if (ids.empty()) {
      id = ord;
    } else {
      id = ids[ord];
    }
    if (id != lastID) {
      if (random()->nextInt(100) < missingPct) {
        missing->set(id);
        if (VERBOSE) {
          wcout << L"  missing id=" << id << endl;
        }
      }

      if (doc->size() > 0) {
        w->addDocument(doc);
        if (random()->nextInt(100) < deletedPct) {
          int idToDelete = random()->nextInt(id);
          w->deleteDocuments(
              make_shared<Term>(L"id", L"" + to_wstring(idToDelete)));
          deleted->set(idToDelete);
          if (VERBOSE) {
            wcout << L"  delete id=" << idToDelete << endl;
          }
        }
      }

      doc = make_shared<Document>();
      doc->push_back(
          newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
      doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
      lastID = id;
    }

    if (missing->get(id) == false) {
      doc->push_back(make_shared<LongPoint>(L"sn_value", values[id]));
      std::deque<char> bytes(8);
      NumericUtils::longToSortableBytes(values[id], bytes, 0);
      doc->push_back(make_shared<BinaryPoint>(L"ss_value", bytes));
    }
  }

  w->addDocument(doc);

  if (random()->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"  forceMerge(1)" << endl;
    }
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  int numThreads = TestUtil::nextInt(random(), 2, 5);

  if (VERBOSE) {
    wcout << L"TEST: use " << numThreads << L" query threads; searcher=" << s
          << endl;
  }

  deque<std::shared_ptr<Thread>> threads = deque<std::shared_ptr<Thread>>();
  constexpr int iters = atLeast(100);

  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  shared_ptr<AtomicBoolean> *const failed = make_shared<AtomicBoolean>();

  for (int i = 0; i < numThreads; i++) {
    shared_ptr<Thread> thread = make_shared<ThreadAnonymousInnerClass>(
        values, missing, deleted, doc, r, s, iters, startingGun, failed);
    thread->setName(L"T" + to_wstring(i));
    thread->start();
    threads.push_back(thread);
  }
  startingGun->countDown();
  for (auto thread : threads) {
    thread->join();
  }
  IOUtils::close({r, dir});
}

TestPointQueries::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    deque<int64_t> &values, shared_ptr<BitSet> missing,
    shared_ptr<BitSet> deleted, shared_ptr<Document> doc,
    shared_ptr<IndexReader> r,
    shared_ptr<org::apache::lucene::search::IndexSearcher> s, int iters,
    shared_ptr<CountDownLatch> startingGun, shared_ptr<AtomicBoolean> failed)
{
  this->values = values;
  this->missing = missing;
  this->deleted = deleted;
  this->doc = doc;
  this->r = r;
  this->s = s;
  this->iters = iters;
  this->startingGun = startingGun;
  this->failed = failed;
}

void TestPointQueries::ThreadAnonymousInnerClass::run()
{
  try {
    _run();
  } catch (const runtime_error &e) {
    failed->set(true);
    throw runtime_error(e);
  }
}

void TestPointQueries::ThreadAnonymousInnerClass::_run() 
{
  startingGun->await();

  for (int iter = 0; iter < iters && failed->get() == false; iter++) {
    optional<int64_t> lower = randomValue();
    optional<int64_t> upper = randomValue();

    if (upper < lower) {
      int64_t x = lower;
      lower = upper;
      upper = x;
    }

    shared_ptr<Query> query;

    if (VERBOSE) {
      wcout << L"\n"
            << Thread::currentThread().getName() << L": TEST: iter=" << iter
            << L" value=" << lower << L" TO " << upper << endl;
      std::deque<char> tmp(8);
      if (lower) {
        NumericUtils::longToSortableBytes(lower, tmp, 0);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wcout << L"  lower bytes=" << Arrays->toString(tmp) << endl;
      }
      if (upper) {
        NumericUtils::longToSortableBytes(upper, tmp, 0);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wcout << L"  upper bytes=" << Arrays->toString(tmp) << endl;
      }
    }

    if (LuceneTestCase::random()->nextBoolean()) {
      query = LongPoint::newRangeQuery(L"sn_value", lower, upper);
    } else {
      std::deque<char> lowerBytes(8);
      NumericUtils::longToSortableBytes(lower, lowerBytes, 0);
      std::deque<char> upperBytes(8);
      NumericUtils::longToSortableBytes(upper, upperBytes, 0);
      query = BinaryPoint::newRangeQuery(L"ss_value", lowerBytes, upperBytes);
    }

    if (VERBOSE) {
      wcout << Thread::currentThread().getName() << L":  using query: " << query
            << endl;
    }

    shared_ptr<BitSet> *const hits = make_shared<BitSet>();
    s->search(query, make_shared<SimpleCollectorAnonymousInnerClass>(
                         shared_from_this(), hits));

    if (VERBOSE) {
      wcout << Thread::currentThread().getName() << L":  hitCount: "
            << hits->cardinality() << endl;
    }

    shared_ptr<NumericDocValues> docIDToID =
        MultiDocValues::getNumericValues(r, L"id");

    for (int docID = 0; docID < r->maxDoc(); docID++) {
      TestUtil::assertEquals(docID, docIDToID->nextDoc());
      int id = static_cast<int>(docIDToID->longValue());
      bool expected = missing->get(id) == false && deleted->get(id) == false &&
                      values[id] >= lower && values[id] <= upper;
      if (hits->get(docID) != expected) {
        // We do exact quantized comparison so the bbox query should never
        // disagree:
        fail(Thread::currentThread().getName() + L": iter=" + to_wstring(iter) +
             L" id=" + to_wstring(id) + L" docID=" + to_wstring(docID) +
             L" value=" + to_wstring(values[id]) + L" (range: " + lower +
             L" TO " + upper + L") expected " +
             StringHelper::toString(expected) + L" but got: " +
             hits->get(docID) + L" deleted?=" + deleted->get(id) + L" query=" +
             query);
      }
    }
  }
}

TestPointQueries::ThreadAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass::SimpleCollectorAnonymousInnerClass(
        shared_ptr<ThreadAnonymousInnerClass> outerInstance,
        shared_ptr<BitSet> hits)
{
  this->outerInstance = outerInstance;
  this->hits = hits;
}

bool TestPointQueries::ThreadAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}

void TestPointQueries::ThreadAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass::doSetNextReader(
        shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

void TestPointQueries::ThreadAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass::collect(int doc)
{
  hits->set(docBase + doc);
}

void TestPointQueries::testRandomBinaryTiny() 
{
  doTestRandomBinary(10);
}

void TestPointQueries::testRandomBinaryMedium() 
{
  doTestRandomBinary(10000);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testRandomBinaryBig() throws Exception
void TestPointQueries::testRandomBinaryBig() 
{
  doTestRandomBinary(100000);
}

void TestPointQueries::doTestRandomBinary(int count) 
{
  int numValues = TestUtil::nextInt(random(), count, count * 2);
  int numBytesPerDim =
      TestUtil::nextInt(random(), 2, PointValues::MAX_NUM_BYTES);
  int numDims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);

  int sameValuePct = random()->nextInt(100);
  if (VERBOSE) {
    wcout << L"TEST: sameValuePct=" << sameValuePct << endl;
  }

  std::deque<std::deque<std::deque<char>>> docValues(numValues);

  bool singleValued = random()->nextBoolean();
  std::deque<int> ids(numValues);

  int id = 0;
  for (int ord = 0; ord < numValues; ord++) {
    if (ord > 0 && random()->nextInt(100) < sameValuePct) {
      // Identical to old value
      docValues[ord] = docValues[random()->nextInt(ord)];
    } else {
      // Make a new random value
      std::deque<std::deque<char>> values(numDims);
      for (int dim = 0; dim < numDims; dim++) {
        values[dim] = std::deque<char>(numBytesPerDim);
        random()->nextBytes(values[dim]);
      }
      docValues[ord] = values;
    }
    ids[ord] = id;
    if (singleValued || random()->nextInt(2) == 1) {
      id++;
    }
  }

  verifyBinary(docValues, ids, numBytesPerDim);
}

void TestPointQueries::verifyBinary(
    std::deque<std::deque<std::deque<char>>> &docValues,
    std::deque<int> &ids, int numBytesPerDim) 
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();

  int numDims = docValues[0].size();
  int bytesPerDim = docValues[0][0].size();

  // Else we can get O(N^2) merging:
  int mbd = iwc->getMaxBufferedDocs();
  if (mbd != -1 && mbd < docValues.size() / 100) {
    iwc->setMaxBufferedDocs(docValues.size() / 100);
  }
  iwc->setCodec(getCodec());

  shared_ptr<Directory> dir;
  if (docValues.size() > 100000) {
    dir = newFSDirectory(createTempDir(L"TestPointQueries"));
  } else {
    dir = newDirectory();
  }

  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int numValues = docValues.size();
  if (VERBOSE) {
    wcout << L"TEST: numValues=" << numValues << L" numDims=" << numDims
          << L" numBytesPerDim=" << numBytesPerDim << endl;
  }

  int missingPct = random()->nextInt(100);
  int deletedPct = random()->nextInt(100);
  if (VERBOSE) {
    wcout << L"  missingPct=" << missingPct << endl;
    wcout << L"  deletedPct=" << deletedPct << endl;
  }

  shared_ptr<BitSet> missing = make_shared<BitSet>();
  shared_ptr<BitSet> deleted = make_shared<BitSet>();

  shared_ptr<Document> doc;
  int lastID = -1;

  for (int ord = 0; ord < numValues; ord++) {
    int id = ids[ord];
    if (id != lastID) {
      if (random()->nextInt(100) < missingPct) {
        missing->set(id);
        if (VERBOSE) {
          wcout << L"  missing id=" << id << endl;
        }
      }

      if (doc->size() > 0) {
        w->addDocument(doc);
        if (random()->nextInt(100) < deletedPct) {
          int idToDelete = random()->nextInt(id);
          w->deleteDocuments(
              make_shared<Term>(L"id", L"" + to_wstring(idToDelete)));
          deleted->set(idToDelete);
          if (VERBOSE) {
            wcout << L"  delete id=" << idToDelete << endl;
          }
        }
      }

      doc = make_shared<Document>();
      doc->push_back(
          newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
      doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
      lastID = id;
    }

    if (missing->get(id) == false) {
      doc->push_back(make_shared<BinaryPoint>(L"value", docValues[ord]));
      if (VERBOSE) {
        wcout << L"id=" << id << endl;
        for (int dim = 0; dim < numDims; dim++) {
          wcout << L"  dim=" << dim << L" value="
                << bytesToString(docValues[ord][dim]) << endl;
        }
      }
    }
  }

  w->addDocument(doc);

  if (random()->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"  forceMerge(1)" << endl;
    }
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  int numThreads = TestUtil::nextInt(random(), 2, 5);

  if (VERBOSE) {
    wcout << L"TEST: use " << numThreads << L" query threads; searcher=" << s
          << endl;
  }

  deque<std::shared_ptr<Thread>> threads = deque<std::shared_ptr<Thread>>();
  constexpr int iters = atLeast(100);

  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  shared_ptr<AtomicBoolean> *const failed = make_shared<AtomicBoolean>();

  for (int i = 0; i < numThreads; i++) {
    shared_ptr<Thread> thread = make_shared<ThreadAnonymousInnerClass2>(
        shared_from_this(), docValues, ids, numDims, bytesPerDim, numValues,
        missing, deleted, doc, r, s, iters, startingGun, failed);
    thread->setName(L"T" + to_wstring(i));
    thread->start();
    threads.push_back(thread);
  }
  startingGun->countDown();
  for (auto thread : threads) {
    thread->join();
  }
  IOUtils::close({r, dir});
}

TestPointQueries::ThreadAnonymousInnerClass2::ThreadAnonymousInnerClass2(
    shared_ptr<TestPointQueries> outerInstance,
    deque<deque<deque<char>>> &docValues, deque<int> &ids, int numDims,
    int bytesPerDim, int numValues, shared_ptr<BitSet> missing,
    shared_ptr<BitSet> deleted, shared_ptr<Document> doc,
    shared_ptr<IndexReader> r,
    shared_ptr<org::apache::lucene::search::IndexSearcher> s, int iters,
    shared_ptr<CountDownLatch> startingGun, shared_ptr<AtomicBoolean> failed)
{
  this->outerInstance = outerInstance;
  this->docValues = docValues;
  this->ids = ids;
  this->numDims = numDims;
  this->bytesPerDim = bytesPerDim;
  this->numValues = numValues;
  this->missing = missing;
  this->deleted = deleted;
  this->doc = doc;
  this->r = r;
  this->s = s;
  this->iters = iters;
  this->startingGun = startingGun;
  this->failed = failed;
}

void TestPointQueries::ThreadAnonymousInnerClass2::run()
{
  try {
    _run();
  } catch (const runtime_error &e) {
    failed->set(true);
    throw runtime_error(e);
  }
}

void TestPointQueries::ThreadAnonymousInnerClass2::_run() 
{
  startingGun->await();

  for (int iter = 0; iter < iters && failed->get() == false; iter++) {

    std::deque<std::deque<char>> lower(numDims);
    std::deque<std::deque<char>> upper(numDims);
    for (int dim = 0; dim < numDims; dim++) {
      lower[dim] = std::deque<char>(bytesPerDim);
      LuceneTestCase::random()->nextBytes(lower[dim]);

      upper[dim] = std::deque<char>(bytesPerDim);
      LuceneTestCase::random()->nextBytes(upper[dim]);

      if (StringHelper::compare(bytesPerDim, lower[dim], 0, upper[dim], 0) >
          0) {
        std::deque<char> x = lower[dim];
        lower[dim] = upper[dim];
        upper[dim] = x;
      }
    }

    if (VERBOSE) {
      wcout << L"\n"
            << Thread::currentThread().getName() << L": TEST: iter=" << iter
            << endl;
      for (int dim = 0; dim < numDims; dim++) {
        wcout << L"  dim=" << dim << L" " << bytesToString(lower[dim])
              << L" TO " << bytesToString(upper[dim]) << endl;
      }
    }

    shared_ptr<Query> query =
        BinaryPoint::newRangeQuery(L"value", lower, upper);

    if (VERBOSE) {
      wcout << Thread::currentThread().getName() << L":  using query: " << query
            << endl;
    }

    shared_ptr<BitSet> *const hits = make_shared<BitSet>();
    s->search(query, make_shared<SimpleCollectorAnonymousInnerClass2>(
                         shared_from_this(), hits));

    if (VERBOSE) {
      wcout << Thread::currentThread().getName() << L":  hitCount: "
            << hits->cardinality() << endl;
    }

    shared_ptr<BitSet> expected = make_shared<BitSet>();
    for (int ord = 0; ord < numValues; ord++) {
      int id = ids[ord];
      if (missing->get(id) == false && deleted->get(id) == false &&
          matches(bytesPerDim, lower, upper, docValues[ord])) {
        expected->set(id);
      }
    }

    shared_ptr<NumericDocValues> docIDToID =
        MultiDocValues::getNumericValues(r, L"id");

    int failCount = 0;
    for (int docID = 0; docID < r->maxDoc(); docID++) {
      TestUtil::assertEquals(docID, docIDToID->nextDoc());
      int id = static_cast<int>(docIDToID->longValue());
      if (hits->get(docID) != expected->get(id)) {
        wcout << L"FAIL: iter=" << iter << L" id=" << id << L" docID=" << docID
              << L" expected=" << expected->get(id) << L" but got "
              << hits->get(docID) << L" deleted?=" << deleted->get(id)
              << L" missing?=" << missing->get(id) << endl;
        for (int dim = 0; dim < numDims; dim++) {
          wcout << L"  dim=" << dim << L" range: " << bytesToString(lower[dim])
                << L" TO " << bytesToString(upper[dim]) << endl;
          failCount++;
        }
      }
    }
    if (failCount != 0) {
      fail(to_wstring(failCount) + L" hits were wrong");
    }
  }
}

TestPointQueries::ThreadAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2::SimpleCollectorAnonymousInnerClass2(
        shared_ptr<ThreadAnonymousInnerClass2> outerInstance,
        shared_ptr<BitSet> hits)
{
  this->outerInstance = outerInstance;
  this->hits = hits;
}

bool TestPointQueries::ThreadAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2::needsScores()
{
  return false;
}

void TestPointQueries::ThreadAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2::doSetNextReader(
        shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

void TestPointQueries::ThreadAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2::collect(int doc)
{
  hits->set(docBase + doc);
}

wstring TestPointQueries::bytesToString(std::deque<char> &bytes)
{
  if (bytes.empty()) {
    return L"null";
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return (make_shared<BytesRef>(bytes))->toString();
}

bool TestPointQueries::matches(int bytesPerDim,
                               std::deque<std::deque<char>> &lower,
                               std::deque<std::deque<char>> &upper,
                               std::deque<std::deque<char>> &value)
{
  int numDims = lower.size();
  for (int dim = 0; dim < numDims; dim++) {

    if (StringHelper::compare(bytesPerDim, value[dim], 0, lower[dim], 0) < 0) {
      // Value is below the lower bound, on this dim
      return false;
    }

    if (StringHelper::compare(bytesPerDim, value[dim], 0, upper[dim], 0) > 0) {
      // Value is above the upper bound, on this dim
      return false;
    }
  }

  return true;
}

int64_t TestPointQueries::randomValue()
{
  if (valueRange == 0) {
    return random()->nextLong();
  } else {
    return valueMid + TestUtil::nextInt(random(), -valueRange, valueRange);
  }
}

void TestPointQueries::testMinMaxLong() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<LongPoint>(L"value", numeric_limits<int64_t>::min()));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<LongPoint>(L"value", numeric_limits<int64_t>::max()));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  TestUtil::assertEquals(1,
                         s->count(LongPoint::newRangeQuery(
                             L"value", numeric_limits<int64_t>::min(), 0LL)));
  TestUtil::assertEquals(
      1, s->count(LongPoint::newRangeQuery(L"value", 0LL,
                                           numeric_limits<int64_t>::max())));
  TestUtil::assertEquals(2, s->count(LongPoint::newRangeQuery(
                                L"value", numeric_limits<int64_t>::min(),
                                numeric_limits<int64_t>::max())));

  IOUtils::close({r, w, dir});
}

std::deque<char> TestPointQueries::toUTF8(const wstring &s)
{
  return s.getBytes(StandardCharsets::UTF_8);
}

std::deque<char> TestPointQueries::toUTF8(const wstring &s, int length)
{
  std::deque<char> bytes = s.getBytes(StandardCharsets::UTF_8);
  if (length < bytes.size()) {
    throw invalid_argument(L"length=" + to_wstring(length) +
                           L" but string's UTF8 bytes has length=" +
                           bytes.size());
  }
  std::deque<char> result(length);
  System::arraycopy(bytes, 0, result, 0, bytes.size());
  return result;
}

void TestPointQueries::testBasicSortedSet() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"value", toUTF8(L"abc")));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"value", toUTF8(L"def")));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  TestUtil::assertEquals(1, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8(L"aaa"), toUTF8(L"bbb"))));
  TestUtil::assertEquals(1, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8(L"c", 3), toUTF8(L"e", 3))));
  TestUtil::assertEquals(2, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8(L"a", 3), toUTF8(L"z", 3))));
  TestUtil::assertEquals(1, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8(L"", 3), toUTF8(L"abc"))));
  TestUtil::assertEquals(1, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8(L"a", 3), toUTF8(L"abc"))));
  TestUtil::assertEquals(0, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8(L"a", 3), toUTF8(L"abb"))));
  TestUtil::assertEquals(1, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8(L"def"), toUTF8(L"zzz"))));
  TestUtil::assertEquals(1, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8((L"def")), toUTF8(L"z", 3))));
  TestUtil::assertEquals(0, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8(L"deg"), toUTF8(L"z", 3))));

  IOUtils::close({r, w, dir});
}

void TestPointQueries::testLongMinMaxNumeric() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<LongPoint>(L"value", numeric_limits<int64_t>::min()));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<LongPoint>(L"value", numeric_limits<int64_t>::max()));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  TestUtil::assertEquals(2, s->count(LongPoint::newRangeQuery(
                                L"value", numeric_limits<int64_t>::min(),
                                numeric_limits<int64_t>::max())));
  TestUtil::assertEquals(1, s->count(LongPoint::newRangeQuery(
                                L"value", numeric_limits<int64_t>::min(),
                                numeric_limits<int64_t>::max() - 1)));
  TestUtil::assertEquals(1, s->count(LongPoint::newRangeQuery(
                                L"value", numeric_limits<int64_t>::min() + 1,
                                numeric_limits<int64_t>::max())));
  TestUtil::assertEquals(0, s->count(LongPoint::newRangeQuery(
                                L"value", numeric_limits<int64_t>::min() + 1,
                                numeric_limits<int64_t>::max() - 1)));

  IOUtils::close({r, w, dir});
}

void TestPointQueries::testLongMinMaxSortedSet() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<LongPoint>(L"value", numeric_limits<int64_t>::min()));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<LongPoint>(L"value", numeric_limits<int64_t>::max()));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  TestUtil::assertEquals(2, s->count(LongPoint::newRangeQuery(
                                L"value", numeric_limits<int64_t>::min(),
                                numeric_limits<int64_t>::max())));
  TestUtil::assertEquals(1, s->count(LongPoint::newRangeQuery(
                                L"value", numeric_limits<int64_t>::min(),
                                numeric_limits<int64_t>::max() - 1)));
  TestUtil::assertEquals(1, s->count(LongPoint::newRangeQuery(
                                L"value", numeric_limits<int64_t>::min() + 1,
                                numeric_limits<int64_t>::max())));
  TestUtil::assertEquals(0, s->count(LongPoint::newRangeQuery(
                                L"value", numeric_limits<int64_t>::min() + 1,
                                numeric_limits<int64_t>::max() - 1)));

  IOUtils::close({r, w, dir});
}

void TestPointQueries::testSortedSetNoOrdsMatch() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"value", toUTF8(L"a")));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryPoint>(L"value", toUTF8(L"z")));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(0, s->count(BinaryPoint::newRangeQuery(
                                L"value", toUTF8(L"m"), toUTF8(L"m"))));

  IOUtils::close({r, w, dir});
}

void TestPointQueries::testNumericNoValuesMatch() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 17));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 22));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  TestUtil::assertEquals(
      0, s->count(LongPoint::newRangeQuery(L"value", 17LL, 13LL)));

  IOUtils::close({r, w, dir});
}

void TestPointQueries::testNoDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  w->addDocument(make_shared<Document>());

  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(
      0, s->count(LongPoint::newRangeQuery(L"value", 17LL, 13LL)));

  IOUtils::close({r, w, dir});
}

void TestPointQueries::testWrongNumDims() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<LongPoint>(L"value", numeric_limits<int64_t>::min()));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();

  // no wrapping, else the exc might happen in executor thread:
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  std::deque<std::deque<char>> point(2);
  point[0] = std::deque<char>(8);
  point[1] = std::deque<char>(8);
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    s->count(BinaryPoint::newRangeQuery(L"value", point, point));
  });
  TestUtil::assertEquals(L"field=\"value\" was indexed with numDims=1 but this "
                         L"query has numDims=2",
                         expected.what());

  IOUtils::close({r, w, dir});
}

void TestPointQueries::testWrongNumBytes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<LongPoint>(L"value", numeric_limits<int64_t>::min()));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();

  // no wrapping, else the exc might happen in executor thread:
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  std::deque<std::deque<char>> point(1);
  point[0] = std::deque<char>(10);
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    s->count(BinaryPoint::newRangeQuery(L"value", point, point));
  });
  TestUtil::assertEquals(L"field=\"value\" was indexed with bytesPerDim=8 but "
                         L"this query has bytesPerDim=10",
                         expected.what());

  IOUtils::close({r, w, dir});
}

void TestPointQueries::testAllPointDocsWereDeletedAndThenMergedAgain() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  doc->push_back(make_shared<LongPoint>(L"value", 0LL));
  w->addDocument(doc);

  // Add document that won't be deleted to avoid IW dropping
  // segment below since it's 100% deleted:
  w->addDocument(make_shared<Document>());
  w->commit();

  // Need another segment so we invoke BKDWriter.merge
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  doc->push_back(make_shared<LongPoint>(L"value", 0LL));
  w->addDocument(doc);
  w->addDocument(make_shared<Document>());

  w->deleteDocuments({make_shared<Term>(L"id", L"0")});
  w->forceMerge(1);

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  doc->push_back(make_shared<LongPoint>(L"value", 0LL));
  w->addDocument(doc);
  w->addDocument(make_shared<Document>());

  w->deleteDocuments({make_shared<Term>(L"id", L"0")});
  w->forceMerge(1);

  IOUtils::close({w, dir});
}

shared_ptr<Codec> TestPointQueries::getCodec()
{
  if (Codec::getDefault()->getName() == L"Lucene70") {
    int maxPointsInLeafNode = TestUtil::nextInt(random(), 16, 2048);
    double maxMBSortInHeap = 5.0 + (3 * random()->nextDouble());
    if (VERBOSE) {
      wcout << L"TEST: using Lucene60PointsFormat with maxPointsInLeafNode="
            << maxPointsInLeafNode << L" and maxMBSortInHeap="
            << maxMBSortInHeap << endl;
    }

    return make_shared<FilterCodecAnonymousInnerClass>(
        Codec::getDefault(), maxPointsInLeafNode, maxMBSortInHeap);
  } else {
    return Codec::getDefault();
  }
}

TestPointQueries::FilterCodecAnonymousInnerClass::
    FilterCodecAnonymousInnerClass(shared_ptr<Codec> getDefault,
                                   int maxPointsInLeafNode,
                                   double maxMBSortInHeap)
    : org::apache::lucene::codecs::FilterCodec(L"Lucene70", getDefault)
{
  this->maxPointsInLeafNode = maxPointsInLeafNode;
  this->maxMBSortInHeap = maxMBSortInHeap;
}

shared_ptr<PointsFormat>
TestPointQueries::FilterCodecAnonymousInnerClass::pointsFormat()
{
  return make_shared<PointsFormatAnonymousInnerClass>(shared_from_this());
}

TestPointQueries::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::PointsFormatAnonymousInnerClass(
        shared_ptr<FilterCodecAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PointsWriter> TestPointQueries::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::fieldsWriter(
        shared_ptr<SegmentWriteState> writeState) 
{
  return make_shared<Lucene60PointsWriter>(writeState,
                                           outerInstance->maxPointsInLeafNode,
                                           outerInstance->maxMBSortInHeap);
}

shared_ptr<PointsReader> TestPointQueries::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::fieldsReader(
        shared_ptr<SegmentReadState> readState) 
{
  return make_shared<Lucene60PointsReader>(readState);
}

void TestPointQueries::testExactPoints() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<LongPoint>(L"long", 5LL));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 42));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"float", 2.0f));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<DoublePoint>(L"double", 1.0));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(1, s->count(IntPoint::newExactQuery(L"int", 42)));
  TestUtil::assertEquals(0, s->count(IntPoint::newExactQuery(L"int", 41)));

  TestUtil::assertEquals(1, s->count(LongPoint::newExactQuery(L"long", 5LL)));
  TestUtil::assertEquals(0, s->count(LongPoint::newExactQuery(L"long", -1LL)));

  TestUtil::assertEquals(1,
                         s->count(FloatPoint::newExactQuery(L"float", 2.0f)));
  TestUtil::assertEquals(0,
                         s->count(FloatPoint::newExactQuery(L"float", 1.0f)));

  TestUtil::assertEquals(1,
                         s->count(DoublePoint::newExactQuery(L"double", 1.0)));
  TestUtil::assertEquals(0,
                         s->count(DoublePoint::newExactQuery(L"double", 2.0)));
  delete w;
  delete r;
  delete dir;
}

void TestPointQueries::testToString() 
{

  // ints
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"field:[1 TO 2]",
                         IntPoint::newRangeQuery(L"field", 1, 2)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"field:[-2 TO 1]",
                         IntPoint::newRangeQuery(L"field", -2, 1)->toString());

  // longs
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"field:[1099511627776 TO 2199023255552]",
      LongPoint::newRangeQuery(L"field", 1LL << 40, 1LL << 41)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"field:[-5 TO 6]",
      LongPoint::newRangeQuery(L"field", -5LL, 6LL)->toString());

  // floats
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"field:[1.3 TO 2.5]",
      FloatPoint::newRangeQuery(L"field", 1.3F, 2.5F)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"field:[-2.9 TO 1.0]",
      FloatPoint::newRangeQuery(L"field", -2.9F, 1.0F)->toString());

  // doubles
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"field:[1.3 TO 2.5]",
      DoublePoint::newRangeQuery(L"field", 1.3, 2.5)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"field:[-2.9 TO 1.0]",
      DoublePoint::newRangeQuery(L"field", -2.9, 1.0)->toString());

  // n-dimensional double
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"field:[1.3 TO 2.5],[-2.9 TO 1.0]",
      DoublePoint::newRangeQuery(L"field", std::deque<double>{1.3, -2.9},
                                 std::deque<double>{2.5, 1.0})
          ->toString());
}

std::deque<int> TestPointQueries::toArray(shared_ptr<Set<int>> valuesSet)
{
  std::deque<int> values(valuesSet->size());
  int upto = 0;
  for (shared_ptr<> : : optional<int> value : valuesSet) {
    values[upto++] = value;
  }
  return values;
}

int TestPointQueries::randomIntValue(optional<int> &min, optional<int> &max)
{
  if (!min) {
    return random()->nextInt();
  } else {
    return TestUtil::nextInt(random(), min, max);
  }
}

void TestPointQueries::testRandomPointInSetQuery() 
{

  bool useNarrowRange = random()->nextBoolean();
  const optional<int> valueMin;
  const optional<int> valueMax;
  int numValues;
  if (useNarrowRange) {
    int gap = random()->nextInt(100);
    valueMin = random()->nextInt(numeric_limits<int>::max() - gap);
    valueMax = valueMin + gap;
    numValues = TestUtil::nextInt(random(), 1, gap + 1);
  } else {
    valueMin = nullopt;
    valueMax = nullopt;
    numValues = TestUtil::nextInt(random(), 1, 100);
  }
  shared_ptr<Set<int>> *const valuesSet = unordered_set<int>();
  while (valuesSet->size() < numValues) {
    valuesSet->add(randomIntValue(valueMin, valueMax));
  }
  std::deque<int> values = toArray(valuesSet);
  int numDocs = TestUtil::nextInt(random(), 1, 10000);

  if (VERBOSE) {
    wcout << L"TEST: numValues=" << numValues << L" numDocs=" << numDocs
          << endl;
  }

  shared_ptr<Directory> dir;
  if (numDocs > 100000) {
    dir = newFSDirectory(createTempDir(L"TestPointQueries"));
  } else {
    dir = newDirectory();
  }

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  std::deque<int> docValues(numDocs);
  for (int i = 0; i < numDocs; i++) {
    int x = values[random()->nextInt(values.size())];
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<IntPoint>(L"int", x));
    docValues[i] = x;
    w->addDocument(doc);
  }

  if (random()->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"  forceMerge(1)" << endl;
    }
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  int numThreads = TestUtil::nextInt(random(), 2, 5);

  if (VERBOSE) {
    wcout << L"TEST: use " << numThreads << L" query threads; searcher=" << s
          << endl;
  }

  deque<std::shared_ptr<Thread>> threads = deque<std::shared_ptr<Thread>>();
  constexpr int iters = atLeast(100);

  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  shared_ptr<AtomicBoolean> *const failed = make_shared<AtomicBoolean>();

  for (int i = 0; i < numThreads; i++) {
    shared_ptr<Thread> thread = make_shared<ThreadAnonymousInnerClass3>(
        shared_from_this(), values, docValues, s, iters, startingGun, failed);
    thread->setName(L"T" + to_wstring(i));
    thread->start();
    threads.push_back(thread);
  }
  startingGun->countDown();
  for (auto thread : threads) {
    thread->join();
  }
  IOUtils::close({r, dir});
}

TestPointQueries::ThreadAnonymousInnerClass3::ThreadAnonymousInnerClass3(
    shared_ptr<TestPointQueries> outerInstance, deque<int> &values,
    deque<int> &docValues,
    shared_ptr<org::apache::lucene::search::IndexSearcher> s, int iters,
    shared_ptr<CountDownLatch> startingGun, shared_ptr<AtomicBoolean> failed)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->docValues = docValues;
  this->s = s;
  this->iters = iters;
  this->startingGun = startingGun;
  this->failed = failed;
}

void TestPointQueries::ThreadAnonymousInnerClass3::run()
{
  try {
    _run();
  } catch (const runtime_error &e) {
    failed->set(true);
    throw runtime_error(e);
  }
}

void TestPointQueries::ThreadAnonymousInnerClass3::_run() 
{
  startingGun->await();

  for (int iter = 0; iter < iters && failed->get() == false; iter++) {

    int numValidValuesToQuery =
        LuceneTestCase::random()->nextInt(values.size());

    shared_ptr<Set<int>> valuesToQuery = unordered_set<int>();
    while (valuesToQuery->size() < numValidValuesToQuery) {
      valuesToQuery->add(
          values[LuceneTestCase::random()->nextInt(values.size())]);
    }

    int numExtraValuesToQuery = LuceneTestCase::random()->nextInt(20);
    while (valuesToQuery->size() <
           numValidValuesToQuery + numExtraValuesToQuery) {
      valuesToQuery->add(LuceneTestCase::random()->nextInt());
    }

    int expectedCount = 0;
    for (auto value : docValues) {
      if (valuesToQuery->contains(value)) {
        expectedCount++;
      }
    }

    if (VERBOSE) {
      wcout << L"TEST: thread=" << Thread::currentThread() << L" values="
            << valuesToQuery << L" expectedCount=" << expectedCount << endl;
    }

    TestUtil::assertEquals(expectedCount,
                           s->count(IntPoint::newSetQuery(
                               L"int", outerInstance->toArray(valuesToQuery))));
  }
}

shared_ptr<Query> TestPointQueries::newMultiDimIntSetQuery(
    const wstring &field, int const numDims,
    deque<int> &valuesIn) 
{
  if (valuesIn->length % numDims != 0) {
    throw invalid_argument(L"incongruent number of values: valuesIn.length=" +
                           valuesIn->length + L" but numDims=" +
                           to_wstring(numDims));
  }

  // Pack all values:
  std::deque<std::deque<char>> packedValues(valuesIn->length / numDims);
  for (int i = 0; i < packedValues.size(); i++) {
    std::deque<char> packedValue(numDims * Integer::BYTES);
    packedValues[i] = packedValue;
    for (int dim = 0; dim < numDims; dim++) {
      IntPoint::encodeDimension(valuesIn[i * numDims + dim], packedValue,
                                dim * Integer::BYTES);
    }
  }

  // Sort:
  Arrays::sort(packedValues, make_shared<ComparatorAnonymousInnerClass>());

  shared_ptr<BytesRef> *const value = make_shared<BytesRef>();
  value->length = numDims * Integer::BYTES;

  return make_shared<PointInSetQuery>(
      field, numDims, Integer::BYTES,
      make_shared<StreamAnonymousInnerClass>(packedValues, value)){
    protected :
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wstring Term::toString(char[] value){
            assert(value->length == numDims * Integer::BYTES);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int dim = 0; dim < numDims; dim++) {
    if (dim > 0) {
      sb->append(L',');
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(Integer::toString(
        IntPoint::decodeDimension(value, dim * Integer::BYTES)));
  }

  return sb->toString();
}
}; // namespace org::apache::lucene::search
}

TestPointQueries::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass()
{
}

int TestPointQueries::ComparatorAnonymousInnerClass::compare(
    std::deque<char> &a, std::deque<char> &b)
{
  return StringHelper::compare(a.size(), a, 0, b, 0);
}

TestPointQueries::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    deque<deque<char>> &packedValues, shared_ptr<BytesRef> value)
{
  this->packedValues = packedValues;
  this->value = value;
}

shared_ptr<BytesRef> TestPointQueries::StreamAnonymousInnerClass::next()
{
  if (upto >= packedValues.size()) {
    return nullptr;
  }
  value->bytes = packedValues[upto];
  upto++;
  return value;
}

void TestPointQueries::testBasicMultiDimPointInSetQuery() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 17, 42));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  TestUtil::assertEquals(0,
                         s->count(newMultiDimIntSetQuery(L"int", 2, {17, 41})));
  TestUtil::assertEquals(1,
                         s->count(newMultiDimIntSetQuery(L"int", 2, {17, 42})));
  TestUtil::assertEquals(
      1, s->count(newMultiDimIntSetQuery(L"int", 2, {-7, -7, 17, 42})));
  TestUtil::assertEquals(
      1, s->count(newMultiDimIntSetQuery(L"int", 2, {17, 42, -14, -14})));

  delete w;
  delete r;
  delete dir;
}

void TestPointQueries::testBasicMultiValueMultiDimPointInSetQuery() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 17, 42));
  doc->push_back(make_shared<IntPoint>(L"int", 34, 79));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  TestUtil::assertEquals(0,
                         s->count(newMultiDimIntSetQuery(L"int", 2, {17, 41})));
  TestUtil::assertEquals(1,
                         s->count(newMultiDimIntSetQuery(L"int", 2, {17, 42})));
  TestUtil::assertEquals(
      1, s->count(newMultiDimIntSetQuery(L"int", 2, {17, 42, 34, 79})));
  TestUtil::assertEquals(
      1, s->count(newMultiDimIntSetQuery(L"int", 2, {-7, -7, 17, 42})));
  TestUtil::assertEquals(
      1, s->count(newMultiDimIntSetQuery(L"int", 2, {-7, -7, 34, 79})));
  TestUtil::assertEquals(
      1, s->count(newMultiDimIntSetQuery(L"int", 2, {17, 42, -14, -14})));

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"int:{-14,-14 17,42}",
      newMultiDimIntSetQuery(L"int", 2, {17, 42, -14, -14})->toString());

  delete w;
  delete r;
  delete dir;
}

void TestPointQueries::testManyEqualValuesMultiDimPointInSetQuery() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  int zeroCount = 0;
  for (int i = 0; i < 10000; i++) {
    int x = random()->nextInt(2);
    if (x == 0) {
      zeroCount++;
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<IntPoint>(L"int", x, x));
    w->addDocument(doc);
  }
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  TestUtil::assertEquals(zeroCount,
                         s->count(newMultiDimIntSetQuery(L"int", 2, {0, 0})));
  TestUtil::assertEquals(10000 - zeroCount,
                         s->count(newMultiDimIntSetQuery(L"int", 2, {1, 1})));
  TestUtil::assertEquals(0,
                         s->count(newMultiDimIntSetQuery(L"int", 2, {2, 2})));

  delete w;
  delete r;
  delete dir;
}

void TestPointQueries::testInvalidMultiDimPointInSetQuery() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    newMultiDimIntSetQuery(L"int", 2, {3, 4, 5});
  });
  TestUtil::assertEquals(
      L"incongruent number of values: valuesIn.length=3 but numDims=2",
      expected.what());
}

void TestPointQueries::testBasicPointInSetQuery() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 17));
  doc->push_back(make_shared<LongPoint>(L"long", 17LL));
  doc->push_back(make_shared<FloatPoint>(L"float", 17.0f));
  doc->push_back(make_shared<DoublePoint>(L"double", 17.0));
  doc->push_back(make_shared<BinaryPoint>(L"bytes", std::deque<char>{0, 17}));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 42));
  doc->push_back(make_shared<LongPoint>(L"long", 42LL));
  doc->push_back(make_shared<FloatPoint>(L"float", 42.0f));
  doc->push_back(make_shared<DoublePoint>(L"double", 42.0));
  doc->push_back(make_shared<BinaryPoint>(L"bytes", std::deque<char>{0, 42}));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 97));
  doc->push_back(make_shared<LongPoint>(L"long", 97LL));
  doc->push_back(make_shared<FloatPoint>(L"float", 97.0f));
  doc->push_back(make_shared<DoublePoint>(L"double", 97.0));
  doc->push_back(make_shared<BinaryPoint>(L"bytes", std::deque<char>{0, 97}));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(0, s->count(IntPoint::newSetQuery(L"int", {16})));
  TestUtil::assertEquals(1, s->count(IntPoint::newSetQuery(L"int", {17})));
  TestUtil::assertEquals(3,
                         s->count(IntPoint::newSetQuery(L"int", {17, 97, 42})));
  TestUtil::assertEquals(
      3, s->count(IntPoint::newSetQuery(L"int", {-7, 17, 42, 97})));
  TestUtil::assertEquals(
      3, s->count(IntPoint::newSetQuery(L"int", {17, 20, 42, 97})));
  TestUtil::assertEquals(
      3, s->count(IntPoint::newSetQuery(L"int", {17, 105, 42, 97})));

  TestUtil::assertEquals(0, s->count(LongPoint::newSetQuery(L"long", 16)));
  TestUtil::assertEquals(1, s->count(LongPoint::newSetQuery(L"long", 17)));
  TestUtil::assertEquals(3,
                         s->count(LongPoint::newSetQuery(L"long", 17, 97, 42)));
  TestUtil::assertEquals(
      3, s->count(LongPoint::newSetQuery(L"long", -7, 17, 42, 97)));
  TestUtil::assertEquals(
      3, s->count(LongPoint::newSetQuery(L"long", 17, 20, 42, 97)));
  TestUtil::assertEquals(
      3, s->count(LongPoint::newSetQuery(L"long", 17, 105, 42, 97)));

  TestUtil::assertEquals(0, s->count(FloatPoint::newSetQuery(L"float", 16)));
  TestUtil::assertEquals(1, s->count(FloatPoint::newSetQuery(L"float", 17)));
  TestUtil::assertEquals(
      3, s->count(FloatPoint::newSetQuery(L"float", 17, 97, 42)));
  TestUtil::assertEquals(
      3, s->count(FloatPoint::newSetQuery(L"float", -7, 17, 42, 97)));
  TestUtil::assertEquals(
      3, s->count(FloatPoint::newSetQuery(L"float", 17, 20, 42, 97)));
  TestUtil::assertEquals(
      3, s->count(FloatPoint::newSetQuery(L"float", 17, 105, 42, 97)));

  TestUtil::assertEquals(0, s->count(DoublePoint::newSetQuery(L"double", 16)));
  TestUtil::assertEquals(1, s->count(DoublePoint::newSetQuery(L"double", 17)));
  TestUtil::assertEquals(
      3, s->count(DoublePoint::newSetQuery(L"double", 17, 97, 42)));
  TestUtil::assertEquals(
      3, s->count(DoublePoint::newSetQuery(L"double", -7, 17, 42, 97)));
  TestUtil::assertEquals(
      3, s->count(DoublePoint::newSetQuery(L"double", 17, 20, 42, 97)));
  TestUtil::assertEquals(
      3, s->count(DoublePoint::newSetQuery(L"double", 17, 105, 42, 97)));

  TestUtil::assertEquals(0, s->count(BinaryPoint::newSetQuery(
                                L"bytes", std::deque<char>{0, 16})));
  TestUtil::assertEquals(1, s->count(BinaryPoint::newSetQuery(
                                L"bytes", std::deque<char>{0, 17})));
  TestUtil::assertEquals(
      3, s->count(BinaryPoint::newSetQuery(
             L"bytes", {std::deque<char>{0, 17}, std::deque<char>{0, 97},
                        std::deque<char>{0, 42}})));
  TestUtil::assertEquals(
      3, s->count(BinaryPoint::newSetQuery(
             L"bytes", {std::deque<char>{0, -7}, std::deque<char>{0, 17},
                        std::deque<char>{0, 42}, std::deque<char>{0, 97}})));
  TestUtil::assertEquals(
      3, s->count(BinaryPoint::newSetQuery(
             L"bytes", {std::deque<char>{0, 17}, std::deque<char>{0, 20},
                        std::deque<char>{0, 42}, std::deque<char>{0, 97}})));
  TestUtil::assertEquals(
      3, s->count(BinaryPoint::newSetQuery(
             L"bytes", {std::deque<char>{0, 17}, std::deque<char>{0, 105},
                        std::deque<char>{0, 42}, std::deque<char>{0, 97}})));

  delete w;
  delete r;
  delete dir;
}

void TestPointQueries::testPointIntSetBoxed() 
{
  TestUtil::assertEquals(
      IntPoint::newSetQuery(L"foo", {1, 2, 3}),
      IntPoint::newSetQuery(L"foo", Arrays::asList(1, 2, 3)));
  TestUtil::assertEquals(
      FloatPoint::newSetQuery(L"foo", {1.0F, 2.0F, 3.0F}),
      FloatPoint::newSetQuery(L"foo", Arrays::asList(1.0F, 2.0F, 3.0F)));
  TestUtil::assertEquals(
      LongPoint::newSetQuery(L"foo", {1LL, 2LL, 3LL}),
      LongPoint::newSetQuery(L"foo", Arrays::asList(1LL, 2LL, 3LL)));
  TestUtil::assertEquals(
      DoublePoint::newSetQuery(L"foo", {1, 2, 3}),
      DoublePoint::newSetQuery(L"foo", Arrays::asList(1, 2, 3)));
}

void TestPointQueries::testBasicMultiValuedPointInSetQuery() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 17));
  doc->push_back(make_shared<IntPoint>(L"int", 42));
  doc->push_back(make_shared<LongPoint>(L"long", 17LL));
  doc->push_back(make_shared<LongPoint>(L"long", 42LL));
  doc->push_back(make_shared<FloatPoint>(L"float", 17.0f));
  doc->push_back(make_shared<FloatPoint>(L"float", 42.0f));
  doc->push_back(make_shared<DoublePoint>(L"double", 17.0));
  doc->push_back(make_shared<DoublePoint>(L"double", 42.0));
  doc->push_back(make_shared<BinaryPoint>(L"bytes", std::deque<char>{0, 17}));
  doc->push_back(make_shared<BinaryPoint>(L"bytes", std::deque<char>{0, 42}));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(0, s->count(IntPoint::newSetQuery(L"int", {16})));
  TestUtil::assertEquals(1, s->count(IntPoint::newSetQuery(L"int", {17})));
  TestUtil::assertEquals(1,
                         s->count(IntPoint::newSetQuery(L"int", {17, 97, 42})));
  TestUtil::assertEquals(
      1, s->count(IntPoint::newSetQuery(L"int", {-7, 17, 42, 97})));
  TestUtil::assertEquals(
      0, s->count(IntPoint::newSetQuery(L"int", {16, 20, 41, 97})));

  TestUtil::assertEquals(0, s->count(LongPoint::newSetQuery(L"long", 16)));
  TestUtil::assertEquals(1, s->count(LongPoint::newSetQuery(L"long", 17)));
  TestUtil::assertEquals(1,
                         s->count(LongPoint::newSetQuery(L"long", 17, 97, 42)));
  TestUtil::assertEquals(
      1, s->count(LongPoint::newSetQuery(L"long", -7, 17, 42, 97)));
  TestUtil::assertEquals(
      0, s->count(LongPoint::newSetQuery(L"long", 16, 20, 41, 97)));

  TestUtil::assertEquals(0, s->count(FloatPoint::newSetQuery(L"float", 16)));
  TestUtil::assertEquals(1, s->count(FloatPoint::newSetQuery(L"float", 17)));
  TestUtil::assertEquals(
      1, s->count(FloatPoint::newSetQuery(L"float", 17, 97, 42)));
  TestUtil::assertEquals(
      1, s->count(FloatPoint::newSetQuery(L"float", -7, 17, 42, 97)));
  TestUtil::assertEquals(
      0, s->count(FloatPoint::newSetQuery(L"float", 16, 20, 41, 97)));

  TestUtil::assertEquals(0, s->count(DoublePoint::newSetQuery(L"double", 16)));
  TestUtil::assertEquals(1, s->count(DoublePoint::newSetQuery(L"double", 17)));
  TestUtil::assertEquals(
      1, s->count(DoublePoint::newSetQuery(L"double", 17, 97, 42)));
  TestUtil::assertEquals(
      1, s->count(DoublePoint::newSetQuery(L"double", -7, 17, 42, 97)));
  TestUtil::assertEquals(
      0, s->count(DoublePoint::newSetQuery(L"double", 16, 20, 41, 97)));

  TestUtil::assertEquals(0, s->count(BinaryPoint::newSetQuery(
                                L"bytes", std::deque<char>{0, 16})));
  TestUtil::assertEquals(1, s->count(BinaryPoint::newSetQuery(
                                L"bytes", std::deque<char>{0, 17})));
  TestUtil::assertEquals(
      1, s->count(BinaryPoint::newSetQuery(
             L"bytes", {std::deque<char>{0, 17}, std::deque<char>{0, 97},
                        std::deque<char>{0, 42}})));
  TestUtil::assertEquals(
      1, s->count(BinaryPoint::newSetQuery(
             L"bytes", {std::deque<char>{0, -7}, std::deque<char>{0, 17},
                        std::deque<char>{0, 42}, std::deque<char>{0, 97}})));
  TestUtil::assertEquals(
      0, s->count(BinaryPoint::newSetQuery(
             L"bytes", {std::deque<char>{0, 16}, std::deque<char>{0, 20},
                        std::deque<char>{0, 41}, std::deque<char>{0, 97}})));

  delete w;
  delete r;
  delete dir;
}

void TestPointQueries::testEmptyPointInSetQuery() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"int", 17));
  doc->push_back(make_shared<LongPoint>(L"long", 17LL));
  doc->push_back(make_shared<FloatPoint>(L"float", 17.0f));
  doc->push_back(make_shared<DoublePoint>(L"double", 17.0));
  doc->push_back(make_shared<BinaryPoint>(L"bytes", std::deque<char>{0, 17}));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(0, s->count(IntPoint::newSetQuery(L"int")));
  TestUtil::assertEquals(0, s->count(LongPoint::newSetQuery(L"long")));
  TestUtil::assertEquals(0, s->count(FloatPoint::newSetQuery(L"float")));
  TestUtil::assertEquals(0, s->count(DoublePoint::newSetQuery(L"double")));
  TestUtil::assertEquals(0, s->count(BinaryPoint::newSetQuery(L"bytes")));

  delete w;
  delete r;
  delete dir;
}

void TestPointQueries::testPointInSetQueryManyEqualValues() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  int zeroCount = 0;
  for (int i = 0; i < 10000; i++) {
    int x = random()->nextInt(2);
    if (x == 0) {
      zeroCount++;
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<IntPoint>(L"int", x));
    doc->push_back(make_shared<LongPoint>(L"long", static_cast<int64_t>(x)));
    doc->push_back(make_shared<FloatPoint>(L"float", static_cast<float>(x)));
    doc->push_back(make_shared<DoublePoint>(L"double", static_cast<double>(x)));
    doc->push_back(make_shared<BinaryPoint>(
        L"bytes", std::deque<char>{static_cast<char>(x)}));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(zeroCount,
                         s->count(IntPoint::newSetQuery(L"int", {0})));
  TestUtil::assertEquals(zeroCount,
                         s->count(IntPoint::newSetQuery(L"int", {0, -7})));
  TestUtil::assertEquals(zeroCount,
                         s->count(IntPoint::newSetQuery(L"int", {7, 0})));
  TestUtil::assertEquals(10000 - zeroCount,
                         s->count(IntPoint::newSetQuery(L"int", {1})));
  TestUtil::assertEquals(0, s->count(IntPoint::newSetQuery(L"int", {2})));

  TestUtil::assertEquals(zeroCount,
                         s->count(LongPoint::newSetQuery(L"long", 0)));
  TestUtil::assertEquals(zeroCount,
                         s->count(LongPoint::newSetQuery(L"long", 0, -7)));
  TestUtil::assertEquals(zeroCount,
                         s->count(LongPoint::newSetQuery(L"long", 7, 0)));
  TestUtil::assertEquals(10000 - zeroCount,
                         s->count(LongPoint::newSetQuery(L"long", 1)));
  TestUtil::assertEquals(0, s->count(LongPoint::newSetQuery(L"long", 2)));

  TestUtil::assertEquals(zeroCount,
                         s->count(FloatPoint::newSetQuery(L"float", 0)));
  TestUtil::assertEquals(zeroCount,
                         s->count(FloatPoint::newSetQuery(L"float", 0, -7)));
  TestUtil::assertEquals(zeroCount,
                         s->count(FloatPoint::newSetQuery(L"float", 7, 0)));
  TestUtil::assertEquals(10000 - zeroCount,
                         s->count(FloatPoint::newSetQuery(L"float", 1)));
  TestUtil::assertEquals(0, s->count(FloatPoint::newSetQuery(L"float", 2)));

  TestUtil::assertEquals(zeroCount,
                         s->count(DoublePoint::newSetQuery(L"double", 0)));
  TestUtil::assertEquals(zeroCount,
                         s->count(DoublePoint::newSetQuery(L"double", 0, -7)));
  TestUtil::assertEquals(zeroCount,
                         s->count(DoublePoint::newSetQuery(L"double", 7, 0)));
  TestUtil::assertEquals(10000 - zeroCount,
                         s->count(DoublePoint::newSetQuery(L"double", 1)));
  TestUtil::assertEquals(0, s->count(DoublePoint::newSetQuery(L"double", 2)));

  TestUtil::assertEquals(zeroCount, s->count(BinaryPoint::newSetQuery(
                                        L"bytes", std::deque<char>{0})));
  TestUtil::assertEquals(
      zeroCount, s->count(BinaryPoint::newSetQuery(
                     L"bytes", {std::deque<char>{0}, std::deque<char>{-7}})));
  TestUtil::assertEquals(
      zeroCount, s->count(BinaryPoint::newSetQuery(
                     L"bytes", {std::deque<char>{7}, std::deque<char>{0}})));
  TestUtil::assertEquals(
      10000 - zeroCount,
      s->count(BinaryPoint::newSetQuery(L"bytes", std::deque<char>{1})));
  TestUtil::assertEquals(
      0, s->count(BinaryPoint::newSetQuery(L"bytes", std::deque<char>{2})));

  delete w;
  delete r;
  delete dir;
}

void TestPointQueries::testPointInSetQueryManyEqualValuesWithBigGap() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  int zeroCount = 0;
  for (int i = 0; i < 10000; i++) {
    int x = 200 * random()->nextInt(2);
    if (x == 0) {
      zeroCount++;
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<IntPoint>(L"int", x));
    doc->push_back(make_shared<LongPoint>(L"long", static_cast<int64_t>(x)));
    doc->push_back(make_shared<FloatPoint>(L"float", static_cast<float>(x)));
    doc->push_back(make_shared<DoublePoint>(L"double", static_cast<double>(x)));
    doc->push_back(make_shared<BinaryPoint>(
        L"bytes", std::deque<char>{static_cast<char>(x)}));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(zeroCount,
                         s->count(IntPoint::newSetQuery(L"int", {0})));
  TestUtil::assertEquals(zeroCount,
                         s->count(IntPoint::newSetQuery(L"int", {0, -7})));
  TestUtil::assertEquals(zeroCount,
                         s->count(IntPoint::newSetQuery(L"int", {7, 0})));
  TestUtil::assertEquals(10000 - zeroCount,
                         s->count(IntPoint::newSetQuery(L"int", {200})));
  TestUtil::assertEquals(0, s->count(IntPoint::newSetQuery(L"int", {2})));

  TestUtil::assertEquals(zeroCount,
                         s->count(LongPoint::newSetQuery(L"long", 0)));
  TestUtil::assertEquals(zeroCount,
                         s->count(LongPoint::newSetQuery(L"long", 0, -7)));
  TestUtil::assertEquals(zeroCount,
                         s->count(LongPoint::newSetQuery(L"long", 7, 0)));
  TestUtil::assertEquals(10000 - zeroCount,
                         s->count(LongPoint::newSetQuery(L"long", 200)));
  TestUtil::assertEquals(0, s->count(LongPoint::newSetQuery(L"long", 2)));

  TestUtil::assertEquals(zeroCount,
                         s->count(FloatPoint::newSetQuery(L"float", 0)));
  TestUtil::assertEquals(zeroCount,
                         s->count(FloatPoint::newSetQuery(L"float", 0, -7)));
  TestUtil::assertEquals(zeroCount,
                         s->count(FloatPoint::newSetQuery(L"float", 7, 0)));
  TestUtil::assertEquals(10000 - zeroCount,
                         s->count(FloatPoint::newSetQuery(L"float", 200)));
  TestUtil::assertEquals(0, s->count(FloatPoint::newSetQuery(L"float", 2)));

  TestUtil::assertEquals(zeroCount,
                         s->count(DoublePoint::newSetQuery(L"double", 0)));
  TestUtil::assertEquals(zeroCount,
                         s->count(DoublePoint::newSetQuery(L"double", 0, -7)));
  TestUtil::assertEquals(zeroCount,
                         s->count(DoublePoint::newSetQuery(L"double", 7, 0)));
  TestUtil::assertEquals(10000 - zeroCount,
                         s->count(DoublePoint::newSetQuery(L"double", 200)));
  TestUtil::assertEquals(0, s->count(DoublePoint::newSetQuery(L"double", 2)));

  TestUtil::assertEquals(zeroCount, s->count(BinaryPoint::newSetQuery(
                                        L"bytes", std::deque<char>{0})));
  TestUtil::assertEquals(
      zeroCount, s->count(BinaryPoint::newSetQuery(
                     L"bytes", {std::deque<char>{0}, std::deque<char>{-7}})));
  TestUtil::assertEquals(
      zeroCount, s->count(BinaryPoint::newSetQuery(
                     L"bytes", {std::deque<char>{7}, std::deque<char>{0}})));
  TestUtil::assertEquals(
      10000 - zeroCount,
      s->count(BinaryPoint::newSetQuery(
          L"bytes", std::deque<char>{static_cast<char>(200)})));
  TestUtil::assertEquals(
      0, s->count(BinaryPoint::newSetQuery(L"bytes", std::deque<char>{2})));

  delete w;
  delete r;
  delete dir;
}

void TestPointQueries::testInvalidPointInSetQuery() 
{
  // C++ TODO: The following anonymous inner class could not be converted:
  //      IllegalArgumentException expected =
  //      expectThrows(IllegalArgumentException.class, () => { new
  //      PointInSetQuery("foo", 3, 4, new PointInSetQuery.Stream()
  //    {
  //                                                                             @@Override public BytesRef next()
  //                                                                             {
  //                                                                               return new BytesRef(new byte[3]);
  //                                                                             }
  //                                                                           }
                                                                            )
                                                                            {
                                                       protected:
//C++ TODO: There is no native C++ equivalent to 'toString':
                                                           wstring Term::toString(char[] point)
                                                           {
//C++ TODO: There is no native C++ equivalent to 'toString':
                                                             return Arrays->toString(point);
}
}
;
}

void<missing_class_definition>::testInvalidPointInSetBinaryQuery() throw(
    runtime_error)
{
  invalid_argument expected =
      LuceneTestCase::expectThrows(invalid_argument::typeid, [&]() {
        BinaryPoint::newSetQuery(L"bytes",
                                 {std::deque<char>{2}, std::deque<char>(0)});
      });
  TestUtil::assertEquals(L"all byte[] must be the same length, but saw 1 and 0",
                         expected.what());
}

void<missing_class_definition>::testPointInSetQueryToString() throw(
    runtime_error)
{
  // int
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"int:{-42 18}",
                         IntPoint::newSetQuery(L"int", {-42, 18})->toString());

  // long
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"long:{-42 18}",
      LongPoint::newSetQuery(L"long", {-42LL, 18LL})->toString());

  // float
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"float:{-42.0 18.0}",
      FloatPoint::newSetQuery(L"float", {-42.0f, 18.0f})->toString());

  // double
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"double:{-42.0 18.0}",
      DoublePoint::newSetQuery(L"double", {-42.0, 18.0})->toString());

  // binary
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"bytes:{[12] [2a]}",
      BinaryPoint::newSetQuery(L"bytes",
                               {std::deque<char>{42}, std::deque<char>{18}})
          ->toString());
}

void<missing_class_definition>::testPointInSetQueryGetPackedPoints() throw(
    runtime_error)
{
  int numValues = randomIntValue(1, 32);
  deque<std::deque<char>> values = deque<std::deque<char>>(numValues);
  for (char i = 0; i < numValues; i++) {
    values.push_back(std::deque<char>{i});
  }

  shared_ptr<PointInSetQuery> query =
      std::static_pointer_cast<PointInSetQuery>(BinaryPoint::newSetQuery(
          L"field", {values.toArray(std::deque<std::deque<char>>())}));
  shared_ptr<deque<std::deque<char>>> packedPoints =
      query->getPackedPoints();
  TestUtil::assertEquals(numValues, packedPoints->size());
  deque<std::deque<char>>::const_iterator iterator =
      packedPoints->begin();
  for (auto expectedValue : values) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    assertArrayEquals(expectedValue, iterator->next());
  }
  LuceneTestCase::expectThrows(NoSuchElementException::typeid,
                               [&]() { iterator->next(); });
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(iterator->hasNext());
}

void<missing_class_definition>::testRangeOptimizesIfAllPointsMatch() throw(
    IOException)
{
  constexpr int numDims = TestUtil::nextInt(LuceneTestCase::random(), 1, 3);
  shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(LuceneTestCase::random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  std::deque<int> value(numDims);
  for (int i = 0; i < numDims; ++i) {
    value[i] = TestUtil::nextInt(LuceneTestCase::random(), 1, 10);
  }
  doc->push_back(make_shared<IntPoint>(L"point", value));
  w->addDocument(doc);
  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(nullptr);
  std::deque<int> lowerBound(numDims);
  std::deque<int> upperBound(numDims);
  for (int i = 0; i < numDims; ++i) {
    lowerBound[i] = value[i] - LuceneTestCase::random()->nextInt(1);
    upperBound[i] = value[i] + LuceneTestCase::random()->nextInt(1);
  }
  shared_ptr<Query> query =
      IntPoint::newRangeQuery(L"point", lowerBound, upperBound);
  shared_ptr<Weight> weight = searcher->createWeight(query, false, 1);
  shared_ptr<Scorer> scorer =
      weight->scorer(searcher->getIndexReader()->leaves()[0]);
  TestUtil::assertEquals(DocIdSetIterator::all(1)->getClass(),
                         scorer->begin().getClass());

  // When not all documents in the query have a value, the optimization is not
  // applicable
  delete reader;
  w->addDocument(make_shared<Document>());
  w->forceMerge(1);
  reader = w->getReader();
  searcher = make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(nullptr);
  weight = searcher->createWeight(query, false, 1);
  scorer = weight->scorer(searcher->getIndexReader()->leaves()[0]);
  assertFalse(
      DocIdSetIterator::all(1)->getClass().equals(scorer->begin().getClass()));

  delete reader;
  delete w;
  delete dir;
}

void<missing_class_definition>::testPointRangeEquals()
{
  shared_ptr<Query> q1, q2;

  q1 = IntPoint::newRangeQuery(L"a", 0, 1000);
  q2 = IntPoint::newRangeQuery(L"a", 0, 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(IntPoint::newRangeQuery(L"a", 1, 1000)));
  assertFalse(q1->equals(IntPoint::newRangeQuery(L"b", 0, 1000)));

  q1 = LongPoint::newRangeQuery(L"a", 0, 1000);
  q2 = LongPoint::newRangeQuery(L"a", 0, 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(LongPoint::newRangeQuery(L"a", 1, 1000)));

  q1 = FloatPoint::newRangeQuery(L"a", 0, 1000);
  q2 = FloatPoint::newRangeQuery(L"a", 0, 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(FloatPoint::newRangeQuery(L"a", 1, 1000)));

  q1 = DoublePoint::newRangeQuery(L"a", 0, 1000);
  q2 = DoublePoint::newRangeQuery(L"a", 0, 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(DoublePoint::newRangeQuery(L"a", 1, 1000)));

  std::deque<char> zeros(5);
  std::deque<char> ones(5);
  Arrays::fill(ones, static_cast<char>(0xff));
  q1 = BinaryPoint::newRangeQuery(L"a", std::deque<std::deque<char>>{zeros},
                                  std::deque<std::deque<char>>{ones});
  q2 = BinaryPoint::newRangeQuery(L"a", std::deque<std::deque<char>>{zeros},
                                  std::deque<std::deque<char>>{ones});
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  std::deque<char> other = ones.clone();
  other[2] = static_cast<char>(5);
  assertFalse(q1->equals(
      BinaryPoint::newRangeQuery(L"a", std::deque<std::deque<char>>{zeros},
                                 std::deque<std::deque<char>>{other})));
}

void<missing_class_definition>::testPointExactEquals()
{
  shared_ptr<Query> q1, q2;

  q1 = IntPoint::newExactQuery(L"a", 1000);
  q2 = IntPoint::newExactQuery(L"a", 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(IntPoint::newExactQuery(L"a", 1)));
  assertFalse(q1->equals(IntPoint::newExactQuery(L"b", 1000)));

  q1 = LongPoint::newExactQuery(L"a", 1000);
  q2 = LongPoint::newExactQuery(L"a", 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(LongPoint::newExactQuery(L"a", 1)));

  q1 = FloatPoint::newExactQuery(L"a", 1000);
  q2 = FloatPoint::newExactQuery(L"a", 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(FloatPoint::newExactQuery(L"a", 1)));

  q1 = DoublePoint::newExactQuery(L"a", 1000);
  q2 = DoublePoint::newExactQuery(L"a", 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(DoublePoint::newExactQuery(L"a", 1)));

  std::deque<char> ones(5);
  Arrays::fill(ones, static_cast<char>(0xff));
  q1 = BinaryPoint::newExactQuery(L"a", ones);
  q2 = BinaryPoint::newExactQuery(L"a", ones);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  std::deque<char> other = ones.clone();
  other[2] = static_cast<char>(5);
  assertFalse(q1->equals(BinaryPoint::newExactQuery(L"a", other)));
}

void<missing_class_definition>::testPointInSetEquals()
{
  shared_ptr<Query> q1, q2;
  q1 = IntPoint::newSetQuery(L"a", {0, 1000, 17});
  q2 = IntPoint::newSetQuery(L"a", {17, 0, 1000});
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(IntPoint::newSetQuery(L"a", {1, 17, 1000})));
  assertFalse(q1->equals(IntPoint::newSetQuery(L"b", {0, 1000, 17})));

  q1 = LongPoint::newSetQuery(L"a", 0, 1000, 17);
  q2 = LongPoint::newSetQuery(L"a", 17, 0, 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(LongPoint::newSetQuery(L"a", 1, 17, 1000)));

  q1 = FloatPoint::newSetQuery(L"a", 0, 1000, 17);
  q2 = FloatPoint::newSetQuery(L"a", 17, 0, 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(FloatPoint::newSetQuery(L"a", 1, 17, 1000)));

  q1 = DoublePoint::newSetQuery(L"a", 0, 1000, 17);
  q2 = DoublePoint::newSetQuery(L"a", 17, 0, 1000);
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(DoublePoint::newSetQuery(L"a", 1, 17, 1000)));

  std::deque<char> zeros(5);
  std::deque<char> ones(5);
  Arrays::fill(ones, static_cast<char>(0xff));
  q1 = BinaryPoint::newSetQuery(L"a",
                                std::deque<std::deque<char>>{zeros, ones});
  q2 = BinaryPoint::newSetQuery(L"a",
                                std::deque<std::deque<char>>{zeros, ones});
  TestUtil::assertEquals(q1, q2);
  TestUtil::assertEquals(q1->hashCode(), q2->hashCode());
  std::deque<char> other = ones.clone();
  other[2] = static_cast<char>(5);
  assertFalse(q1->equals(BinaryPoint::newSetQuery(
      L"a", std::deque<std::deque<char>>{zeros, other})));
}

void<missing_class_definition>::testInvalidPointLength()
{
  // C++ TODO: The following anonymous inner class could not be converted:
  //      IllegalArgumentException e =
  //      expectThrows(IllegalArgumentException.class, () => { new
  //      PointRangeQuery("field", new byte[4], new byte[8], 1)
  //    {
  //                                                  @@Override protected
  //                                                  std::wstring toString(int
  //                                                  dimension, byte[] value)
  //                                                  {
  //                                                    return "foo";
  //                                                  }
  //                                                };
}

void<missing_class_definition>::testNextUp()
{
  assertTrue(Double::compare(0, DoublePoint::nextUp(-0)) == 0);
  assertTrue(Double::compare(Double::MIN_VALUE, DoublePoint::nextUp(0)) == 0);
  assertTrue(
      Double::compare(numeric_limits<double>::infinity(),
                      DoublePoint::nextUp(numeric_limits<double>::max())) == 0);
  assertTrue(Double::compare(
                 numeric_limits<double>::infinity(),
                 DoublePoint::nextUp(numeric_limits<double>::infinity())) == 0);
  assertTrue(Double::compare(-numeric_limits<double>::max(),
                             DoublePoint::nextUp(
                                 -numeric_limits<double>::infinity())) == 0);

  assertTrue(Float::compare(0.0f, FloatPoint::nextUp(-0.0f)) == 0);
  assertTrue(Float::compare(Float::MIN_VALUE, FloatPoint::nextUp(0.0f)) == 0);
  assertTrue(Float::compare(numeric_limits<float>::infinity(),
                            FloatPoint::nextUp(numeric_limits<float>::max())) ==
             0);
  assertTrue(Float::compare(
                 numeric_limits<float>::infinity(),
                 FloatPoint::nextUp(numeric_limits<float>::infinity())) == 0);
  assertTrue(Float::compare(
                 -numeric_limits<float>::max(),
                 FloatPoint::nextUp(-numeric_limits<float>::infinity())) == 0);
}

void<missing_class_definition>::testNextDown()
{
  assertTrue(Double::compare(-0, DoublePoint::nextDown(0)) == 0);
  assertTrue(Double::compare(-Double::MIN_VALUE, DoublePoint::nextDown(-0)) ==
             0);
  assertTrue(Double::compare(
                 -numeric_limits<double>::infinity(),
                 DoublePoint::nextDown(-numeric_limits<double>::max())) == 0);
  assertTrue(Double::compare(-numeric_limits<double>::infinity(),
                             DoublePoint::nextDown(
                                 -numeric_limits<double>::infinity())) == 0);
  assertTrue(Double::compare(numeric_limits<double>::max(),
                             DoublePoint::nextDown(
                                 numeric_limits<double>::infinity())) == 0);

  assertTrue(Float::compare(-0.0f, FloatPoint::nextDown(0.0f)) == 0);
  assertTrue(Float::compare(-Float::MIN_VALUE, FloatPoint::nextDown(-0.0f)) ==
             0);
  assertTrue(
      Float::compare(-numeric_limits<float>::infinity(),
                     FloatPoint::nextDown(-numeric_limits<float>::max())) == 0);
  assertTrue(Float::compare(-numeric_limits<float>::infinity(),
                            FloatPoint::nextDown(
                                -numeric_limits<float>::infinity())) == 0);
  assertTrue(Float::compare(
                 numeric_limits<float>::max(),
                 FloatPoint::nextDown(numeric_limits<float>::infinity())) == 0);
}

void<missing_class_definition>::testInversePointRange() 
{
  shared_ptr<Directory> dir = LuceneTestCase::newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, LuceneTestCase::newIndexWriterConfig());
  constexpr int numDims = TestUtil::nextInt(LuceneTestCase::random(), 1, 3);
  constexpr int numDocs = LuceneTestCase::atLeast(
      10 *
      BKDWriter::DEFAULT_MAX_POINTS_IN_LEAF_NODE); // we need multiple leaves to
                                                   // enable this optimization
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    std::deque<int> values(numDims);
    Arrays::fill(values, i);
    doc->push_back(make_shared<IntPoint>(L"f", values));
    w->addDocument(doc);
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  delete w;

  shared_ptr<IndexSearcher> searcher = LuceneTestCase::newSearcher(r);
  std::deque<int> low(numDims);
  std::deque<int> high(numDims);
  Arrays::fill(high, numDocs - 2);
  TestUtil::assertEquals(
      high[0] - low[0] + 1,
      searcher->count(IntPoint::newRangeQuery(L"f", low, high)));
  Arrays::fill(low, 1);
  TestUtil::assertEquals(
      high[0] - low[0] + 1,
      searcher->count(IntPoint::newRangeQuery(L"f", low, high)));
  Arrays::fill(high, numDocs - 1);
  TestUtil::assertEquals(
      high[0] - low[0] + 1,
      searcher->count(IntPoint::newRangeQuery(L"f", low, high)));
  Arrays::fill(low, BKDWriter::DEFAULT_MAX_POINTS_IN_LEAF_NODE + 1);
  TestUtil::assertEquals(
      high[0] - low[0] + 1,
      searcher->count(IntPoint::newRangeQuery(L"f", low, high)));
  Arrays::fill(high, numDocs - BKDWriter::DEFAULT_MAX_POINTS_IN_LEAF_NODE);
  TestUtil::assertEquals(
      high[0] - low[0] + 1,
      searcher->count(IntPoint::newRangeQuery(L"f", low, high)));

  delete r;
  delete dir;
}
}