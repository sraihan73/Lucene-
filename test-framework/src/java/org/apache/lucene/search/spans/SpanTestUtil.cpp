using namespace std;

#include "SpanTestUtil.h"

namespace org::apache::lucene::search::spans
{
using Term = org::apache::lucene::index::Term;
using QueryUtils = org::apache::lucene::search::QueryUtils;
//    import static org.junit.Assert.*;

shared_ptr<SpanQuery> SpanTestUtil::spanQuery(shared_ptr<SpanQuery> query)
{
  QueryUtils::check(query);
  return make_shared<AssertingSpanQuery>(query);
}

shared_ptr<SpanQuery> SpanTestUtil::spanTermQuery(const wstring &field,
                                                  const wstring &term)
{
  return spanQuery(make_shared<SpanTermQuery>(make_shared<Term>(field, term)));
}

shared_ptr<SpanQuery> SpanTestUtil::spanOrQuery(const wstring &field,
                                                deque<wstring> &terms)
{
  std::deque<std::shared_ptr<SpanQuery>> subqueries(terms->length);
  for (int i = 0; i < terms->length; i++) {
    subqueries[i] = spanTermQuery(field, terms[i]);
  }
  return spanOrQuery(subqueries);
}

shared_ptr<SpanQuery> SpanTestUtil::spanOrQuery(deque<SpanQuery> &subqueries)
{
  return spanQuery(make_shared<SpanOrQuery>(subqueries));
}

shared_ptr<SpanQuery> SpanTestUtil::spanNotQuery(shared_ptr<SpanQuery> include,
                                                 shared_ptr<SpanQuery> exclude)
{
  return spanQuery(make_shared<SpanNotQuery>(include, exclude));
}

shared_ptr<SpanQuery> SpanTestUtil::spanNotQuery(shared_ptr<SpanQuery> include,
                                                 shared_ptr<SpanQuery> exclude,
                                                 int pre, int post)
{
  return spanQuery(make_shared<SpanNotQuery>(include, exclude, pre, post));
}

shared_ptr<SpanQuery> SpanTestUtil::spanFirstQuery(shared_ptr<SpanQuery> query,
                                                   int end)
{
  return spanQuery(make_shared<SpanFirstQuery>(query, end));
}

shared_ptr<SpanQuery>
SpanTestUtil::spanPositionRangeQuery(shared_ptr<SpanQuery> query, int start,
                                     int end)
{
  return spanQuery(make_shared<SpanPositionRangeQuery>(query, start, end));
}

shared_ptr<SpanQuery>
SpanTestUtil::spanContainingQuery(shared_ptr<SpanQuery> big,
                                  shared_ptr<SpanQuery> little)
{
  return spanQuery(make_shared<SpanContainingQuery>(big, little));
}

shared_ptr<SpanQuery>
SpanTestUtil::spanWithinQuery(shared_ptr<SpanQuery> big,
                              shared_ptr<SpanQuery> little)
{
  return spanQuery(make_shared<SpanWithinQuery>(big, little));
}

shared_ptr<SpanQuery> SpanTestUtil::spanNearOrderedQuery(const wstring &field,
                                                         int slop,
                                                         deque<wstring> &terms)
{
  std::deque<std::shared_ptr<SpanQuery>> subqueries(terms->length);
  for (int i = 0; i < terms->length; i++) {
    subqueries[i] = spanTermQuery(field, terms[i]);
  }
  return spanNearOrderedQuery(slop, subqueries);
}

shared_ptr<SpanQuery>
SpanTestUtil::spanNearOrderedQuery(int slop, deque<SpanQuery> &subqueries)
{
  return spanQuery(make_shared<SpanNearQuery>(subqueries, slop, true));
}

shared_ptr<SpanQuery>
SpanTestUtil::spanNearUnorderedQuery(const wstring &field, int slop,
                                     deque<wstring> &terms)
{
  shared_ptr<SpanNearQuery::Builder> builder =
      SpanNearQuery::newUnorderedNearQuery(field);
  builder->setSlop(slop);
  for (wstring term : terms) {
    builder->addClause(
        make_shared<SpanTermQuery>(make_shared<Term>(field, term)));
  }
  return spanQuery(builder->build());
}

shared_ptr<SpanQuery>
SpanTestUtil::spanNearUnorderedQuery(int slop, deque<SpanQuery> &subqueries)
{
  return spanQuery(make_shared<SpanNearQuery>(subqueries, slop, false));
}

void SpanTestUtil::assertNext(shared_ptr<Spans> spans, int doc, int start,
                              int end) 
{
  if (spans->docID() >= doc) {
    assertEquals(L"docId", doc, spans->docID());
  } else { // nextDoc needed before testing start/end
    if (spans->docID() >= 0) {
      assertEquals(L"nextStartPosition of previous doc",
                   Spans::NO_MORE_POSITIONS, spans->nextStartPosition());
      assertEquals(L"endPosition of previous doc", Spans::NO_MORE_POSITIONS,
                   spans->endPosition());
    }
    assertEquals(L"nextDoc", doc, spans->nextDoc());
    if (doc != Spans::NO_MORE_DOCS) {
      assertEquals(L"first startPosition", -1, spans->startPosition());
      assertEquals(L"first endPosition", -1, spans->endPosition());
    }
  }
  if (doc != Spans::NO_MORE_DOCS) {
    assertEquals(L"nextStartPosition", start, spans->nextStartPosition());
    assertEquals(L"startPosition", start, spans->startPosition());
    assertEquals(L"endPosition", end, spans->endPosition());
  }
}

void SpanTestUtil::assertFinished(shared_ptr<Spans> spans) 
{
  if (spans != nullptr) { // null Spans is empty
    assertNext(spans, Spans::NO_MORE_DOCS, -2,
               -2); // start and end positions will be ignored
  }
}
} // namespace org::apache::lucene::search::spans