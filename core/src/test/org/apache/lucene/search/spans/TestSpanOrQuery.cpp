using namespace std;

#include "TestSpanOrQuery.h"

namespace org::apache::lucene::search::spans
{
using Term = org::apache::lucene::index::Term;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSpanOrQuery::testHashcodeEquals()
{
  shared_ptr<SpanTermQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"foo"));
  shared_ptr<SpanTermQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"bar"));
  shared_ptr<SpanTermQuery> q3 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"baz"));

  shared_ptr<SpanOrQuery> or1 = make_shared<SpanOrQuery>(q1, q2);
  shared_ptr<SpanOrQuery> or2 = make_shared<SpanOrQuery>(q2, q3);
  QueryUtils::check(or1);
  QueryUtils::check(or2);
  QueryUtils::checkUnequal(or1, or2);
}

void TestSpanOrQuery::testSpanOrEmpty() 
{
  shared_ptr<SpanOrQuery> a = make_shared<SpanOrQuery>();
  shared_ptr<SpanOrQuery> b = make_shared<SpanOrQuery>();
  assertTrue(L"empty should equal", a->equals(b));
}

void TestSpanOrQuery::testDifferentField() 
{
  shared_ptr<SpanTermQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field1", L"foo"));
  shared_ptr<SpanTermQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field2", L"bar"));
  invalid_argument expected = expectThrows(
      invalid_argument::typeid, [&]() { make_shared<SpanOrQuery>(q1, q2); });
  assertTrue(expected.what()->contains(L"must have same field"));
}
} // namespace org::apache::lucene::search::spans