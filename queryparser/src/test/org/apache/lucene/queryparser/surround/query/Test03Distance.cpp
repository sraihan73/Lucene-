using namespace std;

#include "Test03Distance.h"

namespace org::apache::lucene::queryparser::surround::query
{
using junit::framework::TestSuite;
using junit::textui::TestRunner;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void Test03Distance::main(std::deque<wstring> &args)
{
  TestRunner::run(make_shared<TestSuite>(Test03Distance::typeid));
}

void Test03Distance::test00Exceptions() 
{
  wstring m = ExceptionQueryTst::getFailQueries(exceptionQueries, verbose);
  if (m.length() > 0) {
    fail(L"No ParseException for:\n" + m);
  }
}

void Test03Distance::setUp() 
{
  LuceneTestCase::setUp();
  db1 = make_shared<SingleFieldTestDb>(random(), docs1, fieldName);
  db2 = make_shared<SingleFieldTestDb>(random(), docs2, fieldName);
  db3 = make_shared<SingleFieldTestDb>(random(), docs3, fieldName);
}

void Test03Distance::distanceTst(
    const wstring &query, std::deque<int> &expdnrs,
    shared_ptr<SingleFieldTestDb> db) 
{
  shared_ptr<BooleanQueryTst> bqt = make_shared<BooleanQueryTst>(
      query, expdnrs, db, fieldName, shared_from_this(),
      make_shared<BasicQueryFactory>(maxBasicQueries));
  bqt->setVerbose(verbose);
  bqt->doTest();
}

void Test03Distance::distanceTest1(
    const wstring &query, std::deque<int> &expdnrs) 
{
  distanceTst(query, expdnrs, db1);
}

void Test03Distance::test0W01() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word1 w word2", expdnrs);
}

void Test03Distance::test0N01() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word1 n word2", expdnrs);
}

void Test03Distance::test0N01r() 
{ // r reverse
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word2 n word1", expdnrs);
}

void Test03Distance::test0W02() 
{
  std::deque<int> expdnrs = {};
  distanceTest1(L"word2 w word1", expdnrs);
}

void Test03Distance::test0W03() 
{
  std::deque<int> expdnrs = {};
  distanceTest1(L"word2 2W word1", expdnrs);
}

void Test03Distance::test0N03() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word2 2N word1", expdnrs);
}

void Test03Distance::test0N03r() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word1 2N word2", expdnrs);
}

void Test03Distance::test0W04() 
{
  std::deque<int> expdnrs = {};
  distanceTest1(L"word2 3w word1", expdnrs);
}

void Test03Distance::test0N04() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word2 3n word1", expdnrs);
}

void Test03Distance::test0N04r() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word1 3n word2", expdnrs);
}

void Test03Distance::test0W05() 
{
  std::deque<int> expdnrs = {};
  distanceTest1(L"orda1 w orda3", expdnrs);
}

void Test03Distance::test0W06() 
{
  std::deque<int> expdnrs = {3};
  distanceTest1(L"orda1 2w orda3", expdnrs);
}

void Test03Distance::test1Wtrunc01() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word1* w word2", expdnrs);
}

void Test03Distance::test1Wtrunc02() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word* w word2", expdnrs);
}

void Test03Distance::test1Wtrunc02r() 
{
  std::deque<int> expdnrs = {0, 3};
  distanceTest1(L"word2 w word*", expdnrs);
}

void Test03Distance::test1Ntrunc02() 
{
  std::deque<int> expdnrs = {0, 3};
  distanceTest1(L"word* n word2", expdnrs);
}

void Test03Distance::test1Ntrunc02r() 
{
  std::deque<int> expdnrs = {0, 3};
  distanceTest1(L"word2 n word*", expdnrs);
}

void Test03Distance::test1Wtrunc03() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word1* w word2*", expdnrs);
}

void Test03Distance::test1Ntrunc03() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word1* N word2*", expdnrs);
}

void Test03Distance::test1Wtrunc04() 
{
  std::deque<int> expdnrs = {};
  distanceTest1(L"kxork* w kxor*", expdnrs);
}

void Test03Distance::test1Ntrunc04() 
{
  std::deque<int> expdnrs = {};
  distanceTest1(L"kxork* 99n kxor*", expdnrs);
}

