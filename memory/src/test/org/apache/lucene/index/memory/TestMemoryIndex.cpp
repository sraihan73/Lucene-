using namespace std;

#include "TestMemoryIndex.h"

namespace org::apache::lucene::index::memory
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockPayloadAnalyzer = org::apache::lucene::analysis::MockPayloadAnalyzer;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using BinaryPoint = org::apache::lucene::document::BinaryPoint;
using Document = org::apache::lucene::document::Document;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Term = org::apache::lucene::index::Term;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Before;
using org::junit::Test;
//    import static org.hamcrest.CoreMatchers.is;
//    import static org.hamcrest.CoreMatchers.not;
//    import static org.junit.@internal.matchers.StringContains.containsString;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void setup()
void TestMemoryIndex::setup()
{
  analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setEnableChecks(
      false); // MemoryIndex can close a TokenStream on init error
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFreezeAPI()
void TestMemoryIndex::testFreezeAPI()
{

  shared_ptr<MemoryIndex> mi = make_shared<MemoryIndex>();
  mi->addField(L"f1", L"some text", analyzer);

  assertThat(mi->search(make_shared<MatchAllDocsQuery>()), not(is(0.0f)));
  assertThat(
      mi->search(make_shared<TermQuery>(make_shared<Term>(L"f1", L"some"))),
      not(is(0.0f)));

  // check we can add a new field after searching
  mi->addField(L"f2", L"some more text", analyzer);
  assertThat(
      mi->search(make_shared<TermQuery>(make_shared<Term>(L"f2", L"some"))),
      not(is(0.0f)));

  // freeze!
  mi->freeze();

  runtime_error expected = expectThrows(runtime_error::typeid, [&]() {
    mi->addField(L"f3", L"and yet more", analyzer);
  });
  assertThat(expected.what(), containsString(L"frozen"));

  expected = expectThrows(runtime_error::typeid, [&]() {
    mi->setSimilarity(make_shared<BM25Similarity>(1, 1));
  });
  assertThat(expected.what(), containsString(L"frozen"));

  assertThat(
      mi->search(make_shared<TermQuery>(make_shared<Term>(L"f1", L"some"))),
      not(is(0.0f)));

  mi->reset();
  mi->addField(L"f1", L"wibble", analyzer);
  assertThat(
      mi->search(make_shared<TermQuery>(make_shared<Term>(L"f1", L"some"))),
      is(0.0f));
  assertThat(
      mi->search(make_shared<TermQuery>(make_shared<Term>(L"f1", L"wibble"))),
      not(is(0.0f)));

  // check we can set the Similarity again
  mi->setSimilarity(make_shared<ClassicSimilarity>());
}

void TestMemoryIndex::testSeekByTermOrd() 
{
  shared_ptr<MemoryIndex> mi = make_shared<MemoryIndex>();
  mi->addField(L"field", L"some terms be here", analyzer);
  shared_ptr<IndexSearcher> searcher = mi->createSearcher();
  shared_ptr<LeafReader> reader =
      std::static_pointer_cast<LeafReader>(searcher->getIndexReader());
  shared_ptr<TermsEnum> terms = reader->terms(L"field")->begin();
  terms->seekExact(0);
  TestUtil::assertEquals(L"be", terms->term()->utf8ToString());
  TestUtil::checkReader(reader);
}

void TestMemoryIndex::testFieldsOnlyReturnsIndexedFields() 
{
  shared_ptr<Document> doc = make_shared<Document>();

  doc->push_back(make_shared<NumericDocValuesField>(L"numeric", 29LL));
  doc->push_back(
      make_shared<TextField>(L"text", L"some text", Field::Store::NO));

  shared_ptr<MemoryIndex> mi = MemoryIndex::fromDocument(doc, analyzer);
  shared_ptr<IndexSearcher> searcher = mi->createSearcher();
  shared_ptr<IndexReader> reader = searcher->getIndexReader();

  TestUtil::assertEquals(reader->getTermVectors(0)->size(), 1);
}

void TestMemoryIndex::testReaderConsistency() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockPayloadAnalyzer>();

  // defaults
  shared_ptr<MemoryIndex> mi = make_shared<MemoryIndex>();
  mi->addField(L"field", L"some terms be here", analyzer);
  TestUtil::checkReader(mi->createSearcher()->getIndexReader());

  // all combinations of offsets/payloads options
  mi = make_shared<MemoryIndex>(true, true);
  mi->addField(L"field", L"some terms be here", analyzer);
  TestUtil::checkReader(mi->createSearcher()->getIndexReader());

  mi = make_shared<MemoryIndex>(true, false);
  mi->addField(L"field", L"some terms be here", analyzer);
  TestUtil::checkReader(mi->createSearcher()->getIndexReader());

  mi = make_shared<MemoryIndex>(false, true);
  mi->addField(L"field", L"some terms be here", analyzer);
  TestUtil::checkReader(mi->createSearcher()->getIndexReader());

  mi = make_shared<MemoryIndex>(false, false);
  mi->addField(L"field", L"some terms be here", analyzer);
  TestUtil::checkReader(mi->createSearcher()->getIndexReader());

  delete analyzer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSimilarities() throws
// java.io.IOException
void TestMemoryIndex::testSimilarities() 
{

  shared_ptr<MemoryIndex> mi = make_shared<MemoryIndex>();
  mi->addField(L"f1", L"a long text field that contains many many terms",
               analyzer);

  shared_ptr<IndexSearcher> searcher = mi->createSearcher();
  shared_ptr<LeafReader> reader =
      std::static_pointer_cast<LeafReader>(searcher->getIndexReader());
  shared_ptr<NumericDocValues> norms = reader->getNormValues(L"f1");
  TestUtil::assertEquals(0, norms->nextDoc());
  float n1 = norms->longValue();

  // Norms are re-computed when we change the Similarity
  mi->setSimilarity(
      make_shared<SimilarityAnonymousInnerClass>(shared_from_this()));
  norms = reader->getNormValues(L"f1");
  TestUtil::assertEquals(0, norms->nextDoc());
  float n2 = norms->longValue();

  assertTrue(n1 != n2);
  TestUtil::checkReader(reader);
}

TestMemoryIndex::SimilarityAnonymousInnerClass::SimilarityAnonymousInnerClass(
    shared_ptr<TestMemoryIndex> outerInstance)
{
  this->outerInstance = outerInstance;
}

int64_t TestMemoryIndex::SimilarityAnonymousInnerClass::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return 74;
}

