using namespace std;

#include "TestUnifiedHighlighterMTQ.h"

namespace org::apache::lucene::search::uhighlight
{
using com::carrotsearch::randomizedtesting::annotations::ParametersFactory;
using com::carrotsearch::randomizedtesting::generators::RandomStrings;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordAnalyzer = org::apache::lucene::analysis::core::KeywordAnalyzer;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using RegexpQuery = org::apache::lucene::search::RegexpQuery;
using Sort = org::apache::lucene::search::Sort;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanFirstQuery = org::apache::lucene::search::spans::SpanFirstQuery;
using SpanMultiTermQueryWrapper =
    org::apache::lucene::search::spans::SpanMultiTermQueryWrapper;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanNotQuery = org::apache::lucene::search::spans::SpanNotQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using HighlightFlag =
    org::apache::lucene::search::uhighlight::UnifiedHighlighter::HighlightFlag;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using org::junit::After;
using org::junit::Before;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @ParametersFactory public static Iterable<Object[]>
// parameters()
deque<std::deque<any>> TestUnifiedHighlighterMTQ::parameters()
{
  return UHTestHelper::parametersFactoryList();
}

TestUnifiedHighlighterMTQ::TestUnifiedHighlighterMTQ(
    shared_ptr<FieldType> fieldType)
    : fieldType(fieldType)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void doBefore() throws java.io.IOException
void TestUnifiedHighlighterMTQ::doBefore() 
{
  dir = newDirectory();
  indexAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE,
                                true); // whitespace, punctuation, lowercase
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void doAfter() throws java.io.IOException
void TestUnifiedHighlighterMTQ::doAfter()  { delete dir; }

void TestUnifiedHighlighterMTQ::testWildcards() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighter>(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // disable MTQ; won't highlight
  highlighter->setHandleMultiTermQuery(false);
  snippets = highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a test.", snippets[0]);
  assertEquals(L"Test a one sentence document.", snippets[1]);
  highlighter->setHandleMultiTermQuery(true); // reset

  // wrong field
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::SHOULD)
          ->add(make_shared<WildcardQuery>(make_shared<Term>(L"bogus", L"te*")),
                BooleanClause::Occur::SHOULD)
          ->build();
  topDocs = searcher->search(bq, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", bq, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a test.", snippets[0]);
  assertEquals(L"Test a one sentence document.", snippets[1]);

  delete ir;
}

shared_ptr<UnifiedHighlighter>
TestUnifiedHighlighterMTQ::randomUnifiedHighlighter(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Analyzer> indexAnalyzer)
{
  return TestUnifiedHighlighter::randomUnifiedHighlighter(
      searcher, indexAnalyzer, EnumSet::of(HighlightFlag::MULTI_TERM_QUERY));
}

