using namespace std;

#include "TestExpressionRescorer.h"

namespace org::apache::lucene::expressions
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Rescorer = org::apache::lucene::search::Rescorer;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestExpressionRescorer::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig()->setSimilarity(make_shared<ClassicSimilarity>()));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  doc->push_back(newTextField(L"body", L"some contents and more contents",
                              Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 5));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  doc->push_back(newTextField(
      L"body", L"another document with different contents", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 20));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  doc->push_back(newTextField(L"body", L"crappy contents", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 2));
  iw->addDocument(doc);

  reader = iw->getReader();
  searcher = make_shared<IndexSearcher>(reader);
  // TODO: fix this test to not be so flaky and use newSearcher
  searcher->setSimilarity(make_shared<ClassicSimilarity>());
  delete iw;
}

void TestExpressionRescorer::tearDown() 
{
  reader->close();
  delete dir;
  LuceneTestCase::tearDown();
}

void TestExpressionRescorer::testBasic() 
{

  // create a sort field and sort by it (reverse order)
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"contents"));
  shared_ptr<IndexReader> r = searcher->getIndexReader();

  // Just first pass query
  shared_ptr<TopDocs> hits = searcher->search(query, 10);
  assertEquals(3, hits->totalHits);
  assertEquals(L"3", r->document(hits->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", r->document(hits->scoreDocs[1]->doc)[L"id"]);
  assertEquals(L"2", r->document(hits->scoreDocs[2]->doc)[L"id"]);

  // Now, rescore:

  shared_ptr<Expression> e =
      JavascriptCompiler::compile(L"sqrt(_score) + ln(popularity)");
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"popularity", SortField::Type::INT));
  bindings->add(make_shared<SortField>(L"_score", SortField::Type::SCORE));
  shared_ptr<Rescorer> rescorer = e->getRescorer(bindings);

  hits = rescorer->rescore(searcher, hits, 10);
  assertEquals(3, hits->totalHits);
  assertEquals(L"2", r->document(hits->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", r->document(hits->scoreDocs[1]->doc)[L"id"]);
  assertEquals(L"3", r->document(hits->scoreDocs[2]->doc)[L"id"]);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring expl =
      rescorer
          ->explain(searcher, searcher->explain(query, hits->scoreDocs[0]->doc),
                    hits->scoreDocs[0]->doc)
          ->toString();

  // Confirm the explanation breaks out the individual
  // variables:
  assertTrue(expl.find(L"= double(popularity)") != wstring::npos);

  // Confirm the explanation includes first pass details:
  assertTrue(expl.find(L"= first pass score") != wstring::npos);
  assertTrue(expl.find(L"body:contents in") != wstring::npos);
}
} // namespace org::apache::lucene::expressions