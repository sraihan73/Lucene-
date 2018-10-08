using namespace std;

#include "TestNot.h"

namespace org::apache::lucene::search
{
using Field = org::apache::lucene::document::Field;
using Term = org::apache::lucene::index::Term;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using Document = org::apache::lucene::document::Document;

void TestNot::testNot() 
{
  shared_ptr<Directory> store = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), store);

  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(newTextField(L"field", L"a b", Field::Store::YES));

  writer->addDocument(d1);
  shared_ptr<IndexReader> reader = writer->getReader();

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"b")),
             BooleanClause::Occur::MUST_NOT);

  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query->build(), 1000)->scoreDocs;
  assertEquals(0, hits.size());
  delete writer;
  delete reader;
  delete store;
}
} // namespace org::apache::lucene::search