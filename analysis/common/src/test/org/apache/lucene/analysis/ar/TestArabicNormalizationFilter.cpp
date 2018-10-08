using namespace std;

#include "TestArabicNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ar/ArabicNormalizationFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"

namespace org::apache::lucene::analysis::ar
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestArabicNormalizationFilter::testAlifMadda() 
{
  check(L"آجن", L"اجن");
}

void TestArabicNormalizationFilter::testAlifHamzaAbove() 
{
  check(L"أحمد", L"احمد");
}

void TestArabicNormalizationFilter::testAlifHamzaBelow() 
{
  check(L"إعاذ", L"اعاذ");
}

void TestArabicNormalizationFilter::testAlifMaksura() 
{
  check(L"بنى", L"بني");
}

void TestArabicNormalizationFilter::testTehMarbuta() 
{
  check(L"فاطمة", L"فاطمه");
}

void TestArabicNormalizationFilter::testTatweel() 
{
  check(L"روبرـــــت", L"روبرت");
}

void TestArabicNormalizationFilter::testFatha() 
{
  check(L"مَبنا", L"مبنا");
}

void TestArabicNormalizationFilter::testKasra() 
{
  check(L"علِي", L"علي");
}

void TestArabicNormalizationFilter::testDamma() 
{
  check(L"بُوات", L"بوات");
}

void TestArabicNormalizationFilter::testFathatan() 
{
  check(L"ولداً", L"ولدا");
}

void TestArabicNormalizationFilter::testKasratan() 
{
  check(L"ولدٍ", L"ولد");
}

void TestArabicNormalizationFilter::testDammatan() 
{
  check(L"ولدٌ", L"ولد");
}

void TestArabicNormalizationFilter::testSukun() 
{
  check(L"نلْسون", L"نلسون");
}

void TestArabicNormalizationFilter::testShaddah() 
{
  check(L"هتميّ", L"هتمي");
}

void TestArabicNormalizationFilter::check(
    const wstring &input, const wstring &expected) 
{
  shared_ptr<MockTokenizer> tokenStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenStream->setReader(make_shared<StringReader>(input));
  shared_ptr<ArabicNormalizationFilter> filter =
      make_shared<ArabicNormalizationFilter>(tokenStream);
  assertTokenStreamContents(filter, std::deque<wstring>{expected});
}

void TestArabicNormalizationFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestArabicNormalizationFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestArabicNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestArabicNormalizationFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ArabicNormalizationFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::ar