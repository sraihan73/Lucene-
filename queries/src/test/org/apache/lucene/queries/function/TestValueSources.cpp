using namespace std;

#include "TestValueSources.h"

namespace org::apache::lucene::queries::function
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using BytesRefFieldSource =
    org::apache::lucene::queries::function::valuesource::BytesRefFieldSource;
using ConstValueSource =
    org::apache::lucene::queries::function::valuesource::ConstValueSource;
using DivFloatFunction =
    org::apache::lucene::queries::function::valuesource::DivFloatFunction;
using DocFreqValueSource =
    org::apache::lucene::queries::function::valuesource::DocFreqValueSource;
using DoubleConstValueSource =
    org::apache::lucene::queries::function::valuesource::DoubleConstValueSource;
using DoubleFieldSource =
    org::apache::lucene::queries::function::valuesource::DoubleFieldSource;
using FloatFieldSource =
    org::apache::lucene::queries::function::valuesource::FloatFieldSource;
using IDFValueSource =
    org::apache::lucene::queries::function::valuesource::IDFValueSource;
using IfFunction =
    org::apache::lucene::queries::function::valuesource::IfFunction;
using IntFieldSource =
    org::apache::lucene::queries::function::valuesource::IntFieldSource;
using JoinDocFreqValueSource =
    org::apache::lucene::queries::function::valuesource::JoinDocFreqValueSource;
using LinearFloatFunction =
    org::apache::lucene::queries::function::valuesource::LinearFloatFunction;
using LiteralValueSource =
    org::apache::lucene::queries::function::valuesource::LiteralValueSource;
using LongFieldSource =
    org::apache::lucene::queries::function::valuesource::LongFieldSource;
using MaxDocValueSource =
    org::apache::lucene::queries::function::valuesource::MaxDocValueSource;
using MaxFloatFunction =
    org::apache::lucene::queries::function::valuesource::MaxFloatFunction;
using MinFloatFunction =
    org::apache::lucene::queries::function::valuesource::MinFloatFunction;
using MultiFloatFunction =
    org::apache::lucene::queries::function::valuesource::MultiFloatFunction;
using MultiFunction =
    org::apache::lucene::queries::function::valuesource::MultiFunction;
using MultiValuedDoubleFieldSource = org::apache::lucene::queries::function::
    valuesource::MultiValuedDoubleFieldSource;
using MultiValuedFloatFieldSource = org::apache::lucene::queries::function::
    valuesource::MultiValuedFloatFieldSource;
using MultiValuedIntFieldSource = org::apache::lucene::queries::function::
    valuesource::MultiValuedIntFieldSource;
using MultiValuedLongFieldSource = org::apache::lucene::queries::function::
    valuesource::MultiValuedLongFieldSource;
using NormValueSource =
    org::apache::lucene::queries::function::valuesource::NormValueSource;
using NumDocsValueSource =
    org::apache::lucene::queries::function::valuesource::NumDocsValueSource;
using PowFloatFunction =
    org::apache::lucene::queries::function::valuesource::PowFloatFunction;
using ProductFloatFunction =
    org::apache::lucene::queries::function::valuesource::ProductFloatFunction;
using QueryValueSource =
    org::apache::lucene::queries::function::valuesource::QueryValueSource;
using RangeMapFloatFunction =
    org::apache::lucene::queries::function::valuesource::RangeMapFloatFunction;
using ReciprocalFloatFunction = org::apache::lucene::queries::function::
    valuesource::ReciprocalFloatFunction;
using ScaleFloatFunction =
    org::apache::lucene::queries::function::valuesource::ScaleFloatFunction;
using SumFloatFunction =
    org::apache::lucene::queries::function::valuesource::SumFloatFunction;
using SumTotalTermFreqValueSource = org::apache::lucene::queries::function::
    valuesource::SumTotalTermFreqValueSource;
using TFValueSource =
    org::apache::lucene::queries::function::valuesource::TFValueSource;
using TermFreqValueSource =
    org::apache::lucene::queries::function::valuesource::TermFreqValueSource;
using TotalTermFreqValueSource = org::apache::lucene::queries::function::
    valuesource::TotalTermFreqValueSource;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using Type = org::apache::lucene::search::SortedNumericSelector::Type;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::store::Directory> TestValueSources::dir;
