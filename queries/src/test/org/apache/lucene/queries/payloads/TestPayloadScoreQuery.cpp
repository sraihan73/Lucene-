using namespace std;

#include "TestPayloadScoreQuery.h"

namespace org::apache::lucene::queries::payloads
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using TopDocs = org::apache::lucene::search::TopDocs;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using SpanContainingQuery =
    org::apache::lucene::search::spans::SpanContainingQuery;
using SpanMultiTermQueryWrapper =
    org::apache::lucene::search::spans::SpanMultiTermQueryWrapper;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using org::junit::Test;

void TestPayloadScoreQuery::checkQuery(
    shared_ptr<SpanQuery> query, shared_ptr<PayloadFunction> function,
    std::deque<int> &expectedDocs,
    std::deque<float> &expectedScores) 
{
  checkQuery(query, function, true, expectedDocs, expectedScores);
}

void TestPayloadScoreQuery::checkQuery(
    shared_ptr<SpanQuery> query, shared_ptr<PayloadFunction> function,
    bool includeSpanScore, std::deque<int> &expectedDocs,
    std::deque<float> &expectedScores) 
{

  assertTrue(L"Expected docs and scores arrays must be the same length!",
             expectedDocs.size() == expectedScores.size());

  shared_ptr<PayloadScoreQuery> psq = make_shared<PayloadScoreQuery>(
      query, function, PayloadDecoder::FLOAT_DECODER, includeSpanScore);
  shared_ptr<TopDocs> hits = searcher->search(psq, expectedDocs.size());

  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    if (i > expectedDocs.size() - 1) {
      fail(L"Unexpected hit in document " +
           to_wstring(hits->scoreDocs[i]->doc));
    }
    if (hits->scoreDocs[i]->doc != expectedDocs[i]) {
      fail(L"Unexpected hit in document " +
           to_wstring(hits->scoreDocs[i]->doc));
    }
    assertEquals(L"Bad score in document " + to_wstring(expectedDocs[i]),
                 expectedScores[i], hits->scoreDocs[i]->score, 0.000001);
  }

  if (hits->scoreDocs.size() > expectedDocs.size()) {
    fail(L"Unexpected hit in document " + hits->scoreDocs[expectedDocs.size()]);
  }

  QueryUtils::check(random(), psq, searcher);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTermQuery() throws java.io.IOException
void TestPayloadScoreQuery::testTermQuery() 
{

  shared_ptr<SpanTermQuery> q =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"eighteen"));
  for (auto fn : std::deque<std::shared_ptr<PayloadFunction>>{
           make_shared<AveragePayloadFunction>(),
           make_shared<MaxPayloadFunction>(),
           make_shared<MinPayloadFunction>()}) {
    checkQuery(q, fn, std::deque<int>{118, 218, 18},
               std::deque<float>{4.0f, 4.0f, 2.0f});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOrQuery() throws java.io.IOException
void TestPayloadScoreQuery::testOrQuery() 
{

  shared_ptr<SpanOrQuery> q = make_shared<SpanOrQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"eighteen")),
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"nineteen")));
  for (auto fn : std::deque<std::shared_ptr<PayloadFunction>>{
           make_shared<AveragePayloadFunction>(),
           make_shared<MaxPayloadFunction>(),
           make_shared<MinPayloadFunction>()}) {
    checkQuery(q, fn, std::deque<int>{118, 119, 218, 219, 18, 19},
               std::deque<float>{4.0f, 4.0f, 4.0f, 4.0f, 2.0f, 2.0f});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNearQuery() throws java.io.IOException
void TestPayloadScoreQuery::testNearQuery() 
{

  //   2     4
  // twenty two
  //  2     4      4     4
  // one hundred twenty two

  shared_ptr<SpanNearQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"twenty")),
          make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"two"))},
      0, true);

  checkQuery(q, make_shared<MaxPayloadFunction>(),
             std::deque<int>{22, 122, 222},
             std::deque<float>{4.0f, 4.0f, 4.0f});
  checkQuery(q, make_shared<MinPayloadFunction>(),
             std::deque<int>{122, 222, 22},
             std::deque<float>{4.0f, 4.0f, 2.0f});
  checkQuery(q, make_shared<AveragePayloadFunction>(),
             std::deque<int>{122, 222, 22},
             std::deque<float>{4.0f, 4.0f, 3.0f});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNestedNearQuery() throws Exception
