using namespace std;

#include "TestPatternReplaceFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/pattern/PatternReplaceFilter.h"

namespace org::apache::lucene::analysis::pattern
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestPatternReplaceFilter::testReplaceAll() 
{
  wstring input = L"aabfooaabfooabfoob ab caaaaaaaaab";
  shared_ptr<TokenStream> ts = make_shared<PatternReplaceFilter>(
      whitespaceMockTokenizer(input), Pattern::compile(L"a*b"), L"-", true);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"-foo-foo-foo-", L"-", L"c-"});
}

void TestPatternReplaceFilter::testReplaceFirst() 
{
  wstring input = L"aabfooaabfooabfoob ab caaaaaaaaab";
  shared_ptr<TokenStream> ts = make_shared<PatternReplaceFilter>(
      whitespaceMockTokenizer(input), Pattern::compile(L"a*b"), L"-", false);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"-fooaabfooabfoob", L"-", L"c-"});
}

void TestPatternReplaceFilter::testStripFirst() 
{
  wstring input = L"aabfooaabfooabfoob ab caaaaaaaaab";
  shared_ptr<TokenStream> ts = make_shared<PatternReplaceFilter>(
      whitespaceMockTokenizer(input), Pattern::compile(L"a*b"), nullptr, false);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"fooaabfooabfoob", L"", L"c"});
}

void TestPatternReplaceFilter::testStripAll() 
{
  wstring input = L"aabfooaabfooabfoob ab caaaaaaaaab";
  shared_ptr<TokenStream> ts = make_shared<PatternReplaceFilter>(
      whitespaceMockTokenizer(input), Pattern::compile(L"a*b"), nullptr, true);
  assertTokenStreamContents(ts, std::deque<wstring>{L"foofoofoo", L"", L"c"});
}

void TestPatternReplaceFilter::testReplaceAllWithBackRef() 
{
  wstring input = L"aabfooaabfooabfoob ab caaaaaaaaab";
  shared_ptr<TokenStream> ts = make_shared<PatternReplaceFilter>(
      whitespaceMockTokenizer(input), Pattern::compile(L"(a*)b"), L"$1\\$",
      true);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"aa$fooaa$fooa$foo$", L"a$", L"caaaaaaaaa$"});
}

void TestPatternReplaceFilter::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;

  shared_ptr<Analyzer> b =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkRandomData(random(), b, 1000 * RANDOM_MULTIPLIER);
  delete b;
}

TestPatternReplaceFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestPatternReplaceFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPatternReplaceFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> filter = make_shared<PatternReplaceFilter>(
      tokenizer, Pattern::compile(L"a"), L"b", false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

TestPatternReplaceFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestPatternReplaceFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPatternReplaceFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> filter = make_shared<PatternReplaceFilter>(
      tokenizer, Pattern::compile(L"a"), L"b", true);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

void TestPatternReplaceFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestPatternReplaceFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestPatternReplaceFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPatternReplaceFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<PatternReplaceFilter>(
                     tokenizer, Pattern::compile(L"a"), L"b", true));
}
} // namespace org::apache::lucene::analysis::pattern