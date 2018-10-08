using namespace std;

#include "TestFieldScoreQuery.h"

namespace org::apache::lucene::queries::function
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using org::junit::BeforeClass;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestFieldScoreQuery::beforeClass() 
{
  createIndex(true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRankInt() throws Exception
void TestFieldScoreQuery::testRankInt() 
{
  doTestRank(INT_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRankIntMultiValued() throws Exception
void TestFieldScoreQuery::testRankIntMultiValued() 
{
  doTestRank(INT_MV_MAX_VALUESOURCE);
  doTestRank(INT_MV_MIN_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRankFloat() throws Exception
void TestFieldScoreQuery::testRankFloat() 
{
  // same values, but in flot format
  doTestRank(FLOAT_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRankFloatMultiValued() throws Exception
void TestFieldScoreQuery::testRankFloatMultiValued() 
{
  // same values, but in flot format
  doTestRank(FLOAT_MV_MAX_VALUESOURCE);
  doTestRank(FLOAT_MV_MIN_VALUESOURCE);
}

void TestFieldScoreQuery::doTestRank(shared_ptr<ValueSource> valueSource) throw(
    runtime_error)
{
  shared_ptr<Query> functionQuery = getFunctionQuery(valueSource);
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> s = newSearcher(r);
  log(L"test: " + functionQuery);
  QueryUtils::check(random(), functionQuery, s);
  std::deque<std::shared_ptr<ScoreDoc>> h =
      s->search(functionQuery, 1000)->scoreDocs;
  assertEquals(L"All docs should be matched!", N_DOCS, h.size());
  wstring prevID =
      L"ID" +
      to_wstring(N_DOCS + 1); // greater than all ids of docs in this test
  for (int i = 0; i < h.size(); i++) {
    wstring resID = s->doc(h[i]->doc)[ID_FIELD];
    log(to_wstring(i) + L".   score=" + to_wstring(h[i]->score) + L"  -  " +
        resID);
    log(s->explain(functionQuery, h[i]->doc));
    assertTrue(L"res id " + resID + L" should be < prev res id " + prevID,
               resID.compare(prevID) < 0);
    prevID = resID;
  }
  delete r;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExactScoreInt() throws Exception
void TestFieldScoreQuery::testExactScoreInt() 
{
  doTestExactScore(INT_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExactScoreIntMultiValued() throws
// Exception
void TestFieldScoreQuery::testExactScoreIntMultiValued() 
{
  doTestExactScore(INT_MV_MAX_VALUESOURCE);
  doTestExactScore(INT_MV_MIN_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExactScoreFloat() throws Exception
void TestFieldScoreQuery::testExactScoreFloat() 
{
  // same values, but in flot format
  doTestExactScore(FLOAT_VALUESOURCE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExactScoreFloatMultiValued() throws
// Exception
void TestFieldScoreQuery::testExactScoreFloatMultiValued() 
{
  // same values, but in flot format
  doTestExactScore(FLOAT_MV_MAX_VALUESOURCE);
  doTestExactScore(FLOAT_MV_MIN_VALUESOURCE);
}

void TestFieldScoreQuery::doTestExactScore(
    shared_ptr<ValueSource> valueSource) 
{
  shared_ptr<Query> functionQuery = getFunctionQuery(valueSource);
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<TopDocs> td = s->search(functionQuery, 1000);
  assertEquals(L"All docs should be matched!", N_DOCS, td->totalHits);
  std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
  for (auto aSd : sd) {
    float score = aSd->score;
    log(s->explain(functionQuery, aSd->doc));
    wstring id = s->getIndexReader()->document(aSd->doc)[ID_FIELD];
    float expectedScore = expectedFieldScore(id); // "ID7" --> 7.0
    assertEquals(L"score of " + id + L" shuould be " +
                     to_wstring(expectedScore) + L" != " + to_wstring(score),
                 expectedScore, score, TEST_SCORE_TOLERANCE_DELTA);
  }
  delete r;
}

shared_ptr<Query>
TestFieldScoreQuery::getFunctionQuery(shared_ptr<ValueSource> valueSource)
{
  if (random()->nextBoolean()) {
    return make_shared<FunctionQuery>(valueSource);
  } else {
    optional<int> lower =
        (random()->nextBoolean() ? nullptr : 1); // 1 is the lowest value
    optional<int> upper =
        (random()->nextBoolean() ? nullptr
                                 : N_DOCS); // N_DOCS is the highest value
    return make_shared<FunctionRangeQuery>(
        valueSource, lower, upper, true,
        true); // will match all docs based on the indexed data
  }
}
} // namespace org::apache::lucene::queries::function