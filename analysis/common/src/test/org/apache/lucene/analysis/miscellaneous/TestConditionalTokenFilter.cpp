using namespace std;

#include "TestConditionalTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharacterUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../java/org/apache/lucene/analysis/standard/ClassicTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SolrSynonymParser.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymMap.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Token = org::apache::lucene::analysis::Token;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ValidatingTokenFilter =
    org::apache::lucene::analysis::ValidatingTokenFilter;
using TypeTokenFilter = org::apache::lucene::analysis::core::TypeTokenFilter;
using GermanStemFilter = org::apache::lucene::analysis::de::GermanStemFilter;
using IndicNormalizationFilter =
    org::apache::lucene::analysis::in_::IndicNormalizationFilter;
using NGramTokenizer = org::apache::lucene::analysis::ngram::NGramTokenizer;
using FixedShingleFilter =
    org::apache::lucene::analysis::shingle::FixedShingleFilter;
using ShingleFilter = org::apache::lucene::analysis::shingle::ShingleFilter;
using ClassicTokenizer =
    org::apache::lucene::analysis::standard::ClassicTokenizer;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using SolrSynonymParser =
    org::apache::lucene::analysis::synonym::SolrSynonymParser;
using SynonymGraphFilter =
    org::apache::lucene::analysis::synonym::SynonymGraphFilter;
using SynonymMap = org::apache::lucene::analysis::synonym::SynonymMap;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

TestConditionalTokenFilter::AssertingLowerCaseFilter::AssertingLowerCaseFilter(
    shared_ptr<TestConditionalTokenFilter> outerInstance,
    shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_),
      outerInstance(outerInstance)
{
}

bool TestConditionalTokenFilter::AssertingLowerCaseFilter::
    incrementToken() 
{
  if (input->incrementToken()) {
    CharacterUtils::toLowerCase(termAtt->buffer(), 0, termAtt->length());
    return true;
  } else {
    return false;
  }
}

void TestConditionalTokenFilter::AssertingLowerCaseFilter::end() throw(
    IOException)
{
  TokenFilter::end();
  outerInstance->ended = true;
}

TestConditionalTokenFilter::AssertingLowerCaseFilter::
    ~AssertingLowerCaseFilter()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  outerInstance->closed = true;
}

void TestConditionalTokenFilter::AssertingLowerCaseFilter::reset() throw(
    IOException)
{
  TokenFilter::reset();
  outerInstance->reset = true;
}

TestConditionalTokenFilter::SkipMatchingFilter::SkipMatchingFilter(
    shared_ptr<TestConditionalTokenFilter> outerInstance,
    shared_ptr<TokenStream> input,
    function<TokenStream *(TokenStream *)> &inputFactory,
    const wstring &termRegex)
    : ConditionalTokenFilter(input, inputFactory),
      pattern(Pattern::compile(termRegex)), outerInstance(outerInstance)
{
}

bool TestConditionalTokenFilter::SkipMatchingFilter::shouldFilter() throw(
    IOException)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return pattern->matcher(termAtt->toString()).matches() == false;
}

void TestConditionalTokenFilter::testSimple() 
{
  shared_ptr<TokenStream> stream =
      whitespaceMockTokenizer(L"Alice Bob Clara David");
  shared_ptr<TokenStream> t = make_shared<SkipMatchingFilter>(
      shared_from_this(), stream, AssertingLowerCaseFilter::new, L".*o.*");
  assertTokenStreamContents(
      t, std::deque<wstring>{L"alice", L"Bob", L"clara", L"david"});
  assertTrue(closed);
  assertTrue(reset);
  assertTrue(ended);
}

TestConditionalTokenFilter::TokenSplitter::TokenSplitter(
    shared_ptr<TestConditionalTokenFilter> outerInstance,
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input),
      outerInstance(outerInstance)
{
}

bool TestConditionalTokenFilter::TokenSplitter::incrementToken() throw(
    IOException)
{
  if (half == L"") {
    state = captureState();
    if (input->incrementToken() == false) {
      return false;
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    half = termAtt->toString()->substr(4);
    termAtt->setLength(4);
    return true;
  }
  restoreState(state);
  termAtt->setEmpty()->append(half);
  half = L"";
  return true;
}

void TestConditionalTokenFilter::testMultitokenWrapping() 
{
  shared_ptr<TokenStream> stream =
      whitespaceMockTokenizer(L"tokenpos1 tokenpos2 tokenpos3 tokenpos4");
  shared_ptr<TokenStream> ts = make_shared<SkipMatchingFilter>(
      shared_from_this(), stream, TokenSplitter::new, L".*2.*");
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"toke", L"npos1", L"tokenpos2", L"toke",
                               L"npos3", L"toke", L"npos4"});
}