void TestUnifiedHighlighterMTQ::testOnePrefix() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  // wrap in a BoostQuery to also show we see inside it
  shared_ptr<Query> query = make_shared<BoostQuery>(
      make_shared<PrefixQuery>(make_shared<Term>(L"body", L"te")), 2.0f);
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // wrong field
  highlighter->setFieldMatcher(nullptr); // default
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::SHOULD)
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"bogus", L"te")),
                BooleanClause::Occur::SHOULD)
          ->build();
  topDocs = searcher->search(bq, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", bq, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a test.", snippets[0]);
  assertEquals(L"Test a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testOneRegexp() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<RegexpQuery>(make_shared<Term>(L"body", L"te.*"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // wrong field
  highlighter->setFieldMatcher(nullptr); // default
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::SHOULD)
          ->add(make_shared<RegexpQuery>(make_shared<Term>(L"bogus", L"te.*")),
                BooleanClause::Occur::SHOULD)
          ->build();
  topDocs = searcher->search(bq, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", bq, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a test.", snippets[0]);
  assertEquals(L"Test a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testOneFuzzy() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<FuzzyQuery>(make_shared<Term>(L"body", L"tets"), 1);
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // with prefix
  query = make_shared<FuzzyQuery>(make_shared<Term>(L"body", L"tets"), 1, 2);
  topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // wrong field
  highlighter->setFieldMatcher(nullptr); // default
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::SHOULD)
          ->add(
              make_shared<FuzzyQuery>(make_shared<Term>(L"bogus", L"tets"), 1),
              BooleanClause::Occur::SHOULD)
          ->build();
  topDocs = searcher->search(bq, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", bq, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a test.", snippets[0]);
  assertEquals(L"Test a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testRanges() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      TermRangeQuery::newStringRange(L"body", L"ta", L"tf", true, true);
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // null start
  query = TermRangeQuery::newStringRange(L"body", L"", L"tf", true, true);
  topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This <b>is</b> <b>a</b> <b>test</b>.", snippets[0]);
  assertEquals(
      L"<b>Test</b> <b>a</b> <b>one</b> <b>sentence</b> <b>document</b>.",
      snippets[1]);

  // null end
  query = TermRangeQuery::newStringRange(L"body", L"ta", L"", true, true);
  topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"<b>This</b> is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // exact start inclusive
  query = TermRangeQuery::newStringRange(L"body", L"test", L"tf", true, true);
  topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // exact end inclusive
  query = TermRangeQuery::newStringRange(L"body", L"ta", L"test", true, true);
  topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // exact start exclusive
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::SHOULD)
          ->add(TermRangeQuery::newStringRange(L"body", L"test", L"tf", false,
                                               true),
                BooleanClause::Occur::SHOULD)
          ->build();
  topDocs = searcher->search(bq, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", bq, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a test.", snippets[0]);
  assertEquals(L"Test a one sentence document.", snippets[1]);

  // exact end exclusive
  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::SHOULD)
           ->add(TermRangeQuery::newStringRange(L"body", L"ta", L"test", true,
                                                false),
                 BooleanClause::Occur::SHOULD)
           ->build();
  topDocs = searcher->search(bq, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", bq, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a test.", snippets[0]);
  assertEquals(L"Test a one sentence document.", snippets[1]);

  // wrong field
  highlighter->setFieldMatcher(nullptr); // default
  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::SHOULD)
           ->add(TermRangeQuery::newStringRange(L"bogus", L"ta", L"tf", true,
                                                true),
                 BooleanClause::Occur::SHOULD)
           ->build();
  topDocs = searcher->search(bq, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", bq, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a test.", snippets[0]);
  assertEquals(L"Test a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testWildcardInBoolean() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*")),
                BooleanClause::Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  // must not
  query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<MatchAllDocsQuery>(), BooleanClause::Occur::SHOULD)
          ->add(make_shared<WildcardQuery>(make_shared<Term>(L"bogus", L"te*")),
                BooleanClause::Occur::MUST_NOT)
          ->build();
  topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a test.", snippets[0]);
  assertEquals(L"Test a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testWildcardInFiltered() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*")),
                BooleanClause::Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"test")),
                BooleanClause::Occur::FILTER)
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testWildcardInConstantScore() throw(
    runtime_error)
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<ConstantScoreQuery> query = make_shared<ConstantScoreQuery>(
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*")));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testWildcardInDisjunctionMax() throw(
    runtime_error)
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<DisjunctionMaxQuery> query = make_shared<DisjunctionMaxQuery>(
      Collections::singleton(
          make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*"))),
      0);
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testSpanWildcard() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  // wrap in a SpanBoostQuery to also show we see inside it
  shared_ptr<Query> query = make_shared<SpanBoostQuery>(
      make_shared<SpanMultiTermQueryWrapper<>>(
          make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*"))),
      2.0f);
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testSpanOr() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<SpanQuery> childQuery = make_shared<SpanMultiTermQueryWrapper<>>(
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*")));
  shared_ptr<Query> query = make_shared<SpanOrQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{childQuery});
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testSpanNear() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<SpanQuery> childQuery = make_shared<SpanMultiTermQueryWrapper<>>(
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*")));
  shared_ptr<Query> query = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{childQuery, childQuery}, 0,
      false);
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testSpanNot() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<SpanQuery> include = make_shared<SpanMultiTermQueryWrapper<>>(
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*")));
  shared_ptr<SpanQuery> exclude =
      make_shared<SpanTermQuery>(make_shared<Term>(L"body", L"bogus"));
  shared_ptr<Query> query = make_shared<SpanNotQuery>(include, exclude);
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testSpanPositionCheck() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"This is a test.");
  iw->addDocument(doc);
  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<SpanQuery> childQuery = make_shared<SpanMultiTermQueryWrapper<>>(
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"te*")));
  shared_ptr<Query> query = make_shared<SpanFirstQuery>(childQuery, 1000000);
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(2, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(2, snippets.size());
  assertEquals(L"This is a <b>test</b>.", snippets[0]);
  assertEquals(L"<b>Test</b> a one sentence document.", snippets[1]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testWhichMTQMatched() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"Test a one sentence document.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  // use a variety of common MTQ types
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"body", L"te")),
                BooleanClause::Occur::SHOULD)
          ->add(
              make_shared<WildcardQuery>(make_shared<Term>(L"body", L"*one*")),
              BooleanClause::Occur::SHOULD)
          ->add(
              make_shared<FuzzyQuery>(make_shared<Term>(L"body", L"zentence~")),
              BooleanClause::Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs);
  assertEquals(1, snippets.size());

  // Default formatter just bolds each hit:
  assertEquals(L"<b>Test</b> a <b>one</b> <b>sentence</b> document.",
               snippets[0]);

  // Now use our own formatter, that also stuffs the
  // matching term's text into the result:
  highlighter = make_shared<UnifiedHighlighterAnonymousInnerClass>(
      shared_from_this(), searcher, indexAnalyzer);

  assertEquals(1, topDocs->totalHits);
  snippets = highlighter->highlight(L"body", query, topDocs);
  assertEquals(1, snippets.size());

  assertEquals(L"<b>Test(body:te*)</b> a <b>one(body:*one*)</b> "
               L"<b>sentence(body:zentence~~2)</b> document.",
               snippets[0]);

  delete ir;
}

