using namespace std;

#include "TestMatchAllDocsQuery.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMatchAllDocsQuery::setUp() 
{
  LuceneTestCase::setUp();
  analyzer = make_shared<MockAnalyzer>(random());
}

void TestMatchAllDocsQuery::testQuery() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(2)->setMergePolicy(
          newLogMergePolicy()));
  addDoc(L"one", iw);
  addDoc(L"two", iw);
  addDoc(L"three four", iw);
  shared_ptr<IndexReader> ir = DirectoryReader::open(iw);

  shared_ptr<IndexSearcher> is = newSearcher(ir);
  std::deque<std::shared_ptr<ScoreDoc>> hits;

  hits = is->search(make_shared<MatchAllDocsQuery>(), 1000)->scoreDocs;
  assertEquals(3, hits.size());
  assertEquals(L"one", is->doc(hits[0]->doc)[L"key"]);
  assertEquals(L"two", is->doc(hits[1]->doc)[L"key"]);
  assertEquals(L"three four", is->doc(hits[2]->doc)[L"key"]);

  // some artificial queries to trigger the use of skipTo():

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::MUST);
  bq->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::MUST);
  hits = is->search(bq->build(), 1000)->scoreDocs;
  assertEquals(3, hits.size());

  bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::MUST);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"key", L"three")),
          BooleanClause::Occur::MUST);
  hits = is->search(bq->build(), 1000)->scoreDocs;
  assertEquals(1, hits.size());

  iw->deleteDocuments({make_shared<Term>(L"key", L"one")});
  delete ir;
  ir = DirectoryReader::open(iw);
  is = newSearcher(ir);

  hits = is->search(make_shared<MatchAllDocsQuery>(), 1000)->scoreDocs;
  assertEquals(2, hits.size());

  delete iw;
  delete ir;
  delete dir;
}

void TestMatchAllDocsQuery::testEquals()
{
  shared_ptr<Query> q1 = make_shared<MatchAllDocsQuery>();
  shared_ptr<Query> q2 = make_shared<MatchAllDocsQuery>();
  assertTrue(q1->equals(q2));
}

void TestMatchAllDocsQuery::addDoc(
    const wstring &text, shared_ptr<IndexWriter> iw) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"key", text, Field::Store::YES);
  doc->push_back(f);
  iw->addDocument(doc);
}
} // namespace org::apache::lucene::search