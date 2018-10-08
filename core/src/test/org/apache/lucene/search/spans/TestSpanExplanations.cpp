using namespace std;

#include "TestSpanExplanations.h"

namespace org::apache::lucene::search::spans
{
using namespace org::apache::lucene::search;

void TestSpanExplanations::testST1() 
{
  shared_ptr<SpanQuery> q = st(L"w1");
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testST2() 
{
  shared_ptr<SpanQuery> q = st(L"w1");
  qtest(make_shared<BoostQuery>(q, 1000), std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testST4() 
{
  shared_ptr<SpanQuery> q = st(L"xx");
  qtest(q, std::deque<int>{2, 3});
}

void TestSpanExplanations::testST5() 
{
  shared_ptr<SpanQuery> q = st(L"xx");
  qtest(make_shared<BoostQuery>(q, 1000), std::deque<int>{2, 3});
}

void TestSpanExplanations::testSF1() 
{
  shared_ptr<SpanQuery> q = sf((L"w1"), 1);
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testSF2() 
{
  shared_ptr<SpanQuery> q = sf((L"w1"), 1);
  qtest(make_shared<BoostQuery>(q, 1000), std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testSF4() 
{
  shared_ptr<SpanQuery> q = sf((L"xx"), 2);
  qtest(q, std::deque<int>{2});
}

void TestSpanExplanations::testSF5() 
{
  shared_ptr<SpanQuery> q = sf((L"yy"), 2);
  qtest(q, std::deque<int>());
}

void TestSpanExplanations::testSF6() 
{
  shared_ptr<SpanQuery> q = sf((L"yy"), 4);
  qtest(make_shared<BoostQuery>(q, 1000), std::deque<int>{2});
}

void TestSpanExplanations::testSO1() 
{
  shared_ptr<SpanQuery> q = sor(L"w1", L"QQ");
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testSO2() 
{
  shared_ptr<SpanQuery> q = sor(L"w1", L"w3", L"zz");
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testSO3() 
{
  shared_ptr<SpanQuery> q = sor(L"w5", L"QQ", L"yy");
  qtest(q, std::deque<int>{0, 2, 3});
}

void TestSpanExplanations::testSO4() 
{
  shared_ptr<SpanQuery> q = sor(L"w5", L"QQ", L"yy");
  qtest(q, std::deque<int>{0, 2, 3});
}

void TestSpanExplanations::testSNear1() 
{
  shared_ptr<SpanQuery> q = snear(L"w1", L"QQ", 100, true);
  qtest(q, std::deque<int>());
}

void TestSpanExplanations::testSNear2() 
{
  shared_ptr<SpanQuery> q = snear(L"w1", L"xx", 100, true);
  qtest(q, std::deque<int>{2, 3});
}

void TestSpanExplanations::testSNear3() 
{
  shared_ptr<SpanQuery> q = snear(L"w1", L"xx", 0, true);
  qtest(q, std::deque<int>{2});
}

void TestSpanExplanations::testSNear4() 
{
  shared_ptr<SpanQuery> q = snear(L"w1", L"xx", 1, true);
  qtest(q, std::deque<int>{2, 3});
}

void TestSpanExplanations::testSNear5() 
{
  shared_ptr<SpanQuery> q = snear(L"xx", L"w1", 0, false);
  qtest(q, std::deque<int>{2});
}

void TestSpanExplanations::testSNear6() 
{
  shared_ptr<SpanQuery> q = snear(L"w1", L"w2", L"QQ", 100, true);
  qtest(q, std::deque<int>());
}

void TestSpanExplanations::testSNear7() 
{
  shared_ptr<SpanQuery> q = snear(L"w1", L"xx", L"w2", 100, true);
  qtest(q, std::deque<int>{2, 3});
}

void TestSpanExplanations::testSNear8() 
{
  shared_ptr<SpanQuery> q = snear(L"w1", L"xx", L"w2", 0, true);
  qtest(q, std::deque<int>{2});
}

void TestSpanExplanations::testSNear9() 
{
  shared_ptr<SpanQuery> q = snear(L"w1", L"xx", L"w2", 1, true);
  qtest(q, std::deque<int>{2, 3});
}

void TestSpanExplanations::testSNear10() 
{
  shared_ptr<SpanQuery> q = snear(L"xx", L"w1", L"w2", 0, false);
  qtest(q, std::deque<int>{2});
}

void TestSpanExplanations::testSNear11() 
{
  shared_ptr<SpanQuery> q = snear(L"w1", L"w2", L"w3", 1, true);
  qtest(q, std::deque<int>{0, 1});
}

void TestSpanExplanations::testSNot1() 
{
  shared_ptr<SpanQuery> q = snot(sf(L"w1", 10), st(L"QQ"));
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testSNot2() 
{
  shared_ptr<SpanQuery> q = snot(sf(L"w1", 10), st(L"QQ"));
  qtest(make_shared<BoostQuery>(q, 1000), std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testSNot4() 
{
  shared_ptr<SpanQuery> q = snot(sf(L"w1", 10), st(L"xx"));
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testSNot5() 
{
  shared_ptr<SpanQuery> q = snot(sf(L"w1", 10), st(L"xx"));
  qtest(make_shared<BoostQuery>(q, 1000), std::deque<int>{0, 1, 2, 3});
}

void TestSpanExplanations::testSNot7() 
{
  shared_ptr<SpanQuery> f = snear(L"w1", L"w3", 10, true);
  shared_ptr<SpanQuery> q = snot(f, st(L"xx"));
  qtest(q, std::deque<int>{0, 1, 3});
}

void TestSpanExplanations::testSNot10() 
{
  shared_ptr<SpanQuery> t = st(L"xx");
  shared_ptr<SpanQuery> q = snot(snear(L"w1", L"w3", 10, true), t);
  qtest(q, std::deque<int>{0, 1, 3});
}
} // namespace org::apache::lucene::search::spans