shared_ptr<Similarity::SimWeight>
TestMemoryIndex::SimilarityAnonymousInnerClass::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Similarity::SimScorer>
TestMemoryIndex::SimilarityAnonymousInnerClass::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOmitNorms() throws java.io.IOException
void TestMemoryIndex::testOmitNorms() 
{
  shared_ptr<MemoryIndex> mi = make_shared<MemoryIndex>();
  shared_ptr<FieldType> ft = make_shared<FieldType>();
  ft->setTokenized(true);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  ft->setOmitNorms(true);
  mi->addField(make_shared<Field>(L"f1", L"some text in here", ft), analyzer);
  mi->freeze();

  shared_ptr<LeafReader> leader = std::static_pointer_cast<LeafReader>(
      mi->createSearcher()->getIndexReader());
  shared_ptr<NumericDocValues> norms = leader->getNormValues(L"f1");
  assertNull(norms);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBuildFromDocument()
void TestMemoryIndex::testBuildFromDocument()
{

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"field1", L"some text", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(L"field1", L"some more text", Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"field2", L"untokenized text",
                                          Field::Store::NO));

  analyzer->setPositionIncrementGap(100);

  shared_ptr<MemoryIndex> mi = MemoryIndex::fromDocument(doc, analyzer);

  assertThat(
      mi->search(make_shared<TermQuery>(make_shared<Term>(L"field1", L"text"))),
      not(0.0f));
  assertThat(
      mi->search(make_shared<TermQuery>(make_shared<Term>(L"field2", L"text"))),
      is(0.0f));
  assertThat(mi->search(make_shared<TermQuery>(
                 make_shared<Term>(L"field2", L"untokenized text"))),
             not(0.0f));

  assertThat(mi->search(make_shared<PhraseQuery>(L"field1", L"some", L"more",
                                                 L"text")),
             not(0.0f));
  assertThat(mi->search(make_shared<PhraseQuery>(L"field1", L"some", L"text")),
             not(0.0f));
  assertThat(mi->search(make_shared<PhraseQuery>(L"field1", L"text", L"some")),
             is(0.0f));
}