TestUnifiedHighlighterMTQ::UnifiedHighlighterAnonymousInnerClass::
    UnifiedHighlighterAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterMTQ> outerInstance,
        shared_ptr<IndexSearcher> searcher, shared_ptr<Analyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PassageFormatter>
TestUnifiedHighlighterMTQ::UnifiedHighlighterAnonymousInnerClass::getFormatter(
    const wstring &field)
{
  return make_shared<PassageFormatterAnonymousInnerClass>(shared_from_this());
}

TestUnifiedHighlighterMTQ::UnifiedHighlighterAnonymousInnerClass::
    PassageFormatterAnonymousInnerClass::PassageFormatterAnonymousInnerClass(
        shared_ptr<UnifiedHighlighterAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

any TestUnifiedHighlighterMTQ::UnifiedHighlighterAnonymousInnerClass::
    PassageFormatterAnonymousInnerClass::format(
        std::deque<std::shared_ptr<Passage>> &passages, const wstring &content)
{
  // Copied from DefaultPassageFormatter, but
  // tweaked to include the matched term:
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int pos = 0;
  for (auto passage : passages) {
    // don't add ellipsis if its the first one, or if its connected.
    if (passage->getStartOffset() > pos && pos > 0) {
      sb->append(L"... ");
    }
    pos = passage->getStartOffset();
    for (int i = 0; i < passage->getNumMatches(); i++) {
      int start = passage->getMatchStarts()[i];
      int end = passage->getMatchEnds()[i];
      // its possible to have overlapping terms
      if (start > pos) {
        sb->append(content, pos, start);
      }
      if (end > pos) {
        sb->append(L"<b>");
        sb->append(content, max(pos, start), end);
        sb->append(L'(');
        sb->append(passage->getMatchTerms()[i]->utf8ToString());
        sb->append(L')');
        sb->append(L"</b>");
        pos = end;
      }
    }
    // its possible a "term" from the analyzer could span a sentence boundary.
    sb->append(content, pos, max(pos, passage->getEndOffset()));
    pos = passage->getEndOffset();
  }
  return sb->toString();
}

void TestUnifiedHighlighterMTQ::testWithMaxLen() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(
      L"Alpha Bravo foo foo foo. Foo foo Alpha Bravo"); // 44 char long, 2
                                                        // sentences
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setMaxLength(25); // a little past first sentence

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"alpha")),
                BooleanClause::Occur::MUST)
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"body", L"bra")),
                BooleanClause::Occur::MUST)
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets = highlighter->highlight(
      L"body", query, topDocs, 2); // ask for 2 but we'll only get 1
  assertArrayEquals(
      std::deque<wstring>{L"<b>Alpha</b> <b>Bravo</b> foo foo foo. "},
      snippets);

  delete ir;
}

