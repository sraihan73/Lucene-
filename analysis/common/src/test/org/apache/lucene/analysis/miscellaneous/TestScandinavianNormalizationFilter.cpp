using namespace std;

#include "TestScandinavianNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ScandinavianNormalizationFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestScandinavianNormalizationFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestScandinavianNormalizationFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestScandinavianNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestScandinavianNormalizationFilter::AnalyzerAnonymousInnerClass::
    createComponents(const wstring &field)
{
  shared_ptr<Tokenizer> *const tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const stream =
      make_shared<ScandinavianNormalizationFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

void TestScandinavianNormalizationFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestScandinavianNormalizationFilter::test() 
{

  checkOneTerm(analyzer, L"aeäaeeea",
               L"æææeea"); // should not cause ArrayIndexOutOfBoundsException

  checkOneTerm(analyzer, L"aeäaeeeae", L"æææeeæ");
  checkOneTerm(analyzer, L"aeaeeeae", L"ææeeæ");

  checkOneTerm(analyzer, L"bøen", L"bøen");
  checkOneTerm(analyzer, L"bOEen", L"bØen");
  checkOneTerm(analyzer, L"åene", L"åene");

  checkOneTerm(analyzer, L"blåbærsyltetøj", L"blåbærsyltetøj");
  checkOneTerm(analyzer, L"blaabaersyltetöj", L"blåbærsyltetøj");
  checkOneTerm(analyzer, L"räksmörgås", L"ræksmørgås");
  checkOneTerm(analyzer, L"raeksmörgaos", L"ræksmørgås");
  checkOneTerm(analyzer, L"raeksmörgaas", L"ræksmørgås");
  checkOneTerm(analyzer, L"raeksmoergås", L"ræksmørgås");

  checkOneTerm(analyzer, L"ab", L"ab");
  checkOneTerm(analyzer, L"ob", L"ob");
  checkOneTerm(analyzer, L"Ab", L"Ab");
  checkOneTerm(analyzer, L"Ob", L"Ob");

  checkOneTerm(analyzer, L"å", L"å");

  checkOneTerm(analyzer, L"aa", L"å");
  checkOneTerm(analyzer, L"aA", L"å");
  checkOneTerm(analyzer, L"ao", L"å");
  checkOneTerm(analyzer, L"aO", L"å");

  checkOneTerm(analyzer, L"AA", L"Å");
  checkOneTerm(analyzer, L"Aa", L"Å");
  checkOneTerm(analyzer, L"Ao", L"Å");
  checkOneTerm(analyzer, L"AO", L"Å");

  checkOneTerm(analyzer, L"æ", L"æ");
  checkOneTerm(analyzer, L"ä", L"æ");

  checkOneTerm(analyzer, L"Æ", L"Æ");
  checkOneTerm(analyzer, L"Ä", L"Æ");

  checkOneTerm(analyzer, L"ae", L"æ");
  checkOneTerm(analyzer, L"aE", L"æ");

  checkOneTerm(analyzer, L"Ae", L"Æ");
  checkOneTerm(analyzer, L"AE", L"Æ");

  checkOneTerm(analyzer, L"ö", L"ø");
  checkOneTerm(analyzer, L"ø", L"ø");
  checkOneTerm(analyzer, L"Ö", L"Ø");
  checkOneTerm(analyzer, L"Ø", L"Ø");

  checkOneTerm(analyzer, L"oo", L"ø");
  checkOneTerm(analyzer, L"oe", L"ø");
  checkOneTerm(analyzer, L"oO", L"ø");
  checkOneTerm(analyzer, L"oE", L"ø");

  checkOneTerm(analyzer, L"Oo", L"Ø");
  checkOneTerm(analyzer, L"Oe", L"Ø");
  checkOneTerm(analyzer, L"OO", L"Ø");
  checkOneTerm(analyzer, L"OE", L"Ø");
}

void TestScandinavianNormalizationFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestScandinavianNormalizationFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestScandinavianNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestScandinavianNormalizationFilter::AnalyzerAnonymousInnerClass2::
    createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ScandinavianNormalizationFilter>(tokenizer));
}

void TestScandinavianNormalizationFilter::testRandomData() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}
} // namespace org::apache::lucene::analysis::miscellaneous