shared_ptr<org::apache::lucene::analysis::Analyzer> TestValueSources::analyzer;
shared_ptr<org::apache::lucene::index::IndexReader> TestValueSources::reader;
shared_ptr<org::apache::lucene::search::IndexSearcher>
    TestValueSources::searcher;
const shared_ptr<ValueSource> TestValueSources::BOGUS_FLOAT_VS = make_shared<
    org::apache::lucene::queries::function::valuesource::FloatFieldSource>(
    L"bogus_field");
const shared_ptr<ValueSource> TestValueSources::BOGUS_DOUBLE_VS = make_shared<
    org::apache::lucene::queries::function::valuesource::DoubleFieldSource>(
    L"bogus_field");
const shared_ptr<ValueSource> TestValueSources::BOGUS_INT_VS = make_shared<
    org::apache::lucene::queries::function::valuesource::IntFieldSource>(
    L"bogus_field");
const shared_ptr<ValueSource> TestValueSources::BOGUS_LONG_VS = make_shared<
    org::apache::lucene::queries::function::valuesource::LongFieldSource>(
    L"bogus_field");
const deque<std::deque<wstring>> TestValueSources::documents =
    java::util::Arrays::asList(std::deque<std::deque<wstring>>{
        std::deque<wstring>{L"0", L"3.63", L"5.2", L"35", L"4343", L"test",
                             L"this is a test test test", L"2.13", L"3.69",
                             L"-0.11", L"1", L"7", L"5"},
        std::deque<wstring>{L"1", L"5.65", L"9.3", L"54", L"1954", L"bar",
                             L"second test", L"12.79", L"123.456", L"0.01",
                             L"12", L"900", L"-1"}});

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestValueSources::beforeClass() 
{
  dir = newDirectory();
  analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwConfig = newIndexWriterConfig(analyzer);
  iwConfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConfig);
  for (auto doc : documents) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(
        make_shared<StringField>(L"id", doc[0], Field::Store::NO));
    document->push_back(make_shared<SortedDocValuesField>(
        L"id", make_shared<BytesRef>(doc[0])));
    document->push_back(make_shared<NumericDocValuesField>(
        L"double", Double::doubleToRawLongBits(stod(doc[1]))));
    document->push_back(make_shared<NumericDocValuesField>(
        L"float", Float::floatToRawIntBits(stof(doc[2]))));
    document->push_back(
        make_shared<NumericDocValuesField>(L"int", stoi(doc[3])));
    document->push_back(make_shared<NumericDocValuesField>(
        L"long", StringHelper::fromString<int64_t>(doc[4])));
    document->push_back(
        make_shared<StringField>(L"string", doc[5], Field::Store::NO));
    document->push_back(make_shared<SortedDocValuesField>(
        L"string", make_shared<BytesRef>(doc[5])));
    document->push_back(
        make_shared<TextField>(L"text", doc[6], Field::Store::NO));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"floatMv", NumericUtils::floatToSortableInt(stof(doc[7]))));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"floatMv", NumericUtils::floatToSortableInt(stof(doc[8]))));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"floatMv", NumericUtils::floatToSortableInt(stof(doc[9]))));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"doubleMv", NumericUtils::doubleToSortableLong(stod(doc[7]))));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"doubleMv", NumericUtils::doubleToSortableLong(stod(doc[8]))));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"doubleMv", NumericUtils::doubleToSortableLong(stod(doc[9]))));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"intMv", StringHelper::fromString<int64_t>(doc[10])));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"intMv", StringHelper::fromString<int64_t>(doc[11])));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"intMv", StringHelper::fromString<int64_t>(doc[12])));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"longMv", StringHelper::fromString<int64_t>(doc[10])));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"longMv", StringHelper::fromString<int64_t>(doc[11])));
    document->push_back(make_shared<SortedNumericDocValuesField>(
        L"longMv", StringHelper::fromString<int64_t>(doc[12])));
    iw->addDocument(document);
  }

  reader = iw->getReader();
  searcher = newSearcher(reader);
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestValueSources::afterClass() 
{
  searcher.reset();
  delete reader;
  reader.reset();
  delete dir;
  dir.reset();
  delete analyzer;
  analyzer.reset();
}

