using namespace std;

#include "TestMyanmarSyllable.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/DefaultICUTokenizerConfig.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/ICUTokenizer.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestMyanmarSyllable::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestMyanmarSyllable::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestMyanmarSyllable> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestMyanmarSyllable::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<ICUTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(),
      make_shared<DefaultICUTokenizerConfig>(false, false));
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

void TestMyanmarSyllable::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestMyanmarSyllable::testBasics() 
{
  assertAnalyzesTo(
      a, L"သက်ဝင်လှုပ်ရှားစေပြီး",
      std::deque<wstring>{L"သက်", L"ဝင်", L"လှုပ်", L"ရှား", L"စေ", L"ပြီး"});
}

void TestMyanmarSyllable::testC() 
{
  assertAnalyzesTo(a, L"ကက", std::deque<wstring>{L"က", L"က"});
}

void TestMyanmarSyllable::testCF() 
{
  assertAnalyzesTo(a, L"ကံကံ", std::deque<wstring>{L"ကံ", L"ကံ"});
}

void TestMyanmarSyllable::testCCA() 
{
  assertAnalyzesTo(a, L"ကင်ကင်", std::deque<wstring>{L"ကင်", L"ကင်"});
}

void TestMyanmarSyllable::testCCAF() 
{
  assertAnalyzesTo(a, L"ကင်းကင်း", std::deque<wstring>{L"ကင်း", L"ကင်း"});
}

void TestMyanmarSyllable::testCV() 
{
  assertAnalyzesTo(a, L"ကာကာ", std::deque<wstring>{L"ကာ", L"ကာ"});
}

void TestMyanmarSyllable::testCVF() 
{
  assertAnalyzesTo(a, L"ကားကား", std::deque<wstring>{L"ကား", L"ကား"});
}

void TestMyanmarSyllable::testCVVA() 
{
  assertAnalyzesTo(a, L"ကော်ကော်", std::deque<wstring>{L"ကော်", L"ကော်"});
}

void TestMyanmarSyllable::testCVVCA() 
{
  assertAnalyzesTo(a, L"ကောင်ကောင်", std::deque<wstring>{L"ကောင်", L"ကောင်"});
}

void TestMyanmarSyllable::testCVVCAF() 
{
  assertAnalyzesTo(a, L"ကောင်းကောင်း", std::deque<wstring>{L"ကောင်း", L"ကောင်း"});
}

void TestMyanmarSyllable::testCM() 
{
  assertAnalyzesTo(a, L"ကျကျ", std::deque<wstring>{L"ကျ", L"ကျ"});
}

void TestMyanmarSyllable::testCMF() 
{
  assertAnalyzesTo(a, L"ကျံကျံ", std::deque<wstring>{L"ကျံ", L"ကျံ"});
}

void TestMyanmarSyllable::testCMCA() 
{
  assertAnalyzesTo(a, L"ကျင်ကျင်", std::deque<wstring>{L"ကျင်", L"ကျင်"});
}

void TestMyanmarSyllable::testCMCAF() 
{
  assertAnalyzesTo(a, L"ကျင်းကျင်း", std::deque<wstring>{L"ကျင်း", L"ကျင်း"});
}

void TestMyanmarSyllable::testCMV() 
{
  assertAnalyzesTo(a, L"ကျာကျာ", std::deque<wstring>{L"ကျာ", L"ကျာ"});
}

void TestMyanmarSyllable::testCMVF() 
{
  assertAnalyzesTo(a, L"ကျားကျား", std::deque<wstring>{L"ကျား", L"ကျား"});
}

void TestMyanmarSyllable::testCMVVA() 
{
  assertAnalyzesTo(a, L"ကျော်ကျော်", std::deque<wstring>{L"ကျော်", L"ကျော်"});
}

void TestMyanmarSyllable::testCMVVCA() 
{
  assertAnalyzesTo(a, L"ကြောင်ကြောင်", std::deque<wstring>{L"ကြောင်", L"ကြောင်"});
}

void TestMyanmarSyllable::testCMVVCAF() 
{
  assertAnalyzesTo(a, L"ကြောင်းကြောင်း",
                   std::deque<wstring>{L"ကြောင်း", L"ကြောင်း"});
}

void TestMyanmarSyllable::testI() 
{
  assertAnalyzesTo(a, L"ဪဪ", std::deque<wstring>{L"ဪ", L"ဪ"});
}

void TestMyanmarSyllable::testE() 
{
  assertAnalyzesTo(a, L"ဣဣ", std::deque<wstring>{L"ဣ", L"ဣ"});
}
} // namespace org::apache::lucene::analysis::icu::segmentation