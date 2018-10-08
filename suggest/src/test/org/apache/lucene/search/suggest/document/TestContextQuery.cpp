using namespace std;

#include "TestContextQuery.h"

namespace org::apache::lucene::search::suggest::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
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
void TestContextQuery::before() ; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after() throws Exception
void TestContextQuery::after()  { delete dir; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testIllegalInnerQuery() throws Exception
void TestContextQuery::testIllegalInnerQuery() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ContextQuery>(
        make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
            make_shared<MockAnalyzer>(random()),
            make_shared<Term>(L"suggest_field", L"sugg"))));
  });
  assertTrue(expected.what()->contains(ContextQuery::typeid->getSimpleName()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSimpleContextQuery() throws Exception
void TestContextQuery::testSimpleContextQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion1", 8, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion2", 7, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion3", 6, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion4", 5, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  query->addContext(L"type1", 1);
  query->addContext(L"type2", 2);
  query->addContext(L"type3", 3);
  query->addContext(L"type4", 4);
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"suggestion4", L"type4", 5 * 4),
                    make_shared<Entry>(L"suggestion3", L"type3", 6 * 3),
                    make_shared<Entry>(L"suggestion2", L"type2", 7 * 2),
                    make_shared<Entry>(L"suggestion1", L"type1", 8 * 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testContextQueryOnSuggestField() throws
// Exception
void TestContextQuery::testContextQueryOnSuggestField() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<SuggestField>(L"suggest_field", L"abc", 3));
  document->push_back(make_shared<SuggestField>(L"suggest_field", L"abd", 4));
  document->push_back(
      make_shared<SuggestField>(L"suggest_field", L"The Foo Fighters", 2));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<SuggestField>(L"suggest_field", L"abcdd", 5));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"ab")));
  shared_ptr<IllegalStateException> expected =
      expectThrows(IllegalStateException::typeid,
                   [&]() { suggestIndexSearcher->suggest(query, 4, false); });
  assertTrue(expected->getMessage()->contains(L"SuggestField"));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNonExactContextQuery() throws Exception
void TestContextQuery::testNonExactContextQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion1", 4, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion2", 3, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion3", 2, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion4", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  query->addContext(L"type", 1, false);
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest, make_shared<Entry>(L"suggestion1", L"type1", 4),
                    make_shared<Entry>(L"suggestion2", L"type2", 3),
                    make_shared<Entry>(L"suggestion3", L"type3", 2),
                    make_shared<Entry>(L"suggestion4", L"type4", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testContextPrecedenceBoost() throws
// Exception
void TestContextQuery::testContextPrecedenceBoost() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion1", 4, L"typetype"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion2", 3, L"type"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  query->addContext(L"type", 1);
  query->addContext(L"typetype", 2);
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"suggestion1", L"typetype", 4 * 2),
                    make_shared<Entry>(L"suggestion2", L"type", 3 * 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptyContext() throws Exception
void TestContextQuery::testEmptyContext() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion_no_ctx", 4));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"suggestion_no_ctx", nullptr, 4),
                    make_shared<Entry>(L"suggestion", L"type4", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptyContextWithBoosts() throws
// Exception
void TestContextQuery::testEmptyContextWithBoosts() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(
      make_shared<ContextSuggestField>(L"suggest_field", L"suggestion1", 4));
  document->push_back(
      make_shared<ContextSuggestField>(L"suggest_field", L"suggestion2", 3));
  document->push_back(
      make_shared<ContextSuggestField>(L"suggest_field", L"suggestion3", 2));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion4", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);

  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  query->addContext(L"type4", 10);
  query->addAllContexts();
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"suggestion4", L"type4", 1 * 10),
                    make_shared<Entry>(L"suggestion1", nullptr, 4),
                    make_shared<Entry>(L"suggestion2", nullptr, 3),
                    make_shared<Entry>(L"suggestion3", nullptr, 2));
  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSameSuggestionMultipleContext() throws
// Exception
void TestContextQuery::testSameSuggestionMultipleContext() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion", 4, L"type1", L"type2", L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  query->addContext(L"type1", 10);
  query->addContext(L"type2", 2);
  query->addContext(L"type3", 3);
  query->addContext(L"type4", 4);
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"suggestion", L"type1", 4 * 10),
                    make_shared<Entry>(L"suggestion", L"type3", 4 * 3),
                    make_shared<Entry>(L"suggestion", L"type2", 4 * 2),
                    make_shared<Entry>(L"suggestion", L"type4", 1 * 4));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMixedContextQuery() throws Exception
void TestContextQuery::testMixedContextQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion1", 4, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion2", 3, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion3", 2, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion4", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  query->addContext(L"type1", 7);
  query->addContext(L"type2", 6);
  query->addAllContexts();
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"suggestion1", L"type1", 4 * 7),
                    make_shared<Entry>(L"suggestion2", L"type2", 3 * 6),
                    make_shared<Entry>(L"suggestion3", L"type3", 2),
                    make_shared<Entry>(L"suggestion4", L"type4", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFilteringContextQuery() throws Exception
void TestContextQuery::testFilteringContextQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion1", 4, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion2", 3, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion3", 2, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion4", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  query->addContext(L"type3", 3);
  query->addContext(L"type4", 4);
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"suggestion3", L"type3", 2 * 3),
                    make_shared<Entry>(L"suggestion4", L"type4", 1 * 4));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testContextQueryRewrite() throws Exception