void TestValueSources::testConst() 
{
  shared_ptr<ValueSource> vs = make_shared<ConstValueSource>(0.3f);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{0.3f, 0.3f});
  assertAllExist(vs);
}

void TestValueSources::testDiv() 
{
  shared_ptr<ValueSource> vs =
      make_shared<DivFloatFunction>(make_shared<ConstValueSource>(10.0f),
                                    make_shared<ConstValueSource>(5.0f));
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{2.0f, 2.0f});
  assertAllExist(vs);
  vs = make_shared<DivFloatFunction>(make_shared<ConstValueSource>(10.0f),
                                     BOGUS_FLOAT_VS);
  assertNoneExist(vs);
  vs = make_shared<DivFloatFunction>(BOGUS_FLOAT_VS,
                                     make_shared<ConstValueSource>(10.0f));
  assertNoneExist(vs);
}

void TestValueSources::testDocFreq() 
{
  shared_ptr<ValueSource> vs = make_shared<DocFreqValueSource>(
      L"bogus", L"bogus", L"text", make_shared<BytesRef>(L"test"));
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{2.0f, 2.0f});
  assertAllExist(vs);
}

void TestValueSources::testDoubleConst() 
{
  shared_ptr<ValueSource> vs = make_shared<DoubleConstValueSource>(0.3);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{0.3f, 0.3f});
  assertAllExist(vs);
}

void TestValueSources::testDouble() 
{
  shared_ptr<ValueSource> vs = make_shared<DoubleFieldSource>(L"double");
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{3.63f, 5.65f});
  assertAllExist(vs);
  assertNoneExist(BOGUS_DOUBLE_VS);
}

void TestValueSources::testDoubleMultiValued() 
{
  shared_ptr<ValueSource> vs =
      make_shared<MultiValuedDoubleFieldSource>(L"doubleMv", Type::MAX);
  assertHits(make_shared<FunctionQuery>(vs),
             std::deque<float>{3.69f, 123.456f});
  assertAllExist(vs);

  vs = make_shared<MultiValuedDoubleFieldSource>(L"doubleMv", Type::MIN);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{-0.11f, 0.01f});
  assertAllExist(vs);
}

void TestValueSources::testFloat() 
{
  shared_ptr<ValueSource> vs = make_shared<FloatFieldSource>(L"float");
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{5.2f, 9.3f});
  assertAllExist(vs);
  assertNoneExist(BOGUS_FLOAT_VS);
}

void TestValueSources::testFloatMultiValued() 
{
  shared_ptr<ValueSource> vs =
      make_shared<MultiValuedFloatFieldSource>(L"floatMv", Type::MAX);
  assertHits(make_shared<FunctionQuery>(vs),
             std::deque<float>{3.69f, 123.456f});
  assertAllExist(vs);

  vs = make_shared<MultiValuedFloatFieldSource>(L"floatMv", Type::MIN);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{-0.11f, 0.01f});
  assertAllExist(vs);
}

void TestValueSources::testIDF() 
{
  shared_ptr<Similarity> saved = searcher->getSimilarity(true);
  try {
    searcher->setSimilarity(make_shared<ClassicSimilarity>());
    shared_ptr<ValueSource> vs = make_shared<IDFValueSource>(
        L"bogus", L"bogus", L"text", make_shared<BytesRef>(L"test"));
    assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{1.0f, 1.0f});
    assertAllExist(vs);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    searcher->setSimilarity(saved);
  }
}

void TestValueSources::testIf() 
{
  shared_ptr<ValueSource> vs = make_shared<IfFunction>(
      make_shared<BytesRefFieldSource>(L"id"),
      make_shared<ConstValueSource>(1.0f), make_shared<ConstValueSource>(2.0f));
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{1.0f, 1.0f});
  assertAllExist(vs);

  // true just if a test value exists...
  vs = make_shared<IfFunction>(make_shared<LiteralValueSource>(L"false"),
                               make_shared<ConstValueSource>(1.0f),
                               make_shared<ConstValueSource>(2.0f));
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{1.0f, 1.0f});
  assertAllExist(vs);

  // false value if tests value does not exist
  vs = make_shared<IfFunction>(BOGUS_FLOAT_VS,
                               make_shared<ConstValueSource>(1.0f),
                               make_shared<ConstValueSource>(2.0f)); // match
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{2.0F, 2.0F});
  assertAllExist(vs);

  // final value may still not exist
  vs = make_shared<IfFunction>(make_shared<BytesRefFieldSource>(L"id"),
                               BOGUS_FLOAT_VS,
                               make_shared<ConstValueSource>(1.0f));
  assertNoneExist(vs);
}