void TestUnifiedHighlighterMTQ::
    testWithMaxLenAndMultipleWildcardMatches() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  // tests interleaving of multiple wildcard matches with the
  // CompositePostingsEnum In this case the CompositePostingsEnum will have an
  // underlying PostingsEnum that jumps form pos 1 to 9 for bravo and a second
  // with position 2 for Bravado
  body->setStringValue(L"Alpha Bravo Bravado foo foo foo. Foo foo Alpha Bravo");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  highlighter->setMaxLength(32); // a little past first sentence

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"alpha")),
                BooleanClause::Occur::MUST)
          ->add(make_shared<PrefixQuery>(make_shared<Term>(L"body", L"bra")),
                BooleanClause::Occur::MUST)
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  std::deque<wstring> snippets = highlighter->highlight(
      L"body", query, topDocs, 2); // ask for 2 but we'll only get 1
  assertArrayEquals(
      std::deque<wstring>{
          L"<b>Alpha</b> <b>Bravo</b> <b>Bravado</b> foo foo foo."},
      snippets);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testTokenStreamIsClosed() 
{
  // note: test is a derivative of testWithMaxLen()
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(L"Alpha Bravo foo foo foo. Foo foo Alpha Bravo");
  if (random()->nextBoolean()) { // sometimes add a 2nd value (maybe matters?)
    doc->push_back(
        make_shared<Field>(L"body", L"2nd value Alpha Bravo", fieldType));
  }
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  // use this buggy Analyzer at highlight time
  shared_ptr<Analyzer> buggyAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, buggyAnalyzer);
  highlighter->setHandleMultiTermQuery(true);
  if (rarely()) {
    highlighter->setMaxLength(25); // a little past first sentence
  }

  bool hasClauses = false;
  shared_ptr<BooleanQuery::Builder> queryBuilder =
      make_shared<BooleanQuery::Builder>();
  if (random()->nextBoolean()) {
    hasClauses = true;
    queryBuilder->add(
        make_shared<TermQuery>(make_shared<Term>(L"body", L"alpha")),
        BooleanClause::Occur::MUST);
  }
  if (!hasClauses || random()->nextBoolean()) {
    queryBuilder->add(
        make_shared<PrefixQuery>(make_shared<Term>(L"body", L"bra")),
        BooleanClause::Occur::MUST);
  }
  shared_ptr<BooleanQuery> query = queryBuilder->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  try {
    std::deque<wstring> snippets =
        highlighter->highlight(L"body", query, topDocs, 2);
    // don't even care what the results are; just want to test exception
    // behavior
    if (fieldType == UHTestHelper::reanalysisType) {
      fail(L"Expecting EXPECTED IOException");
    }
  } catch (const runtime_error &e) {
    if (!e.what()->contains(L"EXPECTED")) {
      throw e;
    }
  }
  delete ir;

  // Now test we can get the tokenStream without it puking due to
  // IllegalStateException for not calling close()

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // buggyAnalyzer.tokenStream("body", "anything"))
  {
    org::apache::lucene::analysis::TokenStream ts =
        buggyAnalyzer->tokenStream(L"body", L"anything");
    ts->reset(); // hopefully doesn't throw
    // don't call incrementToken; we know it's buggy ;-)
  }
}