void TestMemoryIndex::testDocValues() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"numeric", 29LL));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 33LL));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 32LL));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 32LL));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 31LL));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 30LL));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"binary", make_shared<BytesRef>(L"a")));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"sorted", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sorted_set", make_shared<BytesRef>(L"f")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sorted_set", make_shared<BytesRef>(L"d")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sorted_set", make_shared<BytesRef>(L"d")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sorted_set", make_shared<BytesRef>(L"c")));

  shared_ptr<MemoryIndex> mi = MemoryIndex::fromDocument(doc, analyzer);
  shared_ptr<LeafReader> leafReader =
      mi->createSearcher()->getIndexReader()->leaves()[0]->reader();
  shared_ptr<NumericDocValues> numericDocValues =
      leafReader->getNumericDocValues(L"numeric");
  TestUtil::assertEquals(0, numericDocValues->nextDoc());
  TestUtil::assertEquals(29LL, numericDocValues->longValue());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         numericDocValues->nextDoc());
  shared_ptr<SortedNumericDocValues> sortedNumericDocValues =
      leafReader->getSortedNumericDocValues(L"sorted_numeric");
  TestUtil::assertEquals(0, sortedNumericDocValues->nextDoc());
  TestUtil::assertEquals(5, sortedNumericDocValues->docValueCount());
  TestUtil::assertEquals(30LL, sortedNumericDocValues->nextValue());
  TestUtil::assertEquals(31LL, sortedNumericDocValues->nextValue());
  TestUtil::assertEquals(32LL, sortedNumericDocValues->nextValue());
  TestUtil::assertEquals(32LL, sortedNumericDocValues->nextValue());
  TestUtil::assertEquals(33LL, sortedNumericDocValues->nextValue());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         sortedNumericDocValues->nextDoc());
  shared_ptr<BinaryDocValues> binaryDocValues =
      leafReader->getBinaryDocValues(L"binary");
  TestUtil::assertEquals(0, binaryDocValues->nextDoc());
  TestUtil::assertEquals(L"a", binaryDocValues->binaryValue()->utf8ToString());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         binaryDocValues->nextDoc());
  shared_ptr<SortedDocValues> sortedDocValues =
      leafReader->getSortedDocValues(L"sorted");
  TestUtil::assertEquals(0, sortedDocValues->nextDoc());
  TestUtil::assertEquals(L"b", sortedDocValues->binaryValue()->utf8ToString());
  TestUtil::assertEquals(0, sortedDocValues->ordValue());
  TestUtil::assertEquals(L"b", sortedDocValues->lookupOrd(0)->utf8ToString());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         sortedDocValues->nextDoc());
  shared_ptr<SortedSetDocValues> sortedSetDocValues =
      leafReader->getSortedSetDocValues(L"sorted_set");
  TestUtil::assertEquals(3, sortedSetDocValues->getValueCount());
  TestUtil::assertEquals(0, sortedSetDocValues->nextDoc());
  TestUtil::assertEquals(0LL, sortedSetDocValues->nextOrd());
  TestUtil::assertEquals(1LL, sortedSetDocValues->nextOrd());
  TestUtil::assertEquals(2LL, sortedSetDocValues->nextOrd());
  TestUtil::assertEquals(SortedSetDocValues::NO_MORE_ORDS,
                         sortedSetDocValues->nextOrd());
  TestUtil::assertEquals(L"c",
                         sortedSetDocValues->lookupOrd(0LL)->utf8ToString());
  TestUtil::assertEquals(L"d",
                         sortedSetDocValues->lookupOrd(1LL)->utf8ToString());
  TestUtil::assertEquals(L"f",
                         sortedSetDocValues->lookupOrd(2LL)->utf8ToString());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                         sortedDocValues->nextDoc());
}

