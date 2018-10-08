using namespace std;

#include "TestMatchNoDocsQuery.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMatchNoDocsQuery::setUp() 
{
  LuceneTestCase::setUp();
  analyzer = make_shared<MockAnalyzer>(random());
}

void TestMatchNoDocsQuery::testSimple() 
{
  shared_ptr<MatchNoDocsQuery> query = make_shared<MatchNoDocsQuery>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(query->toString(), L"MatchNoDocsQuery(\"\")");
  query = make_shared<MatchNoDocsQuery>(L"field 'title' not found");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(query->toString(),
               L"MatchNoDocsQuery(\"field 'title' not found\")");
  shared_ptr<Query> rewrite = query->rewrite(nullptr);
  assertTrue(std::dynamic_pointer_cast<MatchNoDocsQuery>(rewrite) != nullptr);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(rewrite->toString(),
               L"MatchNoDocsQuery(\"field 'title' not found\")");
}

void TestMatchNoDocsQuery::testQuery() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(2)->setMergePolicy(
          newLogMergePolicy()));
  addDoc(L"one", iw);
  addDoc(L"two", iw);
  addDoc(L"three", iw);
  shared_ptr<IndexReader> ir = DirectoryReader::open(iw);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(ir);

  shared_ptr<Query> query = make_shared<MatchNoDocsQuery>(L"field not found");
  assertEquals(searcher->count(query), 0);

  std::deque<std::shared_ptr<ScoreDoc>> hits;
  hits = searcher->search(make_shared<MatchNoDocsQuery>(), 1000)->scoreDocs;
  assertEquals(0, hits.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(query->toString(), L"MatchNoDocsQuery(\"field not found\")");

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"key", L"five")),
      BooleanClause::Occur::SHOULD));
  bq->add(make_shared<BooleanClause>(
      make_shared<MatchNoDocsQuery>(L"field not found"),
      BooleanClause::Occur::MUST));
  query = bq->build();
  assertEquals(searcher->count(query), 0);
  hits = searcher->search(make_shared<MatchNoDocsQuery>(), 1000)->scoreDocs;
  assertEquals(0, hits.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(query->toString(),
               L"key:five +MatchNoDocsQuery(\"field not found\")");

  bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"key", L"one")),
      BooleanClause::Occur::SHOULD));
  bq->add(make_shared<BooleanClause>(
      make_shared<MatchNoDocsQuery>(L"field not found"),
      BooleanClause::Occur::SHOULD));
  query = bq->build();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(query->toString(),
               L"key:one MatchNoDocsQuery(\"field not found\")");
  assertEquals(searcher->count(query), 1);
  hits = searcher->search(query, 1000)->scoreDocs;
  shared_ptr<Query> rewrite = query->rewrite(ir);
  assertEquals(1, hits.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(rewrite->toString(),
               L"key:one MatchNoDocsQuery(\"field not found\")");

  delete iw;
  delete ir;
  delete dir;
}

void TestMatchNoDocsQuery::testEquals()
{
  shared_ptr<Query> q1 = make_shared<MatchNoDocsQuery>();
  shared_ptr<Query> q2 = make_shared<MatchNoDocsQuery>();
  assertTrue(q1->equals(q2));
  QueryUtils::check(q1);
}

void TestMatchNoDocsQuery::addDoc(const wstring &text,
                                  shared_ptr<IndexWriter> iw) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"key", text, Field::Store::YES);
  doc->push_back(f);
  iw->addDocument(doc);
}
} // namespace org::apache::lucene::search