TestUnifiedHighlighterMTQ::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterMTQ> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestUnifiedHighlighterMTQ::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> buggyTokenizer =
      make_shared<TokenizerAnonymousInnerClass>(shared_from_this());
  return make_shared<Analyzer::TokenStreamComponents>(buggyTokenizer);
}

TestUnifiedHighlighterMTQ::AnalyzerAnonymousInnerClass::
    TokenizerAnonymousInnerClass::TokenizerAnonymousInnerClass(
        shared_ptr<AnalyzerAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestUnifiedHighlighterMTQ::AnalyzerAnonymousInnerClass::
    TokenizerAnonymousInnerClass::incrementToken() 
{
  // C++ TODO: The following line could not be converted:
  throw java.io.IOException(L"EXPECTED");
}

void TestUnifiedHighlighterMTQ::testNothingAnalyzes() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(
      L"body", L" ", fieldType)); // just a space! (thus not empty)
  doc->push_back(newTextField(L"id", L"id", Field::Store::YES));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"body", L"something", fieldType));
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  int docID =
      searcher
          ->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"id")), 1)
          ->scoreDocs[0]
          ->doc;

  shared_ptr<Query> query =
      make_shared<PrefixQuery>(make_shared<Term>(L"body", L"nonexistent"));
  std::deque<int> docIDs(1);
  docIDs[0] = docID;
  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIDs,
                                   std::deque<int>{2})[L"body"];
  assertEquals(1, snippets.size());
  assertEquals(L" ", snippets[0]);

  delete ir;
}

void TestUnifiedHighlighterMTQ::testMultiSegment() 
{
  // If we incorrectly got the term deque from mis-matched global/leaf doc ID,
  // this test may fail
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"body", L"word aberration", fieldType));
  iw->addDocument(doc);

  iw->commit(); // make segment

  doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"body", L"word absolve", fieldType));
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<PrefixQuery>(make_shared<Term>(L"body", L"ab"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10);

  std::deque<wstring> snippets = highlighter->highlightFields(
      std::deque<wstring>{L"body"}, query, topDocs)[L"body"];
  Arrays::sort(snippets);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[word <b>aberration</b>, word <b>absolve</b>]",
               Arrays->toString(snippets));

  delete ir;
}

void TestUnifiedHighlighterMTQ::
    testPositionSensitiveWithWildcardDoesNotHighlight() 
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(
      L"body", L"iterate insect ipswitch illinois indirect", fieldType));
  doc->push_back(newTextField(L"id", L"id", Field::Store::YES));

  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, indexAnalyzer);
  int docID =
      searcher
          ->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"id")), 1)
          ->scoreDocs[0]
          ->doc;

  shared_ptr<PhraseQuery> pq = (make_shared<PhraseQuery::Builder>())
                                   ->add(make_shared<Term>(L"body", L"consent"))
                                   ->add(make_shared<Term>(L"body", L"order"))
                                   ->build();

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<WildcardQuery>(
                    make_shared<Term>(L"body", L"enforc*")),
                BooleanClause::Occur::MUST)
          ->add(pq, BooleanClause::Occur::MUST)
          ->build();

  std::deque<int> docIds = {docID};

  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIds,
                                   std::deque<int>{2})[L"body"];
  assertEquals(1, snippets.size());
  assertEquals(L"iterate insect ipswitch illinois indirect", snippets[0]);
  delete ir;
}