void TestMemoryIndex::testDocValues_resetIterator() 
{
  shared_ptr<Document> doc = make_shared<Document>();

  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sorted_set", make_shared<BytesRef>(L"f")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sorted_set", make_shared<BytesRef>(L"d")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sorted_set", make_shared<BytesRef>(L"d")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"sorted_set", make_shared<BytesRef>(L"c")));

  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 33LL));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 32LL));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 32LL));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 31LL));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"sorted_numeric", 30LL));

  shared_ptr<MemoryIndex> mi = MemoryIndex::fromDocument(doc, analyzer);
  shared_ptr<LeafReader> leafReader =
      mi->createSearcher()->getIndexReader()->leaves()[0]->reader();

  shared_ptr<SortedSetDocValues> sortedSetDocValues =
      leafReader->getSortedSetDocValues(L"sorted_set");
  TestUtil::assertEquals(3, sortedSetDocValues->getValueCount());
  for (int times = 0; times < 3; times++) {
    assertTrue(sortedSetDocValues->advanceExact(0));
    TestUtil::assertEquals(0LL, sortedSetDocValues->nextOrd());
    TestUtil::assertEquals(1LL, sortedSetDocValues->nextOrd());
    TestUtil::assertEquals(2LL, sortedSetDocValues->nextOrd());
    TestUtil::assertEquals(SortedSetDocValues::NO_MORE_ORDS,
                           sortedSetDocValues->nextOrd());
  }

  shared_ptr<SortedNumericDocValues> sortedNumericDocValues =
      leafReader->getSortedNumericDocValues(L"sorted_numeric");
  for (int times = 0; times < 3; times++) {
    assertTrue(sortedNumericDocValues->advanceExact(0));
    TestUtil::assertEquals(5, sortedNumericDocValues->docValueCount());
    TestUtil::assertEquals(30LL, sortedNumericDocValues->nextValue());
    TestUtil::assertEquals(31LL, sortedNumericDocValues->nextValue());
    TestUtil::assertEquals(32LL, sortedNumericDocValues->nextValue());
    TestUtil::assertEquals(32LL, sortedNumericDocValues->nextValue());
    TestUtil::assertEquals(33LL, sortedNumericDocValues->nextValue());
  }
}

void TestMemoryIndex::testInvalidDocValuesUsage() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"field", 29LL));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"field", make_shared<BytesRef>(L"30")));
  try {
    MemoryIndex::fromDocument(doc, analyzer);
  } catch (const invalid_argument &e) {
    TestUtil::assertEquals(L"cannot change DocValues type from NUMERIC to "
                           L"BINARY for field \"field\"",
                           e.what());
  }

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"field", 29LL));
  doc->push_back(make_shared<NumericDocValuesField>(L"field", 30LL));
  try {
    MemoryIndex::fromDocument(doc, analyzer);
  } catch (const invalid_argument &e) {
    TestUtil::assertEquals(L"Only one value per field allowed for [NUMERIC] "
                           L"doc values field [field]",
                           e.what());
  }

  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"field", L"a b", Field::Store::NO));
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"field", make_shared<BytesRef>(L"a")));
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"field", make_shared<BytesRef>(L"b")));
  try {
    MemoryIndex::fromDocument(doc, analyzer);
  } catch (const invalid_argument &e) {
    TestUtil::assertEquals(L"Only one value per field allowed for [BINARY] doc "
                           L"values field [field]",
                           e.what());
  }

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedDocValuesField>(L"field", make_shared<BytesRef>(L"a")));
  doc->push_back(
      make_shared<SortedDocValuesField>(L"field", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<TextField>(L"field", L"a b", Field::Store::NO));
  try {
    MemoryIndex::fromDocument(doc, analyzer);
  } catch (const invalid_argument &e) {
    TestUtil::assertEquals(L"Only one value per field allowed for [SORTED] doc "
                           L"values field [field]",
                           e.what());
  }
}

