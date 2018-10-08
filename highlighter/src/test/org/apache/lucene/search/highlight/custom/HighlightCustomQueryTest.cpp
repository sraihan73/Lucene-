using namespace std;

#include "HighlightCustomQueryTest.h"

namespace org::apache::lucene::search::highlight::custom
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using Highlighter = org::apache::lucene::search::highlight::Highlighter;
using InvalidTokenOffsetsException =
    org::apache::lucene::search::highlight::InvalidTokenOffsetsException;
using QueryScorer = org::apache::lucene::search::highlight::QueryScorer;
using SimpleFragmenter =
    org::apache::lucene::search::highlight::SimpleFragmenter;
using SimpleHTMLFormatter =
    org::apache::lucene::search::highlight::SimpleHTMLFormatter;
using WeightedSpanTerm =
    org::apache::lucene::search::highlight::WeightedSpanTerm;
using WeightedSpanTermExtractor =
    org::apache::lucene::search::highlight::WeightedSpanTermExtractor;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring HighlightCustomQueryTest::FIELD_NAME = L"contents";

void HighlightCustomQueryTest::testHighlightCustomQuery() throw(
    IOException, InvalidTokenOffsetsException)
{
  wstring s1 = L"I call our world Flatland, not because we call it so,";

  // Verify that a query against the default field results in text being
  // highlighted
  // regardless of the field name.

  shared_ptr<CustomQuery> q =
      make_shared<CustomQuery>(make_shared<Term>(FIELD_NAME, L"world"));

  wstring expected =
      L"I call our <B>world</B> Flatland, not because we call it so,";
  wstring observed = highlightField(q, L"SOME_FIELD_NAME", s1);
  if (VERBOSE) {
    wcout << L"Expected: \"" << expected << L"\n"
          << L"Observed: \"" << observed << endl;
  }
  assertEquals(L"Query in the default field results in text for *ANY* field "
               L"being highlighted",
               expected, observed);

  // Verify that a query against a named field does not result in any
  // highlighting
  // when the query field name differs from the name of the field being
  // highlighted,
  // which in this example happens to be the default field name.
  q = make_shared<CustomQuery>(make_shared<Term>(L"text", L"world"));

  expected = s1;
  observed = highlightField(q, FIELD_NAME, s1);
  if (VERBOSE) {
    wcout << L"Expected: \"" << expected << L"\n"
          << L"Observed: \"" << observed << endl;
  }
  assertEquals(L"Query in a named field does not result in highlighting when "
               L"that field isn't in the query",
               s1, highlightField(q, FIELD_NAME, s1));
}

void HighlightCustomQueryTest::testHighlightKnownQuery() 
{
  shared_ptr<WeightedSpanTermExtractor> extractor =
      make_shared<WeightedSpanTermExtractorAnonymousInnerClass>(
          shared_from_this());
  unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>> terms =
      extractor->getWeightedSpanTerms(
          make_shared<TermQuery>(make_shared<Term>(L"bar", L"quux")), 3,
          make_shared<CannedTokenStream>());
  // no foo
  assertEquals(Collections::singleton(L"quux"), terms.keySet());
}

HighlightCustomQueryTest::WeightedSpanTermExtractorAnonymousInnerClass::
    WeightedSpanTermExtractorAnonymousInnerClass(
        shared_ptr<HighlightCustomQueryTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

void HighlightCustomQueryTest::WeightedSpanTermExtractorAnonymousInnerClass::
    extractUnknownQuery(
        shared_ptr<Query> query,
        unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>>
            &terms) 
{
  terms.emplace(L"foo", make_shared<WeightedSpanTerm>(3, L"foo"));
}

wstring HighlightCustomQueryTest::highlightField(
    shared_ptr<Query> query, const wstring &fieldName,
    const wstring &text) 
{
  shared_ptr<TokenStream> tokenStream =
      (make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true,
                                 MockTokenFilter::ENGLISH_STOPSET))
          ->tokenStream(fieldName, text);
  // Assuming "<B>", "</B>" used to highlight
  shared_ptr<SimpleHTMLFormatter> formatter =
      make_shared<SimpleHTMLFormatter>();
  shared_ptr<MyQueryScorer> scorer =
      make_shared<MyQueryScorer>(query, fieldName, FIELD_NAME);
  shared_ptr<Highlighter> highlighter =
      make_shared<Highlighter>(formatter, scorer);
  highlighter->setTextFragmenter(
      make_shared<SimpleFragmenter>(numeric_limits<int>::max()));

  wstring rv = highlighter->getBestFragments(tokenStream, text, 1,
                                             L"(FIELD TEXT TRUNCATED)");
  return rv.length() == 0 ? text : rv;
}

HighlightCustomQueryTest::MyWeightedSpanTermExtractor::
    MyWeightedSpanTermExtractor()
    : org::apache::lucene::search::highlight::WeightedSpanTermExtractor()
{
}

HighlightCustomQueryTest::MyWeightedSpanTermExtractor::
    MyWeightedSpanTermExtractor(const wstring &defaultField)
    : org::apache::lucene::search::highlight::WeightedSpanTermExtractor(
          defaultField)
{
}

void HighlightCustomQueryTest::MyWeightedSpanTermExtractor::extractUnknownQuery(
    shared_ptr<Query> query,
    unordered_map<wstring, std::shared_ptr<WeightedSpanTerm>>
        &terms) 
{
  float boost = 1.0f;
  while (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
    shared_ptr<BoostQuery> bq = std::static_pointer_cast<BoostQuery>(query);
    boost *= bq->getBoost();
    query = bq->getQuery();
  }
  if (std::dynamic_pointer_cast<CustomQuery>(query) != nullptr) {
    extractWeightedTerms(
        terms,
        make_shared<TermQuery>(
            (std::static_pointer_cast<CustomQuery>(query))->term),
        boost);
  }
}

HighlightCustomQueryTest::MyQueryScorer::MyQueryScorer(
    shared_ptr<Query> query, const wstring &field, const wstring &defaultField)
    : org::apache::lucene::search::highlight::QueryScorer(query, field,
                                                          defaultField)
{
}

shared_ptr<WeightedSpanTermExtractor>
HighlightCustomQueryTest::MyQueryScorer::newTermExtractor(
    const wstring &defaultField)
{
  return defaultField == L""
             ? make_shared<MyWeightedSpanTermExtractor>()
             : make_shared<MyWeightedSpanTermExtractor>(defaultField);
}

HighlightCustomQueryTest::CustomQuery::CustomQuery(shared_ptr<Term> term)
    : term(term)
{
}

wstring HighlightCustomQueryTest::CustomQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return (make_shared<TermQuery>(term))->toString(field);
}

shared_ptr<Query> HighlightCustomQueryTest::CustomQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  return make_shared<TermQuery>(term);
}

int HighlightCustomQueryTest::CustomQuery::hashCode()
{
  return classHash() + Objects::hashCode(term);
}

bool HighlightCustomQueryTest::CustomQuery::equals(any other)
{
  return sameClassAs(other) &&
         Objects::equals(term,
                         (any_cast<std::shared_ptr<CustomQuery>>(other)).term);
}
} // namespace org::apache::lucene::search::highlight::custom