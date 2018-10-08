using namespace std;

#include "BaseNormsFormatTestCase.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

bool BaseNormsFormatTestCase::codecSupportsSparsity() { return true; }

void BaseNormsFormatTestCase::testByteRange() 
{
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(1, [&]() {
      return TestUtil::nextLong(r, numeric_limits<char>::min(),
                                numeric_limits<char>::max());
    });
  }
}

void BaseNormsFormatTestCase::testSparseByteRange() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(random()->nextDouble(), [&]() {
      return TestUtil::nextLong(r, numeric_limits<char>::min(),
                                numeric_limits<char>::max());
    });
  }
}

void BaseNormsFormatTestCase::testShortRange() 
{
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(1, [&]() {
      return TestUtil::nextLong(r, numeric_limits<short>::min(),
                                numeric_limits<short>::max());
    });
  }
}

void BaseNormsFormatTestCase::testSparseShortRange() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(random()->nextDouble(), [&]() {
      return TestUtil::nextLong(r, numeric_limits<short>::min(),
                                numeric_limits<short>::max());
    });
  }
}

void BaseNormsFormatTestCase::testLongRange() 
{
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(1, [&]() {
      return TestUtil::nextLong(r, numeric_limits<int64_t>::min(),
                                numeric_limits<int64_t>::max());
    });
  }
}

void BaseNormsFormatTestCase::testSparseLongRange() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(random()->nextDouble(), [&]() {
      return TestUtil::nextLong(r, numeric_limits<int64_t>::min(),
                                numeric_limits<int64_t>::max());
    });
  }
}

void BaseNormsFormatTestCase::testFullLongRange() 
{
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(1, [&]() {
      int thingToDo = r->nextInt(3);
      switch (thingToDo) {
      case 0:
        return numeric_limits<int64_t>::min();
      case 1:
        return numeric_limits<int64_t>::max();
      default:
        return TestUtil::nextLong(r, numeric_limits<int64_t>::min(),
                                  numeric_limits<int64_t>::max());
      }
    });
  }
}

void BaseNormsFormatTestCase::testSparseFullLongRange() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(random()->nextDouble(), [&]() {
      int thingToDo = r->nextInt(3);
      switch (thingToDo) {
      case 0:
        return numeric_limits<int64_t>::min();
      case 1:
        return numeric_limits<int64_t>::max();
      default:
        return TestUtil::nextLong(r, numeric_limits<int64_t>::min(),
                                  numeric_limits<int64_t>::max());
      }
    });
  }
}

void BaseNormsFormatTestCase::testFewValues() 
{
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(1, [&]() { return r->nextBoolean() ? 20 : 3; });
  }
}

void BaseNormsFormatTestCase::testFewSparseValues() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(random()->nextDouble(),
                               [&]() { return r->nextBoolean() ? 20 : 3; });
  }
}

void BaseNormsFormatTestCase::testFewLargeValues() 
{
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(
        1, [&]() { return r->nextBoolean() ? 1000000LL : -5000; });
  }
}

void BaseNormsFormatTestCase::testFewSparseLargeValues() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(random()->nextDouble(), [&]() {
      return r->nextBoolean() ? 1000000LL : -5000;
    });
  }
}

void BaseNormsFormatTestCase::testAllZeros() 
{
  int iterations = atLeast(1);
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(1, [&]() { return 0; });
  }
}

void BaseNormsFormatTestCase::testSparseAllZeros() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  int iterations = atLeast(1);
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(random()->nextDouble(), [&]() { return 0; });
  }
}

void BaseNormsFormatTestCase::testMostZeros() 
{
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    doTestNormsVersusDocValues(1, [&]() {
      return r->nextInt(100) == 0
                 ? TestUtil::nextLong(r, numeric_limits<char>::min(),
                                      numeric_limits<char>::max())
                 : 0;
    });
  }
}

void BaseNormsFormatTestCase::testOutliers() 
{
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    constexpr int64_t commonValue = TestUtil::nextLong(
        r, numeric_limits<char>::min(), numeric_limits<char>::max());
    doTestNormsVersusDocValues(1, [&]() {
      return r->nextInt(100) == 0
                 ? TestUtil::nextLong(r, numeric_limits<char>::min(),
                                      numeric_limits<char>::max())
                 : commonValue;
    });
  }
}

void BaseNormsFormatTestCase::testSparseOutliers() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    constexpr int64_t commonValue = TestUtil::nextLong(
        r, numeric_limits<char>::min(), numeric_limits<char>::max());
    doTestNormsVersusDocValues(random()->nextDouble(), [&]() {
      return r->nextInt(100) == 0
                 ? TestUtil::nextLong(r, numeric_limits<char>::min(),
                                      numeric_limits<char>::max())
                 : commonValue;
    });
  }
}

