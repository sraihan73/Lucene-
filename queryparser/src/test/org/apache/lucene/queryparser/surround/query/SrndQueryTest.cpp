using namespace std;

#include "SrndQueryTest.h"

namespace org::apache::lucene::queryparser::surround::query
{
using QueryParser =
    org::apache::lucene::queryparser::surround::parser::QueryParser;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

void SrndQueryTest::checkEqualParsings(const wstring &s1,
                                       const wstring &s2) 
{
  wstring fieldName = L"foo";
  shared_ptr<BasicQueryFactory> qf = make_shared<BasicQueryFactory>(16);
  shared_ptr<Query> lq1, lq2;
  lq1 = QueryParser::parse(s1)->makeLuceneQueryField(fieldName, qf);
  lq2 = QueryParser::parse(s2)->makeLuceneQueryField(fieldName, qf);
  QueryUtils::checkEqual(lq1, lq2);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testHashEquals() throws Exception
void SrndQueryTest::testHashEquals() 
{
  // grab some sample queries from Test02Boolean and Test03Distance and
  // check there hashes and equals
  checkEqualParsings(L"word1 w word2", L" word1  w  word2 ");
  checkEqualParsings(L"2N(w1,w2,w3)", L" 2N(w1, w2 , w3)");
  checkEqualParsings(L"abc?", L" abc? ");
  checkEqualParsings(L"w*rd?", L" w*rd?");
}
} // namespace org::apache::lucene::queryparser::surround::query