void TestValueSources::testInt() 
{
  shared_ptr<ValueSource> vs = make_shared<IntFieldSource>(L"int");
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{35.0f, 54.0f});
  assertAllExist(vs);
  assertNoneExist(BOGUS_INT_VS);
}

void TestValueSources::testIntMultiValued() 
{
  shared_ptr<ValueSource> vs =
      make_shared<MultiValuedIntFieldSource>(L"intMv", Type::MAX);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{7.0f, 900.0f});
  assertAllExist(vs);

  vs = make_shared<MultiValuedIntFieldSource>(L"intMv", Type::MIN);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{1.0f, -1.0f});
  assertAllExist(vs);
}

void TestValueSources::testJoinDocFreq() 
{
  assertHits(make_shared<FunctionQuery>(
                 make_shared<JoinDocFreqValueSource>(L"string", L"text")),
             std::deque<float>{2.0f, 0.0f});

  // TODO: what *should* the rules be for exist() ?
}

void TestValueSources::testLinearFloat() 
{
  shared_ptr<ValueSource> vs = make_shared<LinearFloatFunction>(
      make_shared<ConstValueSource>(2.0f), 3, 1);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{7.0f, 7.0f});
  assertAllExist(vs);
  vs = make_shared<LinearFloatFunction>(BOGUS_FLOAT_VS, 3, 1);
  assertNoneExist(vs);
}

void TestValueSources::testLong() 
{
  shared_ptr<ValueSource> vs = make_shared<LongFieldSource>(L"long");
  assertHits(make_shared<FunctionQuery>(vs),
             std::deque<float>{4343.0f, 1954.0f});
  assertAllExist(vs);
  assertNoneExist(BOGUS_LONG_VS);
}

void TestValueSources::testLongMultiValued() 
{
  shared_ptr<ValueSource> vs =
      make_shared<MultiValuedLongFieldSource>(L"longMv", Type::MAX);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{7.0f, 900.0f});
  assertAllExist(vs);

  vs = make_shared<MultiValuedLongFieldSource>(L"longMv", Type::MIN);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{1.0f, -1.0f});
  assertAllExist(vs);
}

void TestValueSources::testMaxDoc() 
{
  shared_ptr<ValueSource> vs = make_shared<MaxDocValueSource>();
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{2.0f, 2.0f});
  assertAllExist(vs);
}

void TestValueSources::testMaxFloat() 
{
  shared_ptr<ValueSource> vs =
      make_shared<MaxFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
          make_shared<ConstValueSource>(1.0f),
          make_shared<ConstValueSource>(2.0f)});

  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{2.0f, 2.0f});
  assertAllExist(vs);

  // as long as one value exists, then max exists
  vs = make_shared<MaxFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
      BOGUS_FLOAT_VS, make_shared<ConstValueSource>(2.0F)});
  assertAllExist(vs);
  vs = make_shared<MaxFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
      BOGUS_FLOAT_VS, make_shared<ConstValueSource>(2.0F), BOGUS_DOUBLE_VS});
  assertAllExist(vs);
  // if none exist, then max doesn't exist
  vs = make_shared<MaxFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
      BOGUS_FLOAT_VS, BOGUS_INT_VS, BOGUS_DOUBLE_VS});
  assertNoneExist(vs);
}

void TestValueSources::testMinFloat() 
{
  shared_ptr<ValueSource> vs =
      make_shared<MinFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
          make_shared<ConstValueSource>(1.0f),
          make_shared<ConstValueSource>(2.0f)});

  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{1.0f, 1.0f});
  assertAllExist(vs);

  // as long as one value exists, then min exists
  vs = make_shared<MinFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
      BOGUS_FLOAT_VS, make_shared<ConstValueSource>(2.0F)});
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{2.0F, 2.0F});
  assertAllExist(vs);
  vs = make_shared<MinFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
      BOGUS_FLOAT_VS, make_shared<ConstValueSource>(2.0F), BOGUS_DOUBLE_VS});
  assertAllExist(vs);

  // if none exist, then min doesn't exist
  vs = make_shared<MinFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
      BOGUS_FLOAT_VS, BOGUS_INT_VS, BOGUS_DOUBLE_VS});
  assertNoneExist(vs);
}

