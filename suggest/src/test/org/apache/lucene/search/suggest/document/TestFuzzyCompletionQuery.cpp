using namespace std;

#include "TestFuzzyCompletionQuery.h"

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
void TestFuzzyCompletionQuery::before() 
{
  dir = newDirectory();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after() throws Exception
void TestFuzzyCompletionQuery::after()  { delete dir; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFuzzyQuery() throws Exception
void TestFuzzyCompletionQuery::testFuzzyQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suggestion", 2));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"suaggestion", 4));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"ssuggestion", 1));
  iw->addDocument(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"sugfoo", 1));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<CompletionQuery> query = make_shared<FuzzyCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"sugg"));
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 4, false);
  assertSuggestions(suggest, make_shared<Entry>(L"suaggestion", 4 * 2),
                    make_shared<Entry>(L"suggestion", 2 * 3),
                    make_shared<Entry>(L"sugfoo", 1 * 3),
                    make_shared<Entry>(L"ssuggestion", 1 * 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFuzzyContextQuery() throws Exception
void TestFuzzyCompletionQuery::testFuzzyContextQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sduggestion", 1, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sudggestion", 1, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sugdgestion", 1, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggdestion", 1, L"type4"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<CompletionQuery> query =
      make_shared<ContextQuery>(make_shared<FuzzyCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugge")));
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest, make_shared<Entry>(L"suggestion", L"type4", 4),
                    make_shared<Entry>(L"suggdestion", L"type4", 4),
                    make_shared<Entry>(L"sugdgestion", L"type3", 3),
                    make_shared<Entry>(L"sudggestion", L"type2", 2),
                    make_shared<Entry>(L"sduggestion", L"type1", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFuzzyFilteredContextQuery() throws
// Exception
void TestFuzzyCompletionQuery::testFuzzyFilteredContextQuery() throw(
    runtime_error)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sduggestion", 1, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sudggestion", 1, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"sugdgestion", 1, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggdestion", 1, L"type4"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<CompletionQuery> fuzzyQuery = make_shared<FuzzyCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"sugge"));
  shared_ptr<ContextQuery> contextQuery = make_shared<ContextQuery>(fuzzyQuery);
  contextQuery->addContext(L"type1", 6);
  contextQuery->addContext(L"type3", 2);
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(contextQuery, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"sduggestion", L"type1", 1 * (1 + 6)),
                    make_shared<Entry>(L"sugdgestion", L"type3", 1 * (3 + 2)));

  reader->close();
  delete iw;
}
} // namespace org::apache::lucene::search::suggest::document