void TestContextQuery::testContextQueryRewrite() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion1", 4, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion2", 3, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion3", 2, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion4", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<CompletionQuery> query = make_shared<PrefixCompletionQuery>(
      analyzer, make_shared<Term>(L"suggest_field", L"sugg"));
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest, make_shared<Entry>(L"suggestion1", L"type1", 4),
                    make_shared<Entry>(L"suggestion2", L"type2", 3),
                    make_shared<Entry>(L"suggestion3", L"type3", 2),
                    make_shared<Entry>(L"suggestion4", L"type4", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultiContextQuery() throws Exception
void TestContextQuery::testMultiContextQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion1", 8, L"type1", L"type3"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion2", 7, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion3", 6, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion4", 5, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  query->addContext(L"type1", 1);
  query->addContext(L"type2", 2);
  query->addContext(L"type3", 3);
  query->addContext(L"type4", 4);
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 5, false);
  assertSuggestions(suggest,
                    make_shared<Entry>(L"suggestion1", L"type3", 8 * 3),
                    make_shared<Entry>(L"suggestion4", L"type4", 5 * 4),
                    make_shared<Entry>(L"suggestion3", L"type3", 6 * 3),
                    make_shared<Entry>(L"suggestion2", L"type2", 7 * 2),
                    make_shared<Entry>(L"suggestion1", L"type1", 8 * 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAllContextQuery() throws Exception
void TestContextQuery::testAllContextQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
  shared_ptr<Document> document = make_shared<Document>();

  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion1", 4, L"type1"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion2", 3, L"type2"));
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion3", 2, L"type3"));
  iw->addDocument(document);

  document = make_shared<Document>();
  document->push_back(make_shared<ContextSuggestField>(
      L"suggest_field", L"suggestion4", 1, L"type4"));
  iw->addDocument(document);

  if (rarely()) {
    iw->commit();
  }

  shared_ptr<DirectoryReader> reader = iw->getReader();
  shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
      make_shared<SuggestIndexSearcher>(reader);
  shared_ptr<ContextQuery> query =
      make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
          analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
  shared_ptr<TopSuggestDocs> suggest =
      suggestIndexSearcher->suggest(query, 4, false);
  assertSuggestions(suggest, make_shared<Entry>(L"suggestion1", L"type1", 4),
                    make_shared<Entry>(L"suggestion2", L"type2", 3),
                    make_shared<Entry>(L"suggestion3", L"type3", 2),
                    make_shared<Entry>(L"suggestion4", L"type4", 1));

  reader->close();
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRandomContextQueryScoring() throws
// Exception
void TestContextQuery::testRandomContextQueryScoring() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(org.apache.lucene.index.RandomIndexWriter iw
  // = new org.apache.lucene.index.RandomIndexWriter(random(), dir,
  // iwcWithSuggestField(analyzer, "suggest_field")))
  {
    org::apache::lucene::index::RandomIndexWriter iw =
        org::apache::lucene::index::RandomIndexWriter(
            random(), dir, iwcWithSuggestField(analyzer, L"suggest_field"));
    int numSuggestions = atLeast(20);
    int numContexts = atLeast(5);

    shared_ptr<Set<int>> seenWeights = unordered_set<int>();
    deque<std::shared_ptr<Entry>> expectedEntries =
        deque<std::shared_ptr<Entry>>();
    deque<std::shared_ptr<std::wstring>> contexts =
        deque<std::shared_ptr<std::wstring>>();
    for (int i = 1; i <= numContexts; i++) {
      shared_ptr<std::wstring> context =
          TestUtil::randomSimpleString(random(), 10) + to_wstring(i);
      contexts.push_back(context);
      for (int j = 1; j <= numSuggestions; j++) {
        wstring suggestion = L"sugg_" +
                             TestUtil::randomSimpleString(random(), 10) +
                             to_wstring(j);
        int weight =
            TestUtil::nextInt(random(), 1, 1000 * numContexts * numSuggestions);
        while (seenWeights->contains(weight)) {
          weight = TestUtil::nextInt(random(), 1,
                                     1000 * numContexts * numSuggestions);
        }
        seenWeights->add(weight);
        shared_ptr<Document> document = make_shared<Document>();
        document->push_back(make_shared<ContextSuggestField>(
            L"suggest_field", suggestion, weight, context));
        iw->addDocument(document);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        expectedEntries.push_back(
            make_shared<Entry>(suggestion, context->toString(), i * weight));
      }
      if (rarely()) {
        iw->commit();
      }
    }
    std::deque<std::shared_ptr<Entry>> expectedResults =
        expectedEntries.toArray(
            std::deque<std::shared_ptr<Entry>>(expectedEntries.size()));

    ArrayUtil::introSort(
        expectedResults,
        make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try(org.apache.lucene.index.DirectoryReader
    // reader = iw.getReader())
    {
      org::apache::lucene::index::DirectoryReader reader = iw->getReader();
      shared_ptr<SuggestIndexSearcher> suggestIndexSearcher =
          make_shared<SuggestIndexSearcher>(reader);
      shared_ptr<ContextQuery> query =
          make_shared<ContextQuery>(make_shared<PrefixCompletionQuery>(
              analyzer, make_shared<Term>(L"suggest_field", L"sugg")));
      for (int i = 0; i < contexts.size(); i++) {
        query->addContext(contexts[i], i + 1);
      }
      shared_ptr<TopSuggestDocs> suggest =
          suggestIndexSearcher->suggest(query, 4, false);
      assertSuggestions(suggest, Arrays::copyOfRange(expectedResults, 0, 4));
    }
  }
}

TestContextQuery::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<TestContextQuery> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestContextQuery::ComparatorAnonymousInnerClass::compare(
    shared_ptr<Entry> o1, shared_ptr<Entry> o2)
{
  int cmp = Float::compare(o2->value, o1->value);
  if (cmp != 0) {
    return cmp;
  } else {
    return o1->output->compareTo(o2->output);
  }
}
} // namespace org::apache::lucene::search::suggest::document