void TestUnifiedHighlighterMTQ::testCustomSpanQueryHighlighting() throw(
    runtime_error)
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(
      L"body",
      L"alpha bravo charlie delta echo foxtrot golf hotel india juliet",
      fieldType));
  doc->push_back(newTextField(L"id", L"id", Field::Store::YES));

  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighterAnonymousInnerClass2>(
          shared_from_this(), searcher, indexAnalyzer);

  int docId =
      searcher
          ->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"id")), 1)
          ->scoreDocs[0]
          ->doc;

  shared_ptr<WildcardQuery> wildcardQuery =
      make_shared<WildcardQuery>(make_shared<Term>(L"body", L"foxtr*"));
  shared_ptr<SpanMultiTermQueryWrapper> wildcardQueryWrapper =
      make_shared<SpanMultiTermQueryWrapper<>>(wildcardQuery);

  shared_ptr<SpanQuery> wrappedQuery =
      make_shared<MyWrapperSpanQuery>(wildcardQueryWrapper);

  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(wrappedQuery, BooleanClause::Occur::SHOULD)
          ->build();

  std::deque<int> docIds = {docId};

  std::deque<wstring> snippets =
      highlighter->highlightFields(std::deque<wstring>{L"body"}, query, docIds,
                                   std::deque<int>{2})[L"body"];
  assertEquals(1, snippets.size());
  assertEquals(
      L"alpha bravo charlie delta echo <b>foxtrot</b> golf hotel india juliet",
      snippets[0]);
  delete ir;
}

TestUnifiedHighlighterMTQ::UnifiedHighlighterAnonymousInnerClass2::
    UnifiedHighlighterAnonymousInnerClass2(
        shared_ptr<TestUnifiedHighlighterMTQ> outerInstance,
        shared_ptr<IndexSearcher> searcher, shared_ptr<Analyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

deque<std::shared_ptr<Query>>
TestUnifiedHighlighterMTQ::UnifiedHighlighterAnonymousInnerClass2::
    preMultiTermQueryRewrite(shared_ptr<Query> query)
{
  if (std::dynamic_pointer_cast<MyWrapperSpanQuery>(query) != nullptr) {
    return Collections::singletonList(
        (std::static_pointer_cast<MyWrapperSpanQuery>(query))->originalQuery);
  }
  return nullptr;
}

TestUnifiedHighlighterMTQ::MyWrapperSpanQuery::MyWrapperSpanQuery(
    shared_ptr<SpanQuery> originalQuery)
    : originalQuery(Objects::requireNonNull(originalQuery))
{
}

wstring TestUnifiedHighlighterMTQ::MyWrapperSpanQuery::getField()
{
  return originalQuery->getField();
}

wstring
TestUnifiedHighlighterMTQ::MyWrapperSpanQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"(Wrapper[" + originalQuery->toString(field) + L"])";
}

shared_ptr<SpanWeight>
TestUnifiedHighlighterMTQ::MyWrapperSpanQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return originalQuery->createWeight(searcher, needsScores, boost);
}

shared_ptr<Query> TestUnifiedHighlighterMTQ::MyWrapperSpanQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> newOriginalQuery = originalQuery->rewrite(reader);
  if (newOriginalQuery != originalQuery) {
    return make_shared<MyWrapperSpanQuery>(
        std::static_pointer_cast<SpanQuery>(newOriginalQuery));
  }
  return shared_from_this();
}

bool TestUnifiedHighlighterMTQ::MyWrapperSpanQuery::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  return originalQuery->equals(
      (any_cast<std::shared_ptr<MyWrapperSpanQuery>>(o)).originalQuery);
}

int TestUnifiedHighlighterMTQ::MyWrapperSpanQuery::hashCode()
{
  return originalQuery->hashCode();
}

void TestUnifiedHighlighterMTQ::testRussianPrefixQuery() 
{
  shared_ptr<Analyzer> analyzer = make_shared<StandardAnalyzer>();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);
  wstring field = L"title";
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(
      field, L"я", fieldType)); // Russian char; uses 2 UTF8 bytes
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Query> query =
      make_shared<PrefixQuery>(make_shared<Term>(field, L"я"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 1);
  shared_ptr<UnifiedHighlighter> highlighter =
      randomUnifiedHighlighter(searcher, analyzer);
  std::deque<wstring> snippets = highlighter->highlight(field, query, topDocs);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[<b>я</b>]", Arrays->toString(snippets));
  delete ir;
}