TestConditionalTokenFilter::EndTrimmingFilter::EndTrimmingFilter(
    shared_ptr<TestConditionalTokenFilter> outerInstance,
    shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::FilteringTokenFilter(in_),
      outerInstance(outerInstance)
{
}

bool TestConditionalTokenFilter::EndTrimmingFilter::accept() 
{
  return true;
}

void TestConditionalTokenFilter::EndTrimmingFilter::end() 
{
  FilteringTokenFilter::end();
  offsetAtt->setOffset(0, offsetAtt->endOffset() - 2);
}

void TestConditionalTokenFilter::testEndPropagation() 
{
  shared_ptr<CannedTokenStream> cts1 =
      make_shared<CannedTokenStream>(0, 20, make_shared<Token>(L"alice", 0, 5),
                                     make_shared<Token>(L"bob", 6, 8));
  shared_ptr<TokenStream> ts1 =
      make_shared<ConditionalTokenFilterAnonymousInnerClass>(
          shared_from_this(), cts1, EndTrimmingFilter::new);
  assertTokenStreamContents(ts1, std::deque<wstring>{L"alice", L"bob"},
                            nullptr, nullptr, nullptr, nullptr, nullptr, 20);

  shared_ptr<CannedTokenStream> cts2 =
      make_shared<CannedTokenStream>(0, 20, make_shared<Token>(L"alice", 0, 5),
                                     make_shared<Token>(L"bob", 6, 8));
  shared_ptr<TokenStream> ts2 =
      make_shared<ConditionalTokenFilterAnonymousInnerClass2>(
          shared_from_this(), cts2, EndTrimmingFilter::new);
  assertTokenStreamContents(ts2, std::deque<wstring>{L"alice", L"bob"},
                            nullptr, nullptr, nullptr, nullptr, nullptr, 18);
}

TestConditionalTokenFilter::ConditionalTokenFilterAnonymousInnerClass::
    ConditionalTokenFilterAnonymousInnerClass(
        shared_ptr<TestConditionalTokenFilter> outerInstance,
        shared_ptr<CannedTokenStream> cts1,
        shared_ptr<UnknownType> EndTrimmingFilter)
    : ConditionalTokenFilter(cts1, EndTrimmingFilter::new)
{
  this->outerInstance = outerInstance;
}

bool TestConditionalTokenFilter::ConditionalTokenFilterAnonymousInnerClass::
    shouldFilter() 
{
  return false;
}

TestConditionalTokenFilter::ConditionalTokenFilterAnonymousInnerClass2::
    ConditionalTokenFilterAnonymousInnerClass2(
        shared_ptr<TestConditionalTokenFilter> outerInstance,
        shared_ptr<CannedTokenStream> cts2,
        shared_ptr<UnknownType> EndTrimmingFilter)
    : ConditionalTokenFilter(cts2, EndTrimmingFilter::new)
{
  this->outerInstance = outerInstance;
}

bool TestConditionalTokenFilter::ConditionalTokenFilterAnonymousInnerClass2::
    shouldFilter() 
{
  return true;
}

void TestConditionalTokenFilter::testWrapGraphs() 
{

  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"a b c d e");

  shared_ptr<SynonymMap> sm;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer analyzer
  // = new org.apache.lucene.analysis.MockAnalyzer(random()))
  {
    org::apache::lucene::analysis::Analyzer analyzer =
        org::apache::lucene::analysis::MockAnalyzer(random());
    shared_ptr<SolrSynonymParser> parser =
        make_shared<SolrSynonymParser>(true, true, analyzer);
    parser->parse(make_shared<StringReader>(L"a b, f\nc d, g"));
    sm = parser->build();
  }

  shared_ptr<TokenStream> ts = make_shared<SkipMatchingFilter>(
      shared_from_this(), stream,
      [&](any in_) {
        make_shared<org::apache::lucene::analysis::synonym::SynonymGraphFilter>(
            in_, sm, true);
      },
      L"c");

  assertTokenStreamContents(
      ts, std::deque<wstring>{L"f", L"a", L"b", L"c", L"d", L"e"}, nullptr,
      nullptr, nullptr, std::deque<int>{1, 0, 1, 1, 1, 1},
      std::deque<int>{2, 1, 1, 1, 1, 1});
}

void TestConditionalTokenFilter::testReadaheadWithNoFiltering() throw(
    IOException)
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  wstring input = L"one two three four";

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"", input);
    assertTokenStreamContents(
        ts, std::deque<wstring>{L"one", L"one two", L"two", L"two three",
                                 L"three", L"three four", L"four"});
  }
}

TestConditionalTokenFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestConditionalTokenFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestConditionalTokenFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source = make_shared<ClassicTokenizer>();
  shared_ptr<TokenStream> sink =
      make_shared<ConditionalTokenFilter>(source, [&](any in_) {
        make_shared<org::apache::lucene::analysis::shingle::ShingleFilter>(in_,
                                                                           2);
      })
  {
    @Override bool shouldFilter() throws IOException { return true; }
  }