void TestPayloadScoreQuery::testNestedNearQuery() 
{

  // (one OR hundred) NEAR (twenty two) ~ 1
  //  2    4        4    4
  // one hundred twenty two
  // two hundred twenty two

  shared_ptr<SpanNearQuery> q = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{
          make_shared<SpanOrQuery>(
              make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"one")),
              make_shared<SpanTermQuery>(
                  make_shared<Term>(L"field", L"hundred"))),
          make_shared<SpanNearQuery>(
              std::deque<std::shared_ptr<SpanQuery>>{
                  make_shared<SpanTermQuery>(
                      make_shared<Term>(L"field", L"twenty")),
                  make_shared<SpanTermQuery>(
                      make_shared<Term>(L"field", L"two"))},
              0, true)},
      1, true);

  // check includeSpanScore makes a difference here
  searcher->setSimilarity(make_shared<ClassicSimilarity>());
  try {
    checkQuery(q, make_shared<MaxPayloadFunction>(), std::deque<int>{122, 222},
               std::deque<float>{20.901256561279297f, 17.06580352783203f});
    checkQuery(q, make_shared<MinPayloadFunction>(), std::deque<int>{222, 122},
               std::deque<float>{17.06580352783203f, 10.450628280639648f});
    checkQuery(q, make_shared<AveragePayloadFunction>(),
               std::deque<int>{122, 222},
               std::deque<float>{19.15948486328125f, 17.06580352783203f});
    checkQuery(q, make_shared<MaxPayloadFunction>(), false,
               std::deque<int>{122, 222}, std::deque<float>{4.0f, 4.0f});
    checkQuery(q, make_shared<MinPayloadFunction>(), false,
               std::deque<int>{222, 122}, std::deque<float>{4.0f, 2.0f});
    checkQuery(q, make_shared<AveragePayloadFunction>(), false,
               std::deque<int>{222, 122}, std::deque<float>{4.0f, 3.666666f});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    searcher->setSimilarity(similarity);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSpanContainingQuery() throws Exception
void TestPayloadScoreQuery::testSpanContainingQuery() 
{

  // twenty WITHIN ((one OR hundred) NEAR two)~2
  shared_ptr<SpanContainingQuery> q = make_shared<SpanContainingQuery>(
      make_shared<SpanNearQuery>(
          std::deque<std::shared_ptr<SpanQuery>>{
              make_shared<SpanOrQuery>(
                  make_shared<SpanTermQuery>(
                      make_shared<Term>(L"field", L"one")),
                  make_shared<SpanTermQuery>(
                      make_shared<Term>(L"field", L"hundred"))),
              make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"two"))},
          2, true),
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"twenty")));

  checkQuery(q, make_shared<AveragePayloadFunction>(),
             std::deque<int>{222, 122}, std::deque<float>{4.0f, 3.666666f});
  checkQuery(q, make_shared<MaxPayloadFunction>(), std::deque<int>{122, 222},
             std::deque<float>{4.0f, 4.0f});
  checkQuery(q, make_shared<MinPayloadFunction>(), std::deque<int>{222, 122},
             std::deque<float>{4.0f, 2.0f});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEquality()
void TestPayloadScoreQuery::testEquality()
{
  shared_ptr<SpanQuery> sq1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"one"));
  shared_ptr<SpanQuery> sq2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"two"));
  shared_ptr<PayloadFunction> minFunc = make_shared<MinPayloadFunction>();
  shared_ptr<PayloadFunction> maxFunc = make_shared<MaxPayloadFunction>();
  shared_ptr<PayloadScoreQuery> query1 = make_shared<PayloadScoreQuery>(
      sq1, minFunc, PayloadDecoder::FLOAT_DECODER, true);
  shared_ptr<PayloadScoreQuery> query2 = make_shared<PayloadScoreQuery>(
      sq2, minFunc, PayloadDecoder::FLOAT_DECODER, true);
  shared_ptr<PayloadScoreQuery> query3 = make_shared<PayloadScoreQuery>(
      sq2, maxFunc, PayloadDecoder::FLOAT_DECODER, true);
  shared_ptr<PayloadScoreQuery> query4 = make_shared<PayloadScoreQuery>(
      sq2, maxFunc, PayloadDecoder::FLOAT_DECODER, false);
  shared_ptr<PayloadScoreQuery> query5 = make_shared<PayloadScoreQuery>(
      sq1, minFunc, PayloadDecoder::FLOAT_DECODER);

  assertEquals(query1, query5);
  assertFalse(query1->equals(query2));
  assertFalse(query1->equals(query3));
  assertFalse(query1->equals(query4));
  assertFalse(query2->equals(query3));
  assertFalse(query2->equals(query4));
  assertFalse(query3->equals(query4));
}

