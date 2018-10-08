using namespace std;

#include "TestArabicStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ar/ArabicStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::ar
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;

void TestArabicStemFilter::testAlPrefix() 
{
  check(L"الحسن", L"حسن");
}

void TestArabicStemFilter::testWalPrefix() 
{
  check(L"والحسن", L"حسن");
}

void TestArabicStemFilter::testBalPrefix() 
{
  check(L"بالحسن", L"حسن");
}

void TestArabicStemFilter::testKalPrefix() 
{
  check(L"كالحسن", L"حسن");
}

void TestArabicStemFilter::testFalPrefix() 
{
  check(L"فالحسن", L"حسن");
}

void TestArabicStemFilter::testLlPrefix() 
{
  check(L"للاخر", L"اخر");
}

void TestArabicStemFilter::testWaPrefix() 
{
  check(L"وحسن", L"حسن");
}

void TestArabicStemFilter::testAhSuffix() 
{
  check(L"زوجها", L"زوج");
}

void TestArabicStemFilter::testAnSuffix() 
{
  check(L"ساهدان", L"ساهد");
}

void TestArabicStemFilter::testAtSuffix() 
{
  check(L"ساهدات", L"ساهد");
}

void TestArabicStemFilter::testWnSuffix() 
{
  check(L"ساهدون", L"ساهد");
}

void TestArabicStemFilter::testYnSuffix() 
{
  check(L"ساهدين", L"ساهد");
}

void TestArabicStemFilter::testYhSuffix() 
{
  check(L"ساهديه", L"ساهد");
}

void TestArabicStemFilter::testYpSuffix() 
{
  check(L"ساهدية", L"ساهد");
}

void TestArabicStemFilter::testHSuffix() 
{
  check(L"ساهده", L"ساهد");
}

void TestArabicStemFilter::testPSuffix() 
{
  check(L"ساهدة", L"ساهد");
}

void TestArabicStemFilter::testYSuffix() 
{
  check(L"ساهدي", L"ساهد");
}

void TestArabicStemFilter::testComboPrefSuf() 
{
  check(L"وساهدون", L"ساهد");
}

void TestArabicStemFilter::testComboSuf() 
{
  check(L"ساهدهات", L"ساهد");
}

void TestArabicStemFilter::testShouldntStem() 
{
  check(L"الو", L"الو");
}

void TestArabicStemFilter::testNonArabic() 
{
  check(L"English", L"English");
}

void TestArabicStemFilter::testWithKeywordAttribute() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"ساهدهات");
  shared_ptr<MockTokenizer> tokenStream = whitespaceMockTokenizer(L"ساهدهات");

  shared_ptr<ArabicStemFilter> filter = make_shared<ArabicStemFilter>(
      make_shared<SetKeywordMarkerFilter>(tokenStream, set));
  assertTokenStreamContents(filter, std::deque<wstring>{L"ساهدهات"});
}

void TestArabicStemFilter::check(const wstring &input,
                                 const wstring &expected) 
{
  shared_ptr<MockTokenizer> tokenStream = whitespaceMockTokenizer(input);
  shared_ptr<ArabicStemFilter> filter =
      make_shared<ArabicStemFilter>(tokenStream);
  assertTokenStreamContents(filter, std::deque<wstring>{expected});
}

void TestArabicStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestArabicStemFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestArabicStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestArabicStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ArabicStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::ar