void TestValueSources::testNorm() 
{
  shared_ptr<Similarity> saved = searcher->getSimilarity(true);
  try {
    // no norm field (so agnostic to indexed similarity)
    searcher->setSimilarity(make_shared<ClassicSimilarity>());
    shared_ptr<ValueSource> vs = make_shared<NormValueSource>(L"byte");
    assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{1.0f, 1.0f});

    // regardless of whether norms exist, value source exists == 0
    assertAllExist(vs);

    vs = make_shared<NormValueSource>(L"text");
    assertAllExist(vs);

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    searcher->setSimilarity(saved);
  }
}

void TestValueSources::testNumDocs() 
{
  shared_ptr<ValueSource> vs = make_shared<NumDocsValueSource>();
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{2.0f, 2.0f});
  assertAllExist(vs);
}

void TestValueSources::testPow() 
{
  shared_ptr<ValueSource> vs = make_shared<PowFloatFunction>(
      make_shared<ConstValueSource>(2.0f), make_shared<ConstValueSource>(3.0f));
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{8.0f, 8.0f});
  assertAllExist(vs);
  vs = make_shared<PowFloatFunction>(BOGUS_FLOAT_VS,
                                     make_shared<ConstValueSource>(3.0f));
  assertNoneExist(vs);
  vs = make_shared<PowFloatFunction>(make_shared<ConstValueSource>(3.0f),
                                     BOGUS_FLOAT_VS);
  assertNoneExist(vs);
}

void TestValueSources::testProduct() 
{
  shared_ptr<ValueSource> vs = make_shared<ProductFloatFunction>(
      std::deque<std::shared_ptr<ValueSource>>{
          make_shared<ConstValueSource>(2.0f),
          make_shared<ConstValueSource>(3.0f)});
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{6.0f, 6.0f});
  assertAllExist(vs);

  vs = make_shared<ProductFloatFunction>(
      std::deque<std::shared_ptr<ValueSource>>{
          BOGUS_FLOAT_VS, make_shared<ConstValueSource>(3.0f)});
  assertNoneExist(vs);
}

void TestValueSources::testQueryWrapedFuncWrapedQuery() 
{
  shared_ptr<ValueSource> vs = make_shared<QueryValueSource>(
      make_shared<FunctionQuery>(make_shared<ConstValueSource>(2.0f)), 0.0f);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{2.0f, 2.0f});
  assertAllExist(vs);
}

void TestValueSources::testQuery() 
{
  shared_ptr<Similarity> saved = searcher->getSimilarity(true);

  try {
    searcher->setSimilarity(make_shared<ClassicSimilarity>());

    shared_ptr<ValueSource> vs = make_shared<QueryValueSource>(
        make_shared<TermQuery>(make_shared<Term>(L"string", L"bar")), 42.0F);
    assertHits(make_shared<FunctionQuery>(vs),
               std::deque<float>{42.0F, 1.4054651F});

    // valuesource should exist only for things matching the term query
    // sanity check via quick & dirty wrapper arround tf
    shared_ptr<ValueSource> expected =
        make_shared<MultiFloatFunctionAnonymousInnerClass>(
            shared_from_this(),
            std::deque<std::shared_ptr<ValueSource>>{
                make_shared<TFValueSource>(L"bogus", L"bogus", L"string",
                                           make_shared<BytesRef>(L"bar"))});

    assertExists(expected, vs);

    // Query matches all docs, func exists for all docs
    vs = make_shared<QueryValueSource>(
        make_shared<TermQuery>(make_shared<Term>(L"text", L"test")), 0.0F);
    assertAllExist(vs);

    // Query matches no docs, func exists for no docs
    vs = make_shared<QueryValueSource>(
        make_shared<TermQuery>(make_shared<Term>(L"bogus", L"does not exist")),
        0.0F);
    assertNoneExist(vs);

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    searcher->setSimilarity(saved);
  }
}