void BaseNormsFormatTestCase::testOutliers2() 
{
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    constexpr int64_t commonValue = TestUtil::nextLong(
        r, numeric_limits<char>::min(), numeric_limits<char>::max());
    constexpr int64_t uncommonValue = TestUtil::nextLong(
        r, numeric_limits<char>::min(), numeric_limits<char>::max());
    doTestNormsVersusDocValues(1, [&]() {
      return r->nextInt(100) == 0 ? uncommonValue : commonValue;
    });
  }
}

void BaseNormsFormatTestCase::testSparseOutliers2() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; i++) {
    constexpr int64_t commonValue = TestUtil::nextLong(
        r, numeric_limits<char>::min(), numeric_limits<char>::max());
    constexpr int64_t uncommonValue = TestUtil::nextLong(
        r, numeric_limits<char>::min(), numeric_limits<char>::max());
    doTestNormsVersusDocValues(random()->nextDouble(), [&]() {
      return r->nextInt(100) == 0 ? uncommonValue : commonValue;
    });
  }
}

void BaseNormsFormatTestCase::testNCommon() 
{
  shared_ptr<Random> *const r = random();
  constexpr int N = TestUtil::nextInt(r, 2, 15);
  const std::deque<int64_t> commonValues = std::deque<int64_t>(N);
  for (int j = 0; j < N; ++j) {
    commonValues[j] = TestUtil::nextLong(r, numeric_limits<char>::min(),
                                         numeric_limits<char>::max());
  }
  constexpr int numOtherValues = TestUtil::nextInt(r, 2, 256 - N);
  const std::deque<int64_t> otherValues =
      std::deque<int64_t>(numOtherValues);
  for (int j = 0; j < numOtherValues; ++j) {
    otherValues[j] = TestUtil::nextLong(r, numeric_limits<char>::min(),
                                        numeric_limits<char>::max());
  }
  doTestNormsVersusDocValues(1, [&]() {
    return r->nextInt(100) == 0 ? otherValues[r->nextInt(numOtherValues - 1)]
                                : commonValues[r->nextInt(N - 1)];
  });
}

void BaseNormsFormatTestCase::testSparseNCommon() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  shared_ptr<Random> *const r = random();
  constexpr int N = TestUtil::nextInt(r, 2, 15);
  const std::deque<int64_t> commonValues = std::deque<int64_t>(N);
  for (int j = 0; j < N; ++j) {
    commonValues[j] = TestUtil::nextLong(r, numeric_limits<char>::min(),
                                         numeric_limits<char>::max());
  }
  constexpr int numOtherValues = TestUtil::nextInt(r, 2, 256 - N);
  const std::deque<int64_t> otherValues =
      std::deque<int64_t>(numOtherValues);
  for (int j = 0; j < numOtherValues; ++j) {
    otherValues[j] = TestUtil::nextLong(r, numeric_limits<char>::min(),
                                        numeric_limits<char>::max());
  }
  doTestNormsVersusDocValues(random()->nextDouble(), [&]() {
    return r->nextInt(100) == 0 ? otherValues[r->nextInt(numOtherValues - 1)]
                                : commonValues[r->nextInt(N - 1)];
  });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testNCommonBig() throws Exception
