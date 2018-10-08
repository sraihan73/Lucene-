using namespace std;

#include "TestRegexCompletionQuery.h"

namespace org::apache::lucene::search::suggest::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.Entry; import
//    static
//    org.apache.lucene.search.suggest.document.TestSuggestField.assertSuggestions;
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.iwcWithSuggestField;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void before() throws Exception
void TestRegexCompletionQuery::before() 
{
  dir = newDirectory();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after() throws Exception
void TestRegexCompletionQuery::after()  { delete dir; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRegexQuery() throws Exception
void TestRegexCompletionQuery::testRegexQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suggestion", 1));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"asuggestion", 2));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"ssuggestion", 3));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"wsuggestion", 4));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<RegexCompletionQuery> query = make_shared<RegexCompletionQuery>(
      make_shared<Term>(L"suggest_field", L"[a|w|s]s?ugg"));
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 4, false);
  assertSuggestions(suggest, make_shared<Entry>(L"wsuggestion", 4),
                    make_shared<Entry>(L"ssuggestion", 3),
                    make_shared<Entry>(L"asuggestion", 2),
                    make_shared<Entry>(L"suggestion", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptyRegexQuery() throws Exception
void TestRegexCompletionQuery::testEmptyRegexQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suggestion1", 1));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<RegexCompletionQuery> query = make_shared<RegexCompletionQuery>(
      make_shared<Term>(L"suggest_field", L""));

  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertEquals(0, suggest->scoreDocs.size());

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSimpleRegexContextQuery() throws
// Exception
void TestRegexCompletionQuery::testSimpleRegexContextQuery() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sduggestion", 5, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sudggestion", 4, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sugdgestion", 3, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggdestion", 2, L"type4"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<CompletionQuery> query = make_shared<RegexCompletionQuery>(
      make_shared<Term>(L"suggest_field", L"[a|s][d|u|s][u|d|g]"));
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest, make_shared<Entry>(L"sduggestion", L"type1", 5),
                    make_shared<Entry>(L"sudggestion", L"type2", 4),
                    make_shared<Entry>(L"sugdgestion", L"type3", 3),
                    make_shared<Entry>(L"suggdestion", L"type4", 2),
                    make_shared<Entry>(L"suggestion", L"type4", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRegexContextQueryWithBoost() throws
// Exception
void TestRegexCompletionQuery::testRegexContextQueryWithBoost() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sduggestion", 5, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sudggestion", 4, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sugdgestion", 3, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggdestion", 2, L"type4"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<CompletionQuery> query = make_shared<RegexCompletionQuery>(
      make_shared<Term>(L"suggest_field", L"[a|s][d|u|s][u|g]"));
  shared_ptr<ContextQuery> contextQuery = make_shared<ContextQuery>(query);
  contextQuery->addContext(L"type1", 6);
  contextQuery->addContext(L"type3", 7);
  contextQuery->addAllContexts();
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(contextQuery, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"sduggestion", L"type1", 5 * 6),
                    make_shared<Entry>(L"sugdgestion", L"type3", 3 * 7),
                    make_shared<Entry>(L"suggdestion", L"type4", 2),
                    make_shared<Entry>(L"suggestion", L"type4", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptyRegexContextQuery() throws
// Exception
void TestRegexCompletionQuery::testEmptyRegexContextQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion", 1, L"type"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<RegexCompletionQuery>(
          make_shared<Term>(L"suggest_field", L"")));
  query->addContext(L"type", 1);

  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertEquals(0, suggest->scoreDocs.size());

  reader->close();
  delete iw;
}
} // namespace org::apache::lucene::search::suggest::document