using namespace std;

#include "TestCJKWidthFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/cjk/CJKWidthFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"

namespace org::apache::lucene::analysis::cjk
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestCJKWidthFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestCJKWidthFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestCJKWidthFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKWidthFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<CJKWidthFilter>(source));
}

void TestCJKWidthFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestCJKWidthFilter::testFullWidthASCII() 
{
  assertAnalyzesTo(analyzer, L"Ｔｅｓｔ １２３４",
                   std::deque<wstring>{L"Test", L"1234"},
                   std::deque<int>{0, 5}, std::deque<int>{4, 9});
}

void TestCJKWidthFilter::testHalfWidthKana() 
{
  assertAnalyzesTo(analyzer, L"ｶﾀｶﾅ", std::deque<wstring>{L"カタカナ"});
  assertAnalyzesTo(analyzer, L"ｳﾞｨｯﾂ", std::deque<wstring>{L"ヴィッツ"});
  assertAnalyzesTo(analyzer, L"ﾊﾟﾅｿﾆｯｸ", std::deque<wstring>{L"パナソニック"});
}

void TestCJKWidthFilter::testRandomData() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestCJKWidthFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestCJKWidthFilter::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestCJKWidthFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKWidthFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CJKWidthFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::cjk