void TestPayloadScoreQuery::testRewrite() 
{
  shared_ptr<SpanMultiTermQueryWrapper> xyz =
      make_shared<SpanMultiTermQueryWrapper<>>(
          make_shared<WildcardQuery>(make_shared<Term>(L"field", L"xyz*")));
  shared_ptr<PayloadScoreQuery> psq =
      make_shared<PayloadScoreQuery>(xyz, make_shared<AveragePayloadFunction>(),
                                     PayloadDecoder::FLOAT_DECODER, false);

  // if query wasn't rewritten properly, the query would have failed with
  // "Rewrite first!"
  searcher->search(psq, 1);
}

shared_ptr<org::apache::lucene::search::IndexSearcher>
    TestPayloadScoreQuery::searcher;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestPayloadScoreQuery::reader;
shared_ptr<org::apache::lucene::store::Directory>
    TestPayloadScoreQuery::directory;
shared_ptr<JustScorePayloadSimilarity> TestPayloadScoreQuery::similarity =
    make_shared<JustScorePayloadSimilarity>();
std::deque<char> TestPayloadScoreQuery::payload2 = {2};
std::deque<char> TestPayloadScoreQuery::payload4 = {4};

shared_ptr<Analyzer::TokenStreamComponents>
TestPayloadScoreQuery::PayloadAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      result, make_shared<PayloadFilter>(result));
}

TestPayloadScoreQuery::PayloadFilter::PayloadFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input),
      payAtt(addAttribute(PayloadAttribute::typeid))
{
}

bool TestPayloadScoreQuery::PayloadFilter::incrementToken() 
{
  bool result = false;
  if (input->incrementToken()) {
    if (numSeen % 4 == 0) {
      payAtt->setPayload(make_shared<BytesRef>(payload2));
    } else {
      payAtt->setPayload(make_shared<BytesRef>(payload4));
    }
    numSeen++;
    result = true;
  }
  return result;
}

void TestPayloadScoreQuery::PayloadFilter::reset() 
{
  TokenFilter::reset();
  this->numSeen = 0;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestPayloadScoreQuery::beforeClass() 
{
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<PayloadAnalyzer>())
          ->setMergePolicy(NoMergePolicy::INSTANCE));
  // writer.infoStream = System.out;
  for (int i = 0; i < 300; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"field", English::intToEnglish(i), Field::Store::YES));
    wstring txt = English::intToEnglish(i) + StringHelper::toString(L' ') +
                  English::intToEnglish(i + 1);
    doc->push_back(newTextField(L"field2", txt, Field::Store::YES));
    writer->addDocument(doc);
  }
  reader = writer->getReader();
  delete writer;

  searcher = newSearcher(reader);
  searcher->setSimilarity(make_shared<JustScorePayloadSimilarity>());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestPayloadScoreQuery::afterClass() 
{
  searcher.reset();
  delete reader;
  reader.reset();
  delete directory;
  directory.reset();
}

float TestPayloadScoreQuery::JustScorePayloadSimilarity::lengthNorm(int length)
{
  return 1;
}

float TestPayloadScoreQuery::JustScorePayloadSimilarity::sloppyFreq(
    int distance)
{
  return 1.0f;
}

float TestPayloadScoreQuery::JustScorePayloadSimilarity::tf(float freq)
{
  return 1.0f;
}

shared_ptr<Explanation>
TestPayloadScoreQuery::JustScorePayloadSimilarity::idfExplain(
    shared_ptr<CollectionStatistics> collectionStats,
    std::deque<std::shared_ptr<TermStatistics>> &termStats)
{
  return Explanation::match(1.0f, L"Inexplicable");
}

shared_ptr<Explanation>
TestPayloadScoreQuery::JustScorePayloadSimilarity::idfExplain(
    shared_ptr<CollectionStatistics> collectionStats,
    shared_ptr<TermStatistics> termStats)
{
  return Explanation::match(1.0f, L"Inexplicable");
}
} // namespace org::apache::lucene::queries::payloads