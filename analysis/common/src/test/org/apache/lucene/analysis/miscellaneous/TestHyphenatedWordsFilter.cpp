using namespace std;

#include "TestHyphenatedWordsFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/HyphenatedWordsFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestHyphenatedWordsFilter::testHyphenatedWords() 
{
  wstring input = L"ecologi-\r\ncal devel-\r\n\r\nop "
                  L"compre-\u0009hensive-hands-on and ecologi-\ncal";
  // first test
  shared_ptr<TokenStream> ts =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(input));
  ts = make_shared<HyphenatedWordsFilter>(ts);
  assertTokenStreamContents(ts, std::deque<wstring>{L"ecological", L"develop",
                                                     L"comprehensive-hands-on",
                                                     L"and", L"ecological"});
}

void TestHyphenatedWordsFilter::testHyphenAtEnd() 
{
  wstring input = L"ecologi-\r\ncal devel-\r\n\r\nop "
                  L"compre-\u0009hensive-hands-on and ecology-";
  // first test
  shared_ptr<TokenStream> ts =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(input));
  ts = make_shared<HyphenatedWordsFilter>(ts);
  assertTokenStreamContents(ts, std::deque<wstring>{L"ecological", L"develop",
                                                     L"comprehensive-hands-on",
                                                     L"and", L"ecology-"});
}

void TestHyphenatedWordsFilter::testOffsets() 
{
  wstring input = L"abc- def geh 1234- 5678-";
  shared_ptr<TokenStream> ts =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(input));
  ts = make_shared<HyphenatedWordsFilter>(ts);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"abcdef", L"geh", L"12345678-"},
      std::deque<int>{0, 9, 13}, std::deque<int>{8, 12, 24});
}

void TestHyphenatedWordsFilter::testRandomString() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

TestHyphenatedWordsFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestHyphenatedWordsFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestHyphenatedWordsFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<HyphenatedWordsFilter>(tokenizer));
}

void TestHyphenatedWordsFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestHyphenatedWordsFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestHyphenatedWordsFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestHyphenatedWordsFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<HyphenatedWordsFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::miscellaneous