void BaseNormsFormatTestCase::testNCommonBig() 
{
  constexpr int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; ++i) {
    // 16 is 4 bpv, the max before we jump to 8bpv
    for (int n = 2; n < 16; ++n) {
      constexpr int N = n;
      const std::deque<int64_t> commonValues = std::deque<int64_t>(N);
      for (int j = 0; j < N; ++j) {
        commonValues[j] = TestUtil::nextLong(r, numeric_limits<char>::min(),
                                             numeric_limits<char>::max());
      }
      constexpr int numOtherValues = TestUtil::nextInt(r, 2, 256 - N);
      const std::deque<int64_t> otherValues =
          std::deque<int64_t>(numOtherValues);
      for (int j = 0; j < numOtherValues; ++j) {
        otherValues[j] = TestUtil::nextLong(r, numeric_limits<char>::min(),
                                            numeric_limits<char>::max());
      }
      doTestNormsVersusDocValues(1, [&]() {
        return r->nextInt(100) == 0
                   ? otherValues[r->nextInt(numOtherValues - 1)]
                   : commonValues[r->nextInt(N - 1)];
      });
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testSparseNCommonBig() throws Exception
void BaseNormsFormatTestCase::testSparseNCommonBig() 
{
  assumeTrue(L"Requires sparse norms support", codecSupportsSparsity());
  constexpr int iterations = atLeast(1);
  shared_ptr<Random> *const r = random();
  for (int i = 0; i < iterations; ++i) {
    // 16 is 4 bpv, the max before we jump to 8bpv
    for (int n = 2; n < 16; ++n) {
      constexpr int N = n;
      const std::deque<int64_t> commonValues = std::deque<int64_t>(N);
      for (int j = 0; j < N; ++j) {
        commonValues[j] = TestUtil::nextLong(r, numeric_limits<char>::min(),
                                             numeric_limits<char>::max());
      }
      constexpr int numOtherValues = TestUtil::nextInt(r, 2, 256 - N);
      const std::deque<int64_t> otherValues =
          std::deque<int64_t>(numOtherValues);
      for (int j = 0; j < numOtherValues; ++j) {
        otherValues[j] = TestUtil::nextLong(r, numeric_limits<char>::min(),
                                            numeric_limits<char>::max());
      }
      doTestNormsVersusDocValues(random()->nextDouble(), [&]() {
        return r->nextInt(100) == 0
                   ? otherValues[r->nextInt(numOtherValues - 1)]
                   : commonValues[r->nextInt(N - 1)];
      });
    }
  }
}

void BaseNormsFormatTestCase::doTestNormsVersusDocValues(
    double density, function<int64_t()> &longs) 
{
  int numDocs = atLeast(500);
  shared_ptr<FixedBitSet> *const docsWithField =
      make_shared<FixedBitSet>(numDocs);
  constexpr int numDocsWithField = max(1, static_cast<int>(density * numDocs));
  if (numDocsWithField == numDocs) {
    docsWithField->set(0, numDocs);
  } else {
    int i = 0;
    while (i < numDocsWithField) {
      int doc = random()->nextInt(numDocs);
      if (docsWithField->get(doc) == false) {
        docsWithField->set(doc);
        ++i;
      }
    }
  }
  std::deque<int64_t> norms(numDocsWithField);
  for (int i = 0; i < numDocsWithField; i++) {
    norms[i] = longs();
  }

  shared_ptr<Directory> dir = applyCreatedVersionMajor(newDirectory());
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(NoMergePolicy::INSTANCE);
  conf->setSimilarity(make_shared<CannedNormSimilarity>(norms));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField =
      make_shared<StringField>(L"id", L"", Field::Store::NO);
  shared_ptr<Field> indexedField =
      make_shared<TextField>(L"indexed", L"", Field::Store::NO);
  shared_ptr<Field> dvField = make_shared<NumericDocValuesField>(L"dv", 0);
  doc->push_back(idField);
  doc->push_back(indexedField);
  doc->push_back(dvField);

  for (int i = 0, j = 0; i < numDocs; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    idField->setStringValue(Integer::toString(i));
    if (docsWithField->get(i) == false) {
      shared_ptr<Document> doc2 = make_shared<Document>();
      doc2->push_back(idField);
      writer->addDocument(doc2);
    } else {
      int64_t value = norms[j++];
      dvField->setLongValue(value);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      indexedField->setStringValue(Long::toString(value));
      writer->addDocument(doc);
    }
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  // delete some docs
  int numDeletions = random()->nextInt(numDocs / 20);
  for (int i = 0; i < numDeletions; i++) {
    int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }

  writer->commit();

  // compare
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  checkNormsVsDocValues(ir);
  ir->close();

  writer->forceMerge(1);

  // compare again
  ir = DirectoryReader::open(dir);
  checkNormsVsDocValues(ir);

  delete writer;
  ir->close();
  delete dir;
}

void BaseNormsFormatTestCase::checkNormsVsDocValues(
    shared_ptr<IndexReader> ir) 
{
  for (auto context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<NumericDocValues> expected = r->getNumericDocValues(L"dv");
    shared_ptr<NumericDocValues> actual = r->getNormValues(L"indexed");
    TestUtil::assertEquals(expected == nullptr, actual == nullptr);
    if (expected != nullptr) {
      for (int d = expected->nextDoc(); d != DocIdSetIterator::NO_MORE_DOCS;
           d = expected->nextDoc()) {
        TestUtil::assertEquals(d, actual->nextDoc());
        assertEquals(L"doc " + to_wstring(d), expected->longValue(),
                     actual->longValue());
      }
      TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, actual->nextDoc());
    }
  }
}

BaseNormsFormatTestCase::CannedNormSimilarity::CannedNormSimilarity(
    std::deque<int64_t> &norms)
    : norms(norms)
{
}

int64_t BaseNormsFormatTestCase::CannedNormSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return norms[index++];
}

shared_ptr<Similarity::SimWeight>
BaseNormsFormatTestCase::CannedNormSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Similarity::SimScorer>
BaseNormsFormatTestCase::CannedNormSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}

