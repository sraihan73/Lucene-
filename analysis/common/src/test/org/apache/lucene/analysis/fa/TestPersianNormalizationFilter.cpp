using namespace std;

#include "TestPersianNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/fa/PersianNormalizationFilter.h"

namespace org::apache::lucene::analysis::fa
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestPersianNormalizationFilter::testFarsiYeh() 
{
  check(L"های", L"هاي");
}

void TestPersianNormalizationFilter::testYehBarree() 
{
  check(L"هاے", L"هاي");
}

void TestPersianNormalizationFilter::testKeheh() 
{
  check(L"کشاندن", L"كشاندن");
}

void TestPersianNormalizationFilter::testHehYeh() 
{
  check(L"كتابۀ", L"كتابه");
}

void TestPersianNormalizationFilter::testHehHamzaAbove() 
{
  check(L"كتابهٔ", L"كتابه");
}

void TestPersianNormalizationFilter::testHehGoal() 
{
  check(L"زادہ", L"زاده");
}

void TestPersianNormalizationFilter::check(
    const wstring &input, const wstring &expected) 
{
  shared_ptr<MockTokenizer> tokenStream = whitespaceMockTokenizer(input);
  shared_ptr<PersianNormalizationFilter> filter =
      make_shared<PersianNormalizationFilter>(tokenStream);
  assertTokenStreamContents(filter, std::deque<wstring>{expected});
}

void TestPersianNormalizationFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestPersianNormalizationFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestPersianNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPersianNormalizationFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<PersianNormalizationFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::fa