TestValueSources::MultiFloatFunctionAnonymousInnerClass::
    MultiFloatFunctionAnonymousInnerClass(
        shared_ptr<TestValueSources> outerInstance,
        std::deque<std::shared_ptr<ValueSource>> &org)
    : org.apache::lucene::queries::function::valuesource::MultiFloatFunction(
          BytesRef(L"bar"))
} // namespace org::apache::lucene::queries::function
)
    {
        this->outerInstance = outerInstance;
    }

wstring TestValueSources::MultiFloatFunctionAnonymousInnerClass::name()
{
  return L"tf_based_exists";
}

float TestValueSources::MultiFloatFunctionAnonymousInnerClass::func(
    int doc,
    std::deque<std::shared_ptr<FunctionValues>> &valsArr) 
{
  return valsArr[0]->floatVal(doc);
}

bool TestValueSources::MultiFloatFunctionAnonymousInnerClass::exists(
    int doc,
    std::deque<std::shared_ptr<FunctionValues>> &valsArr) 
{
  // if tf > 0, then it should exist
  return 0 < func(doc, valsArr);
}

void TestValueSources::testRangeMap() 
{
  assertHits(make_shared<FunctionQuery>(make_shared<RangeMapFloatFunction>(
                 make_shared<FloatFieldSource>(L"float"), 5, 6, 1, 0.0f)),
             std::deque<float>{1.0f, 0.0f});
  assertHits(make_shared<FunctionQuery>(make_shared<RangeMapFloatFunction>(
                 make_shared<FloatFieldSource>(L"float"), 5, 6,
                 make_shared<SumFloatFunction>(
                     std::deque<std::shared_ptr<ValueSource>>{
                         make_shared<ConstValueSource>(1.0f),
                         make_shared<ConstValueSource>(2.0f)}),
                 make_shared<ConstValueSource>(11.0f))),
             std::deque<float>{3.0f, 11.0f});

  // TODO: what *should* the rules be for exist() ?
  // ((source exists && source in range && target exists) OR (source not in
  // range && default exists)) ?
}

void TestValueSources::testReciprocal() 
{
  shared_ptr<ValueSource> vs = make_shared<ReciprocalFloatFunction>(
      make_shared<ConstValueSource>(2.0f), 3, 1, 4);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{0.1f, 0.1f});
  assertAllExist(vs);

  vs = make_shared<ReciprocalFloatFunction>(BOGUS_FLOAT_VS, 3, 1, 4);
  assertNoneExist(vs);
}

void TestValueSources::testScale() 
{
  shared_ptr<ValueSource> vs = make_shared<ScaleFloatFunction>(
      make_shared<IntFieldSource>(L"int"), 0, 1);
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{0.0f, 1.0f});
  assertAllExist(vs);

  vs = make_shared<ScaleFloatFunction>(BOGUS_INT_VS, 0, 1);
  assertNoneExist(vs);
}

void TestValueSources::testSumFloat() 
{
  shared_ptr<ValueSource> vs =
      make_shared<SumFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
          make_shared<ConstValueSource>(1.0f),
          make_shared<ConstValueSource>(2.0f)});
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{3.0f, 3.0f});
  assertAllExist(vs);

  vs = make_shared<SumFloatFunction>(std::deque<std::shared_ptr<ValueSource>>{
      BOGUS_FLOAT_VS, make_shared<ConstValueSource>(2.0f)});
  assertNoneExist(vs);
}

void TestValueSources::testSumTotalTermFreq() 
{
  shared_ptr<ValueSource> vs =
      make_shared<SumTotalTermFreqValueSource>(L"text");
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{8.0f, 8.0f});
  assertAllExist(vs);
}

void TestValueSources::testTermFreq() 
{
  shared_ptr<ValueSource> vs = make_shared<TermFreqValueSource>(
      L"bogus", L"bogus", L"text", make_shared<BytesRef>(L"test"));
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{3.0f, 1.0f});
  assertAllExist(vs);

  vs = make_shared<TermFreqValueSource>(L"bogus", L"bogus", L"string",
                                        make_shared<BytesRef>(L"bar"));
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{0.0f, 1.0f});
  assertAllExist(vs);

  // regardless of whether norms exist, value source exists == 0
  vs = make_shared<TermFreqValueSource>(L"bogus", L"bogus", L"bogus",
                                        make_shared<BytesRef>(L"bogus"));
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{0.0F, 0.0F});
  assertAllExist(vs);
}