void BaseNormsFormatTestCase::addRandomFields(shared_ptr<Document> doc)
{
  // TODO: improve
  doc->push_back(make_shared<TextField>(
      L"foobar", TestUtil::randomSimpleString(random()), Field::Store::NO));
}

void BaseNormsFormatTestCase::testMergeStability() 
{
  // TODO: can we improve this base test to just have subclasses declare the
  // extensions to check, rather than a blacklist to exclude? we need to index
  // stuff to get norms, but we dont care about testing the PFs actually doing
  // that...
  assumeTrue(
      L"The MockRandom PF randomizes content on the fly, so we can't check it",
      false);
}

void BaseNormsFormatTestCase::testUndeadNorms() 
{
  shared_ptr<Directory> dir = applyCreatedVersionMajor(newDirectory());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  int numDocs = atLeast(500);
  deque<int> toDelete = deque<int>();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<StringField>(L"id", L"" + to_wstring(i), Field::Store::NO));
    if (random()->nextInt(5) == 1) {
      toDelete.push_back(i);
      doc->push_back(make_shared<TextField>(L"content", L"some content",
                                            Field::Store::NO));
    }
    w->addDocument(doc);
  }
  for (shared_ptr<> : : optional<int> id : toDelete) {
    w->deleteDocuments(make_shared<Term>(L"id", L"" + id));
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> r = w->getReader();
  assertFalse(r->hasDeletions());

  // Confusingly, norms should exist, and should all be 0, even though we
  // deleted all docs that had the field "content".  They should not be undead:
  shared_ptr<NumericDocValues> norms =
      MultiDocValues::getNormValues(r, L"content");
  assertNotNull(norms);
  if (codecSupportsSparsity()) {
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, norms->nextDoc());
  } else {
    for (int i = 0; i < r->maxDoc(); i++) {
      TestUtil::assertEquals(i, norms->nextDoc());
      TestUtil::assertEquals(0, norms->longValue());
    }
  }

  delete r;
  delete w;
  delete dir;
}

void BaseNormsFormatTestCase::testThreads() 
{
  float density = codecSupportsSparsity() == false || random()->nextBoolean()
                      ? 1.0f
                      : random()->nextFloat();
  int numDocs = atLeast(500);
  shared_ptr<FixedBitSet> *const docsWithField =
      make_shared<FixedBitSet>(numDocs);
  constexpr int numDocsWithField = max(1, static_cast<int>(density * numDocs));
  if (numDocsWithField == numDocs) {
    docsWithField->set(0, numDocs);
  } else {
    int i = 0;
    while (i < numDocsWithField) {
      int doc = random()->nextInt(numDocs);
      if (docsWithField->get(doc) == false) {
        docsWithField->set(doc);
        ++i;
      }
    }
  }

  std::deque<int64_t> norms(numDocsWithField);
  for (int i = 0; i < numDocsWithField; i++) {
    norms[i] = random()->nextLong();
  }

  shared_ptr<Directory> dir = applyCreatedVersionMajor(newDirectory());
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  conf->setMergePolicy(NoMergePolicy::INSTANCE);
  conf->setSimilarity(make_shared<CannedNormSimilarity>(norms));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField =
      make_shared<StringField>(L"id", L"", Field::Store::NO);
  shared_ptr<Field> indexedField =
      make_shared<TextField>(L"indexed", L"", Field::Store::NO);
  shared_ptr<Field> dvField = make_shared<NumericDocValuesField>(L"dv", 0);
  doc->push_back(idField);
  doc->push_back(indexedField);
  doc->push_back(dvField);

  for (int i = 0, j = 0; i < numDocs; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    idField->setStringValue(Integer::toString(i));
    if (docsWithField->get(i) == false) {
      shared_ptr<Document> doc2 = make_shared<Document>();
      doc2->push_back(idField);
      writer->addDocument(doc2);
    } else {
      int64_t value = norms[j++];
      dvField->setLongValue(value);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      indexedField->setStringValue(Long::toString(value));
      writer->addDocument(doc);
    }
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  shared_ptr<DirectoryReader> reader = writer->getReader();
  delete writer;

  constexpr int numThreads = TestUtil::nextInt(random(), 3, 30);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  shared_ptr<CountDownLatch> *const latch = make_shared<CountDownLatch>(1);
  for (int i = 0; i < numThreads; ++i) {
    threads[i] = make_shared<Thread>([&]() {
      try {
        latch->await();
        checkNormsVsDocValues(reader);
        TestUtil::checkReader(reader);
      } catch (const runtime_error &e) {
        throw runtime_error(e);
      }
    });
  }

  for (auto thread : threads) {
    thread->start();
  }
  latch->countDown();
  for (auto thread : threads) {
    thread->join();
  }

  reader->close();
  delete dir;
}
} // namespace org::apache::lucene::index