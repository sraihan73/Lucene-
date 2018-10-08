using namespace std;

#include "TestSynonymQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSynonymQuery::testEquals()
{
  QueryUtils::checkEqual(make_shared<SynonymQuery>(),
                         make_shared<SynonymQuery>());
  QueryUtils::checkEqual(
      make_shared<SynonymQuery>(make_shared<Term>(L"foo", L"bar")),
      make_shared<SynonymQuery>(make_shared<Term>(L"foo", L"bar")));

  QueryUtils::checkEqual(
      make_shared<SynonymQuery>(make_shared<Term>(L"a", L"a"),
                                make_shared<Term>(L"a", L"b")),
      make_shared<SynonymQuery>(make_shared<Term>(L"a", L"b"),
                                make_shared<Term>(L"a", L"a")));
}

void TestSynonymQuery::testBogusParams()
{
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SynonymQuery>(make_shared<Term>(L"field1", L"a"),
                              make_shared<Term>(L"field2", L"b"));
  });
}

void TestSynonymQuery::testToString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Synonym()", (make_shared<SynonymQuery>())->toString());
  shared_ptr<Term> t1 = make_shared<Term>(L"foo", L"bar");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Synonym(foo:bar)",
               (make_shared<SynonymQuery>(t1))->toString());
  shared_ptr<Term> t2 = make_shared<Term>(L"foo", L"baz");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Synonym(foo:bar foo:baz)",
               (make_shared<SynonymQuery>(t1, t2))->toString());
}

void TestSynonymQuery::testScores() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"f", L"a", Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"f", L"b", Store::NO));
  for (int i = 0; i < 10; ++i) {
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<SynonymQuery> query = make_shared<SynonymQuery>(
      make_shared<Term>(L"f", L"a"), make_shared<Term>(L"f", L"b"));

  shared_ptr<TopDocs> topDocs = searcher->search(query, 20);
  assertEquals(11, topDocs->totalHits);
  // All docs must have the same score
  for (int i = 0; i < topDocs->scoreDocs.size(); ++i) {
    assertEquals(topDocs->scoreDocs[0]->score, topDocs->scoreDocs[i]->score,
                 0.0f);
  }

  delete reader;
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::search