void TestMemoryIndex::testDocValuesDoNotAffectBoostPositionsOrOffset() throw(
    runtime_error)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"text", make_shared<BytesRef>(L"quick brown fox")));
  doc->push_back(
      make_shared<TextField>(L"text", L"quick brown fox", Field::Store::NO));
  shared_ptr<MemoryIndex> mi =
      MemoryIndex::fromDocument(doc, analyzer, true, true);
  shared_ptr<LeafReader> leafReader =
      mi->createSearcher()->getIndexReader()->leaves()[0]->reader();
  shared_ptr<TermsEnum> tenum = leafReader->terms(L"text")->begin();

  TestUtil::assertEquals(L"brown", tenum->next().utf8ToString());
  shared_ptr<PostingsEnum> penum =
      tenum->postings(nullptr, PostingsEnum::OFFSETS);
  TestUtil::assertEquals(0, penum->nextDoc());
  TestUtil::assertEquals(1, penum->freq());
  TestUtil::assertEquals(1, penum->nextPosition());
  TestUtil::assertEquals(6, penum->startOffset());
  TestUtil::assertEquals(11, penum->endOffset());

  TestUtil::assertEquals(L"fox", tenum->next().utf8ToString());
  penum = tenum->postings(penum, PostingsEnum::OFFSETS);
  TestUtil::assertEquals(0, penum->nextDoc());
  TestUtil::assertEquals(1, penum->freq());
  TestUtil::assertEquals(2, penum->nextPosition());
  TestUtil::assertEquals(12, penum->startOffset());
  TestUtil::assertEquals(15, penum->endOffset());

  TestUtil::assertEquals(L"quick", tenum->next().utf8ToString());
  penum = tenum->postings(penum, PostingsEnum::OFFSETS);
  TestUtil::assertEquals(0, penum->nextDoc());
  TestUtil::assertEquals(1, penum->freq());
  TestUtil::assertEquals(0, penum->nextPosition());
  TestUtil::assertEquals(0, penum->startOffset());
  TestUtil::assertEquals(5, penum->endOffset());

  shared_ptr<BinaryDocValues> binaryDocValues =
      leafReader->getBinaryDocValues(L"text");
  TestUtil::assertEquals(0, binaryDocValues->nextDoc());
  TestUtil::assertEquals(L"quick brown fox",
                         binaryDocValues->binaryValue()->utf8ToString());
}

