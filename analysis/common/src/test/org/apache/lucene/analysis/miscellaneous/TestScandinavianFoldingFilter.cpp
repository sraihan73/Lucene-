using namespace std;

#include "TestScandinavianFoldingFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ScandinavianFoldingFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestScandinavianFoldingFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestScandinavianFoldingFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestScandinavianFoldingFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestScandinavianFoldingFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> *const tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const stream =
      make_shared<ScandinavianFoldingFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

void TestScandinavianFoldingFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestScandinavianFoldingFilter::test() 
{

  checkOneTerm(analyzer, L"aeäaeeea",
               L"aaaeea"); // should not cause ArrayOutOfBoundsException

  checkOneTerm(analyzer, L"aeäaeeeae", L"aaaeea");
  checkOneTerm(analyzer, L"aeaeeeae", L"aaeea");

  checkOneTerm(analyzer, L"bøen", L"boen");
  checkOneTerm(analyzer, L"åene", L"aene");

  checkOneTerm(analyzer, L"blåbærsyltetøj", L"blabarsyltetoj");
  checkOneTerm(analyzer, L"blaabaarsyltetoej", L"blabarsyltetoj");
  checkOneTerm(analyzer, L"blåbärsyltetöj", L"blabarsyltetoj");

  checkOneTerm(analyzer, L"raksmorgas", L"raksmorgas");
  checkOneTerm(analyzer, L"räksmörgås", L"raksmorgas");
  checkOneTerm(analyzer, L"ræksmørgås", L"raksmorgas");
  checkOneTerm(analyzer, L"raeksmoergaas", L"raksmorgas");
  checkOneTerm(analyzer, L"ræksmörgaos", L"raksmorgas");

  checkOneTerm(analyzer, L"ab", L"ab");
  checkOneTerm(analyzer, L"ob", L"ob");
  checkOneTerm(analyzer, L"Ab", L"Ab");
  checkOneTerm(analyzer, L"Ob", L"Ob");

  checkOneTerm(analyzer, L"å", L"a");

  checkOneTerm(analyzer, L"aa", L"a");
  checkOneTerm(analyzer, L"aA", L"a");
  checkOneTerm(analyzer, L"ao", L"a");
  checkOneTerm(analyzer, L"aO", L"a");

  checkOneTerm(analyzer, L"AA", L"A");
  checkOneTerm(analyzer, L"Aa", L"A");
  checkOneTerm(analyzer, L"Ao", L"A");
  checkOneTerm(analyzer, L"AO", L"A");

  checkOneTerm(analyzer, L"æ", L"a");
  checkOneTerm(analyzer, L"ä", L"a");

  checkOneTerm(analyzer, L"Æ", L"A");
  checkOneTerm(analyzer, L"Ä", L"A");

  checkOneTerm(analyzer, L"ae", L"a");
  checkOneTerm(analyzer, L"aE", L"a");

  checkOneTerm(analyzer, L"Ae", L"A");
  checkOneTerm(analyzer, L"AE", L"A");

  checkOneTerm(analyzer, L"ö", L"o");
  checkOneTerm(analyzer, L"ø", L"o");
  checkOneTerm(analyzer, L"Ö", L"O");
  checkOneTerm(analyzer, L"Ø", L"O");

  checkOneTerm(analyzer, L"oo", L"o");
  checkOneTerm(analyzer, L"oe", L"o");
  checkOneTerm(analyzer, L"oO", L"o");
  checkOneTerm(analyzer, L"oE", L"o");

  checkOneTerm(analyzer, L"Oo", L"O");
  checkOneTerm(analyzer, L"Oe", L"O");
  checkOneTerm(analyzer, L"OO", L"O");
  checkOneTerm(analyzer, L"OE", L"O");
}

void TestScandinavianFoldingFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestScandinavianFoldingFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestScandinavianFoldingFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestScandinavianFoldingFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ScandinavianFoldingFilter>(tokenizer));
}

void TestScandinavianFoldingFilter::testRandomData() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}
} // namespace org::apache::lucene::analysis::miscellaneous