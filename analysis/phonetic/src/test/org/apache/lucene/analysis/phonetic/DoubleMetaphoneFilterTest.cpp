using namespace std;

#include "DoubleMetaphoneFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/DoubleMetaphoneFilter.h"

namespace org::apache::lucene::analysis::phonetic
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using TestUtil = org::apache::lucene::util::TestUtil;

void DoubleMetaphoneFilterTest::testSize4FalseInject() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"international");
  shared_ptr<TokenStream> filter =
      make_shared<DoubleMetaphoneFilter>(stream, 4, false);
  assertTokenStreamContents(filter, std::deque<wstring>{L"ANTR"});
}

void DoubleMetaphoneFilterTest::testSize4TrueInject() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"international");
  shared_ptr<TokenStream> filter =
      make_shared<DoubleMetaphoneFilter>(stream, 4, true);
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"international", L"ANTR"});
}

void DoubleMetaphoneFilterTest::testAlternateInjectFalse() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"Kuczewski");
  shared_ptr<TokenStream> filter =
      make_shared<DoubleMetaphoneFilter>(stream, 4, false);
  assertTokenStreamContents(filter, std::deque<wstring>{L"KSSK", L"KXFS"});
}

void DoubleMetaphoneFilterTest::testSize8FalseInject() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"international");
  shared_ptr<TokenStream> filter =
      make_shared<DoubleMetaphoneFilter>(stream, 8, false);
  assertTokenStreamContents(filter, std::deque<wstring>{L"ANTRNXNL"});
}

void DoubleMetaphoneFilterTest::testNonConvertableStringsWithInject() throw(
    runtime_error)
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"12345 #$%@#^%&");
  shared_ptr<TokenStream> filter =
      make_shared<DoubleMetaphoneFilter>(stream, 8, true);
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"12345", L"#$%@#^%&"});
}

void DoubleMetaphoneFilterTest::testNonConvertableStringsWithoutInject() throw(
    runtime_error)
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"12345 #$%@#^%&");
  shared_ptr<TokenStream> filter =
      make_shared<DoubleMetaphoneFilter>(stream, 8, false);
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"12345", L"#$%@#^%&"});

  // should have something after the stream
  stream = whitespaceMockTokenizer(L"12345 #$%@#^%& hello");
  filter = make_shared<DoubleMetaphoneFilter>(stream, 8, false);
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"12345", L"#$%@#^%&", L"HL"});
}

void DoubleMetaphoneFilterTest::testRandom() 
{
  constexpr int codeLen = TestUtil::nextInt(random(), 1, 8);
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), codeLen);
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;

  shared_ptr<Analyzer> b =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), codeLen);
  checkRandomData(random(), b, 1000 * RANDOM_MULTIPLIER);
  delete b;
}

DoubleMetaphoneFilterTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<DoubleMetaphoneFilterTest> outerInstance, int codeLen)
{
  this->outerInstance = outerInstance;
  this->codeLen = codeLen;
}

shared_ptr<Analyzer::TokenStreamComponents>
DoubleMetaphoneFilterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<DoubleMetaphoneFilter>(tokenizer, codeLen, false));
}

DoubleMetaphoneFilterTest::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<DoubleMetaphoneFilterTest> outerInstance, int codeLen)
{
  this->outerInstance = outerInstance;
  this->codeLen = codeLen;
}

shared_ptr<Analyzer::TokenStreamComponents>
DoubleMetaphoneFilterTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<DoubleMetaphoneFilter>(tokenizer, codeLen, true));
}

void DoubleMetaphoneFilterTest::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

DoubleMetaphoneFilterTest::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<DoubleMetaphoneFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
DoubleMetaphoneFilterTest::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<DoubleMetaphoneFilter>(tokenizer, 8,
                                                    random()->nextBoolean()));
}
} // namespace org::apache::lucene::analysis::phonetic