void TestMemoryIndex::testPointValues() 
{
  deque<function<IndexableField *(int64_t)>> fieldFunctions = Arrays::asList(
      [&](t) {
        make_shared<org::apache::lucene::document::IntPoint>(L"number",
                                                             t::intValue());
      },
      [&](t) {
        make_shared<org::apache::lucene::document::LongPoint>(L"number", t);
      },
      [&](t) {
        make_shared<org::apache::lucene::document::FloatPoint>(L"number",
                                                               t::floatValue());
      },
      [&](t) {
        make_shared<org::apache::lucene::document::DoublePoint>(
            L"number", t::doubleValue());
      };);
  deque<function<Query *(int64_t)>> exactQueryFunctions = Arrays::asList(
      [&](t) {
        org::apache::lucene::document::IntPoint::newExactQuery(L"number",
                                                               t::intValue());
      },
      [&](t) {
        org::apache::lucene::document::LongPoint::newExactQuery(L"number", t);
      },
      [&](t) {
        org::apache::lucene::document::FloatPoint::newExactQuery(
            L"number", t::floatValue());
      },
      [&](t) {
        org::apache::lucene::document::DoublePoint::newExactQuery(
            L"number", t::doubleValue());
      });
  deque<function<Query *(int64_t[] *)>> setQueryFunctions = Arrays::asList(
      [&](t) {
        org::apache::lucene::document::IntPoint::newSetQuery(
            L"number",
            java::util::stream::LongStream::of(t)
                .mapToInt([&](any value) { static_cast<int>(value); })
                .toArray());
      },
      [&](t) {
        org::apache::lucene::document::LongPoint::newSetQuery(L"number", t);
      },
      [&](t) {
        org::apache::lucene::document::FloatPoint::newSetQuery(
            L"number",
            java::util::Arrays::asList(
                java::util::stream::LongStream::of(t)
                    .mapToObj([&](any value) { static_cast<float>(value); })
                    .toArray(optional<float>[] ::new)));
      },
      [&](t) {
        org::apache::lucene::document::DoublePoint::newSetQuery(
            L"number",
            java::util::stream::LongStream::of(t)
                .mapToDouble([&](any value) { static_cast<double>(value); })
                .toArray());
      });
  deque<function<Query *(int64_t, int64_t)>> rangeQueryFunctions =
      Arrays::asList(
          [&](t, u) {
            org::apache::lucene::document::IntPoint::newRangeQuery(
                L"number", t::intValue(), u::intValue());
          },
          [&](t, u) {
            org::apache::lucene::document::LongPoint::newRangeQuery(L"number",
                                                                    t, u);
          },
          [&](t, u) {
            org::apache::lucene::document::FloatPoint::newRangeQuery(
                L"number", t::floatValue(), u::floatValue());
          },
          [&](t, u) {
            org::apache::lucene::document::DoublePoint::newRangeQuery(
                L"number", t::doubleValue(), u::doubleValue());
          });

  for (int i = 0; i < fieldFunctions.size(); i++) {
    function<IndexableField *(int64_t)> fieldFunction = fieldFunctions[i];
    function<Query *(int64_t)> exactQueryFunction = exactQueryFunctions[i];
    function<Query *(int64_t[] *)> setQueryFunction = setQueryFunctions[i];
    function<Query *(int64_t, int64_t)> rangeQueryFunction =
        rangeQueryFunctions[i];

    shared_ptr<Document> doc = make_shared<Document>();
    for (int number = 1; number < 32; number += 2) {
      doc->push_back(fieldFunction(static_cast<int64_t>(number)));
    }
    shared_ptr<MemoryIndex> mi = MemoryIndex::fromDocument(doc, analyzer);
    shared_ptr<IndexSearcher> indexSearcher = mi->createSearcher();
    shared_ptr<Query> query = exactQueryFunction(5LL);
    TestUtil::assertEquals(1, indexSearcher->count(query));
    query = exactQueryFunction(4LL);
    TestUtil::assertEquals(0, indexSearcher->count(query));

    query = setQueryFunction(std::deque<int64_t>{3LL, 9LL, 19LL});
    TestUtil::assertEquals(1, indexSearcher->count(query));
    query = setQueryFunction(std::deque<int64_t>{2LL, 8LL, 13LL});
    TestUtil::assertEquals(1, indexSearcher->count(query));
    query = setQueryFunction(std::deque<int64_t>{2LL, 8LL, 16LL});
    TestUtil::assertEquals(0, indexSearcher->count(query));

    query = rangeQueryFunction(2LL, 16LL);
    TestUtil::assertEquals(1, indexSearcher->count(query));
    query = rangeQueryFunction(24LL, 48LL);
    TestUtil::assertEquals(1, indexSearcher->count(query));
    query = rangeQueryFunction(48LL, 68LL);
    TestUtil::assertEquals(0, indexSearcher->count(query));
  }
}

void TestMemoryIndex::testMissingPoints() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StoredField>(L"field", 42));
  shared_ptr<MemoryIndex> mi = MemoryIndex::fromDocument(doc, analyzer);
  shared_ptr<IndexSearcher> indexSearcher = mi->createSearcher();
  // field that exists but does not have points
  assertNull(
      indexSearcher->getIndexReader()->leaves()[0]->reader().getPointValues(
          L"field"));
  // field that does not exist
  assertNull(
      indexSearcher->getIndexReader()->leaves()[0]->reader().getPointValues(
          L"some_missing_field"));
}

