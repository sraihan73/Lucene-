using namespace std;

#include "TestSoraniNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ckb/SoraniNormalizationFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"

namespace org::apache::lucene::analysis::ckb
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestSoraniNormalizationFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestSoraniNormalizationFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestSoraniNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSoraniNormalizationFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SoraniNormalizationFilter>(tokenizer));
}

void TestSoraniNormalizationFilter::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestSoraniNormalizationFilter::testY() 
{
  checkOneTerm(a, L"\u064A", L"\u06CC");
  checkOneTerm(a, L"\u0649", L"\u06CC");
  checkOneTerm(a, L"\u06CC", L"\u06CC");
}

void TestSoraniNormalizationFilter::testK() 
{
  checkOneTerm(a, L"\u0643", L"\u06A9");
  checkOneTerm(a, L"\u06A9", L"\u06A9");
}

void TestSoraniNormalizationFilter::testH() 
{
  // initial
  checkOneTerm(a, L"\u0647\u200C", L"\u06D5");
  // medial
  checkOneTerm(a, L"\u0647\u200C\u06A9", L"\u06D5\u06A9");

  checkOneTerm(a, L"\u06BE", L"\u0647");
  checkOneTerm(a, L"\u0629", L"\u06D5");
}

void TestSoraniNormalizationFilter::testFinalH() 
{
  // always (and in final form by def), so frequently omitted
  checkOneTerm(a, L"\u0647\u0647\u0647", L"\u0647\u0647\u06D5");
}

void TestSoraniNormalizationFilter::testRR() 
{
  checkOneTerm(a, L"\u0692", L"\u0695");
}

void TestSoraniNormalizationFilter::testInitialRR() 
{
  // always, so frequently omitted
  checkOneTerm(a, L"\u0631\u0631\u0631", L"\u0695\u0631\u0631");
}

void TestSoraniNormalizationFilter::testRemove() 
{
  checkOneTerm(a, L"\u0640", L"");
  checkOneTerm(a, L"\u064B", L"");
  checkOneTerm(a, L"\u064C", L"");
  checkOneTerm(a, L"\u064D", L"");
  checkOneTerm(a, L"\u064E", L"");
  checkOneTerm(a, L"\u064F", L"");
  checkOneTerm(a, L"\u0650", L"");
  checkOneTerm(a, L"\u0651", L"");
  checkOneTerm(a, L"\u0652", L"");
  // we peek backwards in this case to look for h+200C, ensure this works
  checkOneTerm(a, L"\u200C", L"");
}

void TestSoraniNormalizationFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestSoraniNormalizationFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestSoraniNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSoraniNormalizationFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SoraniNormalizationFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::ckb