void TestValueSources::testTF() 
{
  shared_ptr<Similarity> saved = searcher->getSimilarity(true);
  try {
    // no norm field (so agnostic to indexed similarity)
    searcher->setSimilarity(make_shared<ClassicSimilarity>());

    shared_ptr<ValueSource> vs = make_shared<TFValueSource>(
        L"bogus", L"bogus", L"text", make_shared<BytesRef>(L"test"));
    assertHits(make_shared<FunctionQuery>(vs),
               std::deque<float>{static_cast<float>(sqrt(3)),
                                  static_cast<float>(sqrt(1))});
    assertAllExist(vs);

    vs = make_shared<TFValueSource>(L"bogus", L"bogus", L"string",
                                    make_shared<BytesRef>(L"bar"));
    assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{0.0f, 1.0f});
    assertAllExist(vs);

    // regardless of whether norms exist, value source exists == 0
    vs = make_shared<TFValueSource>(L"bogus", L"bogus", L"bogus",
                                    make_shared<BytesRef>(L"bogus"));
    assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{0.0F, 0.0F});
    assertAllExist(vs);

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    searcher->setSimilarity(saved);
  }
}

void TestValueSources::testTotalTermFreq() 
{
  shared_ptr<ValueSource> vs = make_shared<TotalTermFreqValueSource>(
      L"bogus", L"bogus", L"text", make_shared<BytesRef>(L"test"));
  assertHits(make_shared<FunctionQuery>(vs), std::deque<float>{4.0f, 4.0f});
  assertAllExist(vs);
}

void TestValueSources::testMultiFunctionHelperEquivilence() 
{
  // the 2 arg versions of these methods should return the exact same results as
  // the multi arg versions with a 2 element array

  // actual doc / index is not relevant for this test
  shared_ptr<LeafReaderContext> *const leaf =
      searcher->getIndexReader()->leaves()[0];
  constexpr unordered_map context = ValueSource::newContext(searcher);

  ALL_EXIST_VS->createWeight(context, searcher);
  NONE_EXIST_VS->createWeight(context, searcher);

  shared_ptr<FunctionValues> *const ALL =
      ALL_EXIST_VS->getValues(context, leaf);
  shared_ptr<FunctionValues> *const NONE =
      NONE_EXIST_VS->getValues(context, leaf);

  // quick sanity checks of explicit results
  assertTrue(MultiFunction::allExists(1, ALL, ALL));
  assertTrue(MultiFunction::allExists(
      1, std::deque<std::shared_ptr<FunctionValues>>{ALL, ALL}));
  assertTrue(MultiFunction::anyExists(1, ALL, NONE));
  assertTrue(MultiFunction::anyExists(
      1, std::deque<std::shared_ptr<FunctionValues>>{ALL, NONE}));
  //
  assertFalse(MultiFunction::allExists(1, ALL, NONE));
  assertFalse(MultiFunction::allExists(
      1, std::deque<std::shared_ptr<FunctionValues>>{ALL, NONE}));
  assertFalse(MultiFunction::anyExists(1, NONE, NONE));
  assertFalse(MultiFunction::anyExists(
      1, std::deque<std::shared_ptr<FunctionValues>>{NONE, NONE}));

  // iterate all permutations and verify equivilence
  for (auto firstArg :
       std::deque<std::shared_ptr<FunctionValues>>{ALL, NONE}) {
    for (auto secondArg :
         std::deque<std::shared_ptr<FunctionValues>>{ALL, NONE}) {
      assertEquals(L"allExists(" + firstArg + L"," + secondArg + L")",
                   MultiFunction::allExists(1, firstArg, secondArg),
                   MultiFunction::allExists(
                       1, std::deque<std::shared_ptr<FunctionValues>>{
                              firstArg, secondArg}));
      assertEquals(L"anyExists(" + firstArg + L"," + secondArg + L")",
                   MultiFunction::anyExists(1, firstArg, secondArg),
                   MultiFunction::anyExists(
                       1, std::deque<std::shared_ptr<FunctionValues>>{
                              firstArg, secondArg}));

      // future proof against posibility of someone "optimizing" the array
      // method if .length==2 ... redundent third arg should give same results
      // as well...
      assertEquals(
          L"allExists(" + firstArg + L"," + secondArg + L"," + secondArg + L")",
          MultiFunction::allExists(1, firstArg, secondArg),
          MultiFunction::allExists(1,
                                   std::deque<std::shared_ptr<FunctionValues>>{
                                       firstArg, secondArg, secondArg}));
      assertEquals(
          L"anyExists(" + firstArg + L"," + secondArg + L"," + secondArg + L")",
          MultiFunction::anyExists(1, firstArg, secondArg),
          MultiFunction::anyExists(1,
                                   std::deque<std::shared_ptr<FunctionValues>>{
                                       firstArg, secondArg, secondArg}));
    }
  }
}