void Test03Distance::test1Wtrunc05() 
{
  std::deque<int> expdnrs = {};
  distanceTest1(L"word2* 2W word1*", expdnrs);
}

void Test03Distance::test1Ntrunc05() 
{
  std::deque<int> expdnrs = {0};
  distanceTest1(L"word2* 2N word1*", expdnrs);
}

void Test03Distance::test1Wtrunc06() 
{
  std::deque<int> expdnrs = {3};
  distanceTest1(L"ord* W word*", expdnrs);
}

void Test03Distance::test1Ntrunc06() 
{
  std::deque<int> expdnrs = {3};
  distanceTest1(L"ord* N word*", expdnrs);
}

void Test03Distance::test1Ntrunc06r() 
{
  std::deque<int> expdnrs = {3};
  distanceTest1(L"word* N ord*", expdnrs);
}

void Test03Distance::test1Wtrunc07() 
{
  std::deque<int> expdnrs = {3};
  distanceTest1(L"(orda2 OR orda3) W word*", expdnrs);
}

void Test03Distance::test1Wtrunc08() 
{
  std::deque<int> expdnrs = {3};
  distanceTest1(L"(orda2 OR orda3) W (word2 OR worda3)", expdnrs);
}

void Test03Distance::test1Wtrunc09() 
{
  std::deque<int> expdnrs = {3};
  distanceTest1(L"(orda2 OR orda3) 2W (word2 OR worda3)", expdnrs);
}

void Test03Distance::test1Ntrunc09() 
{
  std::deque<int> expdnrs = {3};
  distanceTest1(L"(orda2 OR orda3) 2N (word2 OR worda3)", expdnrs);
}

void Test03Distance::distanceTest2(
    const wstring &query, std::deque<int> &expdnrs) 
{
  distanceTst(query, expdnrs, db2);
}

void Test03Distance::test2Wprefix01() 
{
  std::deque<int> expdnrs = {0};
  distanceTest2(L"W (w1, w2, w3)", expdnrs);
}

void Test03Distance::test2Nprefix01a() 
{
  std::deque<int> expdnrs = {0, 1};
  distanceTest2(L"N(w1, w2, w3)", expdnrs);
}

void Test03Distance::test2Nprefix01b() 
{
  std::deque<int> expdnrs = {0, 1};
  distanceTest2(L"N(w3, w1, w2)", expdnrs);
}

void Test03Distance::test2Wprefix02() 
{
  std::deque<int> expdnrs = {0, 1};
  distanceTest2(L"2W(w1,w2,w3)", expdnrs);
}

void Test03Distance::test2Nprefix02a() 
{
  std::deque<int> expdnrs = {0, 1};
  distanceTest2(L"2N(w1,w2,w3)", expdnrs);
}

void Test03Distance::test2Nprefix02b() 
{
  std::deque<int> expdnrs = {0, 1};
  distanceTest2(L"2N(w2,w3,w1)", expdnrs);
}

void Test03Distance::test2Wnested01() 
{
  std::deque<int> expdnrs = {0};
  distanceTest2(L"w1 W w2 W w3", expdnrs);
}

void Test03Distance::test2Nnested01() 
{
  std::deque<int> expdnrs = {0};
  distanceTest2(L"w1 N w2 N w3", expdnrs);
}

void Test03Distance::test2Wnested02() 
{
  std::deque<int> expdnrs = {0, 1};
  distanceTest2(L"w1 2W w2 2W w3", expdnrs);
}

void Test03Distance::test2Nnested02() 
{
  std::deque<int> expdnrs = {0, 1};
  distanceTest2(L"w1 2N w2 2N w3", expdnrs);
}

void Test03Distance::distanceTest3(
    const wstring &query, std::deque<int> &expdnrs) 
{
  distanceTst(query, expdnrs, db3);
}

void Test03Distance::test3Example01() 
{
  std::deque<int> expdnrs = {0, 2}; // query does not match doc 1 because
                                     // "gradient" is in wrong place there.
  distanceTest3(wstring(L"50n((low w pressure*) or depression*,") +
                    L"5n(temperat*, (invers* or (negativ* 3n gradient*)))," +
                    L"rain* or precipitat*)",
                expdnrs);
}
} // namespace org::apache::lucene::queryparser::surround::query