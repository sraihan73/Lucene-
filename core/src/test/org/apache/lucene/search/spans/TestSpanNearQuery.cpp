using namespace std;

#include "TestSpanNearQuery.h"

namespace org::apache::lucene::search::spans
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSpanNearQuery::testHashcodeEquals()
{
  shared_ptr<SpanTermQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"foo"));
  shared_ptr<SpanTermQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"bar"));
  shared_ptr<SpanTermQuery> q3 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"baz"));

  shared_ptr<SpanNearQuery> near1 = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{q1, q2}, 10, true);
  shared_ptr<SpanNearQuery> near2 = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{q2, q3}, 10, true);
  QueryUtils::check(near1);
  QueryUtils::check(near2);
  QueryUtils::checkUnequal(near1, near2);
}

void TestSpanNearQuery::testDifferentField() 
{
  shared_ptr<SpanTermQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field1", L"foo"));
  shared_ptr<SpanTermQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field2", L"bar"));
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SpanNearQuery>(std::deque<std::shared_ptr<SpanQuery>>{q1, q2},
                               10, true);
  });
  assertTrue(expected.what()->contains(L"must have same field"));
}

void TestSpanNearQuery::testNoPositions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Field::Store::NO));
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> is = make_shared<IndexSearcher>(ir);
  shared_ptr<SpanTermQuery> query =
      make_shared<SpanTermQuery>(make_shared<Term>(L"foo", L"bar"));
  shared_ptr<SpanTermQuery> query2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"foo", L"baz"));

  shared_ptr<IllegalStateException> expected =
      expectThrows(IllegalStateException::typeid, [&]() {
        is->search(make_shared<SpanNearQuery>(
                       std::deque<std::shared_ptr<SpanQuery>>{query, query2},
                       10, true),
                   5);
      });
  assertTrue(
      expected->getMessage()->contains(L"was indexed without position data"));

  delete ir;
  delete dir;
}

void TestSpanNearQuery::testBuilder() 
{

  // Can't add subclauses from different fields
  expectThrows(invalid_argument::typeid, [&]() {
    SpanNearQuery::newOrderedNearQuery(L"field1")->addClause(
        make_shared<SpanTermQuery>(make_shared<Term>(L"field2", L"term")));
  });

  // Can't add gaps to unordered queries
  expectThrows(invalid_argument::typeid, [&]() {
    SpanNearQuery::newUnorderedNearQuery(L"field1")->addGap(1);
  });
}
} // namespace org::apache::lucene::search::spans