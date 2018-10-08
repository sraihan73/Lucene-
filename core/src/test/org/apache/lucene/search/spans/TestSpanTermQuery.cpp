using namespace std;

#include "TestSpanTermQuery.h"

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

void TestSpanTermQuery::testHashcodeEquals()
{
  shared_ptr<SpanTermQuery> q1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"foo"));
  shared_ptr<SpanTermQuery> q2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"bar"));
  QueryUtils::check(q1);
  QueryUtils::check(q2);
  QueryUtils::checkUnequal(q1, q2);
}

void TestSpanTermQuery::testNoPositions() 
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
  shared_ptr<IllegalStateException> expected = expectThrows(
      IllegalStateException::typeid, [&]() { is->search(query, 5); });
  assertTrue(
      expected->getMessage()->contains(L"was indexed without position data"));

  delete ir;
  delete dir;
}
} // namespace org::apache::lucene::search::spans