void TestValueSources::assertAllExist(shared_ptr<ValueSource> vs)
{
  assertExists(ALL_EXIST_VS, vs);
}

void TestValueSources::assertNoneExist(shared_ptr<ValueSource> vs)
{
  assertExists(NONE_EXIST_VS, vs);
}

void TestValueSources::assertExists(shared_ptr<ValueSource> expected,
                                    shared_ptr<ValueSource> actual)
{
  unordered_map context = ValueSource::newContext(searcher);
  try {
    expected->createWeight(context, searcher);
    actual->createWeight(context, searcher);

    for (auto leaf : searcher->getIndexReader()->leaves()) {
      shared_ptr<FunctionValues> *const expectedVals =
          expected->getValues(context, leaf);
      shared_ptr<FunctionValues> *const actualVals =
          actual->getValues(context, leaf);

      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring msg =
          expected->toString() + L" ?= " + actual->toString() + L" -> ";
      for (int i = 0; i < leaf->reader()->maxDoc(); ++i) {
        assertEquals(msg + to_wstring(i), expectedVals->exists(i),
                     actualVals->exists(i));
      }
    }
  } catch (const IOException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException(actual.toString(), e);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw runtime_error(actual->toString());
  }
}

void TestValueSources::assertHits(
    shared_ptr<Query> q, std::deque<float> &scores) 
{
  std::deque<std::shared_ptr<ScoreDoc>> expected(scores.size());
  std::deque<int> expectedDocs(scores.size());
  for (int i = 0; i < expected.size(); i++) {
    expectedDocs[i] = i;
    expected[i] = make_shared<ScoreDoc>(i, scores[i]);
  }
  shared_ptr<TopDocs> docs = searcher->search(
      q, documents.size(),
      make_shared<Sort>(make_shared<SortField>(L"id", SortField::Type::STRING)),
      true, false);
  CheckHits::checkHits(random(), q, L"", searcher, expectedDocs);
  CheckHits::checkHitsQuery(q, expected, docs->scoreDocs, expectedDocs);
  CheckHits::checkExplanations(q, L"", searcher);
}

TestValueSources::ExistsValueSource::ExistsValueSource(bool expected)
    : expected(expected), value(expected ? 1 : 0)
{
}

bool TestValueSources::ExistsValueSource::equals(any o)
{
  return o == shared_from_this();
}

int TestValueSources::ExistsValueSource::hashCode() { return value; }

wstring TestValueSources::ExistsValueSource::description()
{
  return expected ? L"ALL_EXIST" : L"NONE_EXIST";
}

shared_ptr<FunctionValues> TestValueSources::ExistsValueSource::getValues(
    unordered_map context, shared_ptr<LeafReaderContext> readerContext)
{
  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this());
}

TestValueSources::ExistsValueSource::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<ExistsValueSource> outerInstance)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
}

float TestValueSources::ExistsValueSource::FloatDocValuesAnonymousInnerClass::
    floatVal(int doc)
{
  return static_cast<float>(outerInstance->value);
}

bool TestValueSources::ExistsValueSource::FloatDocValuesAnonymousInnerClass::
    exists(int doc)
{
  return outerInstance->expected;
}

const shared_ptr<ValueSource> TestValueSources::ALL_EXIST_VS =
    make_shared<ExistsValueSource>(true);
const shared_ptr<ValueSource> TestValueSources::NONE_EXIST_VS =
    make_shared<ExistsValueSource>(false);
}