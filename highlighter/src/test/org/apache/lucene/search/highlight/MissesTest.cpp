using namespace std;

#include "MissesTest.h"

namespace org::apache::lucene::search::highlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void MissesTest::testTermQuery() throw(IOException,
                                       InvalidTokenOffsetsException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer analyzer
  // = new org.apache.lucene.analysis.MockAnalyzer(random(),
  // org.apache.lucene.analysis.MockTokenizer.WHITESPACE, false))
  {
    org::apache::lucene::analysis::Analyzer analyzer =
        org::apache::lucene::analysis::MockAnalyzer(
            random(), org::apache::lucene::analysis::MockTokenizer::WHITESPACE,
            false);
    shared_ptr<Query> *const query =
        make_shared<TermQuery>(make_shared<Term>(L"test", L"foo"));
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<QueryScorer>(query));
    assertEquals(L"this is a <B>foo</B> bar example",
                 highlighter->getBestFragment(analyzer, L"test",
                                              L"this is a foo bar example"));
    assertNull(highlighter->getBestFragment(analyzer, L"test",
                                            L"this does not match"));
  }
}

void MissesTest::testBooleanQuery() throw(IOException,
                                          InvalidTokenOffsetsException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer analyzer
  // = new org.apache.lucene.analysis.MockAnalyzer(random(),
  // org.apache.lucene.analysis.MockTokenizer.WHITESPACE, false))
  {
    org::apache::lucene::analysis::Analyzer analyzer =
        org::apache::lucene::analysis::MockAnalyzer(
            random(), org::apache::lucene::analysis::MockTokenizer::WHITESPACE,
            false);
    shared_ptr<BooleanQuery::Builder> *const query =
        make_shared<BooleanQuery::Builder>();
    query->add(make_shared<TermQuery>(make_shared<Term>(L"test", L"foo")),
               Occur::MUST);
    query->add(make_shared<TermQuery>(make_shared<Term>(L"test", L"bar")),
               Occur::MUST);
    shared_ptr<Highlighter> *const highlighter =
        make_shared<Highlighter>(make_shared<SimpleHTMLFormatter>(),
                                 make_shared<QueryScorer>(query->build()));
    assertEquals(L"this is a <B>foo</B> <B>bar</B> example",
                 highlighter->getBestFragment(analyzer, L"test",
                                              L"this is a foo bar example"));
    assertNull(highlighter->getBestFragment(analyzer, L"test",
                                            L"this does not match"));
  }
}

void MissesTest::testPhraseQuery() throw(IOException,
                                         InvalidTokenOffsetsException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer analyzer
  // = new org.apache.lucene.analysis.MockAnalyzer(random(),
  // org.apache.lucene.analysis.MockTokenizer.WHITESPACE, false))
  {
    org::apache::lucene::analysis::Analyzer analyzer =
        org::apache::lucene::analysis::MockAnalyzer(
            random(), org::apache::lucene::analysis::MockTokenizer::WHITESPACE,
            false);
    shared_ptr<PhraseQuery> *const query =
        make_shared<PhraseQuery>(L"test", L"foo", L"bar");
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<QueryScorer>(query));
    assertEquals(L"this is a <B>foo</B> <B>bar</B> example",
                 highlighter->getBestFragment(analyzer, L"test",
                                              L"this is a foo bar example"));
    assertNull(highlighter->getBestFragment(analyzer, L"test",
                                            L"this does not match"));
  }
}

void MissesTest::testSpanNearQuery() throw(IOException,
                                           InvalidTokenOffsetsException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer analyzer
  // = new org.apache.lucene.analysis.MockAnalyzer(random(),
  // org.apache.lucene.analysis.MockTokenizer.WHITESPACE, false))
  {
    org::apache::lucene::analysis::Analyzer analyzer =
        org::apache::lucene::analysis::MockAnalyzer(
            random(), org::apache::lucene::analysis::MockTokenizer::WHITESPACE,
            false);
    shared_ptr<Query> *const query = make_shared<SpanNearQuery>(
        std::deque<std::shared_ptr<SpanQuery>>{
            make_shared<SpanTermQuery>(make_shared<Term>(L"test", L"foo")),
            make_shared<SpanTermQuery>(make_shared<Term>(L"test", L"bar"))},
        0, true);
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<QueryScorer>(query));
    assertEquals(L"this is a <B>foo</B> <B>bar</B> example",
                 highlighter->getBestFragment(analyzer, L"test",
                                              L"this is a foo bar example"));
    assertNull(highlighter->getBestFragment(analyzer, L"test",
                                            L"this does not match"));
  }
}
} // namespace org::apache::lucene::search::highlight