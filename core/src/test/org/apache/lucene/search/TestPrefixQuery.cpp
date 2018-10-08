using namespace std;

#include "TestPrefixQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPrefixQuery::testPrefixQuery() 
{
  shared_ptr<Directory> directory = newDirectory();

  std::deque<wstring> categories = {L"/Computers", L"/Computers/Mac",
                                     L"/Computers/Windows"};
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  for (int i = 0; i < categories.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"category", categories[i], Field::Store::YES));
    writer->addDocument(doc);
  }
  shared_ptr<IndexReader> reader = writer->getReader();

  shared_ptr<PrefixQuery> query =
      make_shared<PrefixQuery>(make_shared<Term>(L"category", L"/Computers"));
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"All documents in /Computers category and below", 3,
               hits.size());

  query = make_shared<PrefixQuery>(
      make_shared<Term>(L"category", L"/Computers/Mac"));
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"One in /Computers/Mac", 1, hits.size());

  query = make_shared<PrefixQuery>(make_shared<Term>(L"category", L""));
  hits = searcher->search(query, 1000)->scoreDocs;
  assertEquals(L"everything", 3, hits.size());
  delete writer;
  delete reader;
  delete directory;
}

void TestPrefixQuery::testMatchAll() 
{
  shared_ptr<Directory> directory = newDirectory();

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"field", Field::Store::YES));
  writer->addDocument(doc);

  shared_ptr<IndexReader> reader = writer->getReader();

  shared_ptr<PrefixQuery> query =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L""));
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  TestUtil::assertEquals(1, searcher->search(query, 1000)->totalHits);
  delete writer;
  delete reader;
  delete directory;
}

void TestPrefixQuery::testRandomBinaryPrefix() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);

  int numTerms = atLeast(10000);
  shared_ptr<Set<std::shared_ptr<BytesRef>>> terms =
      unordered_set<std::shared_ptr<BytesRef>>();
  while (terms->size() < numTerms) {
    std::deque<char> bytes(TestUtil::nextInt(random(), 1, 10));
    random()->nextBytes(bytes);
    terms->add(make_shared<BytesRef>(bytes));
  }

  deque<std::shared_ptr<BytesRef>> termsList =
      deque<std::shared_ptr<BytesRef>>(terms);
  Collections::shuffle(termsList, random());
  for (auto term : termsList) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"field", term, Field::Store::NO));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    std::deque<char> bytes(random()->nextInt(3));
    random()->nextBytes(bytes);
    shared_ptr<BytesRef> prefix = make_shared<BytesRef>(bytes);
    shared_ptr<PrefixQuery> q =
        make_shared<PrefixQuery>(make_shared<Term>(L"field", prefix));
    int count = 0;
    for (auto term : termsList) {
      if (StringHelper::startsWith(term, prefix)) {
        count++;
      }
    }
    TestUtil::assertEquals(count, s->search(q, 1)->totalHits);
  }
  delete r;
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::search