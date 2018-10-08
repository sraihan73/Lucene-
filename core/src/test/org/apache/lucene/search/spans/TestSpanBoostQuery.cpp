using namespace std;

#include "TestSpanBoostQuery.h"

namespace org::apache::lucene::search::spans
{
using Term = org::apache::lucene::index::Term;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSpanBoostQuery::testEquals()
{
  constexpr float boost = random()->nextFloat() * 3 - 1;
  shared_ptr<SpanTermQuery> q =
      make_shared<SpanTermQuery>(make_shared<Term>(L"foo", L"bar"));
  shared_ptr<SpanBoostQuery> q1 = make_shared<SpanBoostQuery>(q, boost);
  shared_ptr<SpanBoostQuery> q2 = make_shared<SpanBoostQuery>(q, boost);
  assertEquals(q1, q2);
  assertEquals(q1->getBoost(), q2->getBoost(), 0.0f);

  float boost2 = boost;
  while (boost == boost2) {
    boost2 = random()->nextFloat() * 3 - 1;
  }
  shared_ptr<SpanBoostQuery> q3 = make_shared<SpanBoostQuery>(q, boost2);
  assertFalse(q1->equals(q3));
  assertFalse(q1->hashCode() == q3->hashCode());
}

void TestSpanBoostQuery::testToString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"(foo:bar)^2.0",
      (make_shared<SpanBoostQuery>(
           make_shared<SpanTermQuery>(make_shared<Term>(L"foo", L"bar")), 2))
          ->toString());
  shared_ptr<SpanOrQuery> bq = make_shared<SpanOrQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(L"foo", L"bar")),
      make_shared<SpanTermQuery>(make_shared<Term>(L"foo", L"baz")));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"(spanOr([foo:bar, foo:baz]))^2.0",
               (make_shared<SpanBoostQuery>(bq, 2))->toString());
}
} // namespace org::apache::lucene::search::spans