void TestUnifiedHighlighterMTQ::testMultiByteMTQ() 
{
  shared_ptr<Analyzer> analyzer = make_shared<KeywordAnalyzer>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.RandomIndexWriter iw
  // = new org.apache.lucene.index.RandomIndexWriter(random(), dir, analyzer))
  {
    org::apache::lucene::index::RandomIndexWriter iw =
        org::apache::lucene::index::RandomIndexWriter(random(), dir, analyzer);
    for (int attempt = 0; attempt < 20; attempt++) {
      iw->deleteAll();
      wstring field = L"title";
      wstring value = RandomStrings::randomUnicodeOfLength(random(), 3);
      if (value.find(
              StringHelper::toString(UnifiedHighlighter::MULTIVAL_SEP_CHAR) +
              L"") != wstring::npos) { // will throw things off
        continue;
      }
      std::deque<int> valuePoints = value.codePoints().toArray();

      iw->addDocument(
          Collections::singleton(make_shared<Field>(field, value, fieldType)));
      iw->commit();
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
      // ir = iw.getReader())
      {
        org::apache::lucene::index::IndexReader ir = iw->getReader();
        shared_ptr<IndexSearcher> searcher = newSearcher(ir);
        shared_ptr<UnifiedHighlighter> highlighter =
            randomUnifiedHighlighter(searcher, analyzer);
        highlighter->setBreakIterator(WholeBreakIterator::new);

        // Test PrefixQuery
        shared_ptr<Query> query = make_shared<PrefixQuery>(make_shared<Term>(
            field, UnicodeUtil::newString(valuePoints, 0, 1)));
        highlightAndAssertMatch(searcher, highlighter, query, field, value);

        // Test TermRangeQuery
        query = make_shared<TermRangeQuery>(field, make_shared<BytesRef>(value),
                                            make_shared<BytesRef>(value), true,
                                            true);
        highlightAndAssertMatch(searcher, highlighter, query, field, value);

        // Test FuzzyQuery
        query =
            make_shared<FuzzyQuery>(make_shared<Term>(field, value + L"Z"), 1);
        highlightAndAssertMatch(searcher, highlighter, query, field, value);

        if (valuePoints.size() != 3) {
          continue; // even though we ask RandomStrings for a std::wstring with 3 code
                    // points, it seems sometimes it's less
        }

        // Test WildcardQuery
        query = make_shared<WildcardQuery>(make_shared<Term>(
            field,
            make_shared<StringBuilder>()
                // C++ TODO: There is no native C++ equivalent to 'toString':
                .append(WildcardQuery::WILDCARD_ESCAPE)
                .appendCodePoint(valuePoints[0])
                ->append(WildcardQuery::WILDCARD_CHAR)
                ->append(WildcardQuery::WILDCARD_ESCAPE)
                .appendCodePoint(valuePoints[2])
                ->toString()));
        highlightAndAssertMatch(searcher, highlighter, query, field, value);

        // TODO hmmm; how to randomly generate RegexpQuery? Low priority; we've
        // covered the others well.
      }
    }
  }
}

void TestUnifiedHighlighterMTQ::highlightAndAssertMatch(
    shared_ptr<IndexSearcher> searcher,
    shared_ptr<UnifiedHighlighter> highlighter, shared_ptr<Query> query,
    const wstring &field, const wstring &fieldVal) 
{
  shared_ptr<TopDocs> topDocs = searcher->search(query, 1);
  assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets = highlighter->highlight(field, query, topDocs);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[<b>" + fieldVal + L"</b>]", Arrays->toString(snippets));
}
} // namespace org::apache::lucene::search::uhighlight