void TestMemoryIndex::testPointValuesDoNotAffectPositionsOrOffset() throw(
    runtime_error)
{
  shared_ptr<MemoryIndex> mi = make_shared<MemoryIndex>(true, true);
  mi->addField(
      make_shared<TextField>(L"text", L"quick brown fox", Field::Store::NO),
      analyzer);
  mi->addField(
      make_shared<BinaryPoint>(
          L"text", (wstring(L"quick")).getBytes(StandardCharsets::UTF_8)),
      analyzer);
  mi->addField(
      make_shared<BinaryPoint>(
          L"text", (wstring(L"brown")).getBytes(StandardCharsets::UTF_8)),
      analyzer);
  shared_ptr<LeafReader> leafReader =
      mi->createSearcher()->getIndexReader()->leaves()[0]->reader();
  shared_ptr<TermsEnum> tenum = leafReader->terms(L"text")->begin();

  TestUtil::assertEquals(L"brown", tenum->next().utf8ToString());
  shared_ptr<PostingsEnum> penum =
      tenum->postings(nullptr, PostingsEnum::OFFSETS);
  TestUtil::assertEquals(0, penum->nextDoc());
  TestUtil::assertEquals(1, penum->freq());
  TestUtil::assertEquals(1, penum->nextPosition());
  TestUtil::assertEquals(6, penum->startOffset());
  TestUtil::assertEquals(11, penum->endOffset());

  TestUtil::assertEquals(L"fox", tenum->next().utf8ToString());
  penum = tenum->postings(penum, PostingsEnum::OFFSETS);
  TestUtil::assertEquals(0, penum->nextDoc());
  TestUtil::assertEquals(1, penum->freq());
  TestUtil::assertEquals(2, penum->nextPosition());
  TestUtil::assertEquals(12, penum->startOffset());
  TestUtil::assertEquals(15, penum->endOffset());

  TestUtil::assertEquals(L"quick", tenum->next().utf8ToString());
  penum = tenum->postings(penum, PostingsEnum::OFFSETS);
  TestUtil::assertEquals(0, penum->nextDoc());
  TestUtil::assertEquals(1, penum->freq());
  TestUtil::assertEquals(0, penum->nextPosition());
  TestUtil::assertEquals(0, penum->startOffset());
  TestUtil::assertEquals(5, penum->endOffset());

  shared_ptr<IndexSearcher> indexSearcher = mi->createSearcher();
  TestUtil::assertEquals(
      1, indexSearcher->count(BinaryPoint::newExactQuery(
             L"text", (wstring(L"quick")).getBytes(StandardCharsets::UTF_8))));
  TestUtil::assertEquals(
      1, indexSearcher->count(BinaryPoint::newExactQuery(
             L"text", (wstring(L"brown")).getBytes(StandardCharsets::UTF_8))));
  TestUtil::assertEquals(
      0, indexSearcher->count(BinaryPoint::newExactQuery(
             L"text", (wstring(L"jumps")).getBytes(StandardCharsets::UTF_8))));
}

void TestMemoryIndex::test2DPoints() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"ints", 0, -100));
  doc->push_back(make_shared<IntPoint>(L"ints", 20, 20));
  doc->push_back(make_shared<IntPoint>(L"ints", 100, -100));
  doc->push_back(make_shared<LongPoint>(L"longs", 0LL, -100LL));
  doc->push_back(make_shared<LongPoint>(L"longs", 20LL, 20LL));
  doc->push_back(make_shared<LongPoint>(L"longs", 100LL, -100LL));
  doc->push_back(make_shared<FloatPoint>(L"floats", 0.0F, -100.0F));
  doc->push_back(make_shared<FloatPoint>(L"floats", 20.0F, 20.0F));
  doc->push_back(make_shared<FloatPoint>(L"floats", 100.0F, -100.0F));
  doc->push_back(make_shared<DoublePoint>(L"doubles", 0, -100));
  doc->push_back(make_shared<DoublePoint>(L"doubles", 20, 20));
  doc->push_back(make_shared<DoublePoint>(L"doubles", 100, -100));

  shared_ptr<MemoryIndex> mi = MemoryIndex::fromDocument(doc, analyzer);
  shared_ptr<IndexSearcher> s = mi->createSearcher();

  TestUtil::assertEquals(
      1, s->count(IntPoint::newRangeQuery(L"ints", std::deque<int>{10, 10},
                                          std::deque<int>{30, 30})));
  TestUtil::assertEquals(1, s->count(LongPoint::newRangeQuery(
                                L"longs", std::deque<int64_t>{10LL, 10LL},
                                std::deque<int64_t>{30LL, 30LL})));
  TestUtil::assertEquals(1, s->count(FloatPoint::newRangeQuery(
                                L"floats", std::deque<float>{10.0F, 10.0F},
                                std::deque<float>{30.0F, 30.0F})));
  TestUtil::assertEquals(1, s->count(DoublePoint::newRangeQuery(
                                L"doubles", std::deque<double>{10, 10},
                                std::deque<double>{30, 30})));
}

void TestMemoryIndex::testMultiValuedPointsSortedCorrectly() throw(
    runtime_error)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<IntPoint>(L"ints", 3));
  doc->push_back(make_shared<IntPoint>(L"ints", 2));
  doc->push_back(make_shared<IntPoint>(L"ints", 1));
  doc->push_back(make_shared<LongPoint>(L"longs", 3LL));
  doc->push_back(make_shared<LongPoint>(L"longs", 2LL));
  doc->push_back(make_shared<LongPoint>(L"longs", 1LL));
  doc->push_back(make_shared<FloatPoint>(L"floats", 3.0F));
  doc->push_back(make_shared<FloatPoint>(L"floats", 2.0F));
  doc->push_back(make_shared<FloatPoint>(L"floats", 1.0F));
  doc->push_back(make_shared<DoublePoint>(L"doubles", 3));
  doc->push_back(make_shared<DoublePoint>(L"doubles", 2));
  doc->push_back(make_shared<DoublePoint>(L"doubles", 1));

  shared_ptr<MemoryIndex> mi = MemoryIndex::fromDocument(doc, analyzer);
  shared_ptr<IndexSearcher> s = mi->createSearcher();

  TestUtil::assertEquals(1, s->count(IntPoint::newSetQuery(L"ints", {2})));
  TestUtil::assertEquals(1, s->count(LongPoint::newSetQuery(L"longs", 2)));
  TestUtil::assertEquals(1, s->count(FloatPoint::newSetQuery(L"floats", 2)));
  TestUtil::assertEquals(1, s->count(DoublePoint::newSetQuery(L"doubles", 2)));
}

void TestMemoryIndex::testIndexingPointsAndDocValues() 
{
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(1, 4);
  type->setDocValuesType(DocValuesType::BINARY);
  type->freeze();
  shared_ptr<Document> doc = make_shared<Document>();
  std::deque<char> packedPoint =
      (wstring(L"term")).getBytes(StandardCharsets::UTF_8);
  doc->push_back(make_shared<BinaryPoint>(L"field", packedPoint, type));
  shared_ptr<MemoryIndex> mi = MemoryIndex::fromDocument(doc, analyzer);
  shared_ptr<LeafReader> leafReader =
      mi->createSearcher()->getIndexReader()->leaves()[0]->reader();

  TestUtil::assertEquals(1, leafReader->getPointValues(L"field")->size());
  assertArrayEquals(packedPoint,
                    leafReader->getPointValues(L"field")->getMinPackedValue());
  assertArrayEquals(packedPoint,
                    leafReader->getPointValues(L"field")->getMaxPackedValue());

  shared_ptr<BinaryDocValues> dvs = leafReader->getBinaryDocValues(L"field");
  TestUtil::assertEquals(0, dvs->nextDoc());
  TestUtil::assertEquals(L"term", dvs->binaryValue()->utf8ToString());
}

void TestMemoryIndex::testToStringDebug()
{
  shared_ptr<MemoryIndex> mi = make_shared<MemoryIndex>(true, true);
  shared_ptr<Analyzer> analyzer = make_shared<MockPayloadAnalyzer>();

  mi->addField(L"analyzedField", L"aa bb aa", analyzer);

  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(1, 4);
  type->setDocValuesType(DocValuesType::BINARY);
  type->freeze();
  mi->addField(make_shared<BinaryPoint>(
                   L"pointAndDvField",
                   (wstring(L"term")).getBytes(StandardCharsets::UTF_8), type),
               analyzer);

  TestUtil::assertEquals(
      wstring(L"analyzedField:\n") +
          L"\t'[61 61]':2: [(0, 0, 2, [70 6f 73 3a 20 30]), (1, 6, 8, [70 6f "
          L"73 3a 20 32])]\n" +
          L"\t'[62 62]':1: [(1, 3, 5, [70 6f 73 3a 20 31])]\n" +
          L"\tterms=2, positions=3\n" + L"pointAndDvField:\n" +
          L"\tterms=0, positions=0\n" + L"\n" +
          L"fields=2, terms=2, positions=3",
      mi->toStringDebug());
}
} // namespace org::apache::lucene::index::memory