using namespace std;

#include "TestSoraniStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ckb/SoraniAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ckb/SoraniNormalizationFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ckb/SoraniStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"

namespace org::apache::lucene::analysis::ckb
{
//    import static
//    org.apache.lucene.analysis.VocabularyAssert.assertVocabulary;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestSoraniStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<SoraniAnalyzer>();
}

void TestSoraniStemFilter::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestSoraniStemFilter::testIndefiniteSingular() 
{
  checkOneTerm(a, L"پیاوێک", L"پیاو");    // -ek
  checkOneTerm(a, L"دەرگایەک", L"دەرگا"); // -yek
}

void TestSoraniStemFilter::testDefiniteSingular() 
{
  checkOneTerm(a, L"پیاوەكە", L"پیاو");  // -aka
  checkOneTerm(a, L"دەرگاكە", L"دەرگا"); // -ka
}

void TestSoraniStemFilter::testDemonstrativeSingular() 
{
  checkOneTerm(a, L"کتاویە", L"کتاوی");  // -a
  checkOneTerm(a, L"دەرگایە", L"دەرگا"); // -ya
}

void TestSoraniStemFilter::testIndefinitePlural() 
{
  checkOneTerm(a, L"پیاوان", L"پیاو");    // -An
  checkOneTerm(a, L"دەرگایان", L"دەرگا"); // -yAn
}

void TestSoraniStemFilter::testDefinitePlural() 
{
  checkOneTerm(a, L"پیاوەکان", L"پیاو");  // -akAn
  checkOneTerm(a, L"دەرگاکان", L"دەرگا"); // -kAn
}

void TestSoraniStemFilter::testDemonstrativePlural() 
{
  checkOneTerm(a, L"پیاوانە", L"پیاو");    // -Ana
  checkOneTerm(a, L"دەرگایانە", L"دەرگا"); // -yAna
}

void TestSoraniStemFilter::testEzafe() 
{
  checkOneTerm(a, L"هۆتیلی", L"هۆتیل");   // singular
  checkOneTerm(a, L"هۆتیلێکی", L"هۆتیل"); // indefinite
  checkOneTerm(a, L"هۆتیلانی", L"هۆتیل"); // plural
}

void TestSoraniStemFilter::testPostpositions() 
{
  checkOneTerm(a, L"دوورەوە", L"دوور");      // -awa
  checkOneTerm(a, L"نیوەشەودا", L"نیوەشەو"); // -dA
  checkOneTerm(a, L"سۆرانا", L"سۆران");      // -A
}

void TestSoraniStemFilter::testPossessives() 
{
  checkOneTerm(a, L"پارەمان", L"پارە"); // -mAn
  checkOneTerm(a, L"پارەتان", L"پارە"); // -tAn
  checkOneTerm(a, L"پارەیان", L"پارە"); // -yAn
}

void TestSoraniStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestSoraniStemFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestSoraniStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSoraniStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SoraniStemFilter>(tokenizer));
}

void TestSoraniStemFilter::testVocabulary() 
{
  // top 8k words or so: freq > 1000

  // just normalization+stem, we are testing that the stemming doesn't break.
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  assertVocabulary(a, getDataPath(L"ckbtestdata.zip"), L"testdata.txt");
  delete a;
}

TestSoraniStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(shared_ptr<TestSoraniStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSoraniStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  shared_ptr<TokenStream> stream =
      make_shared<SoraniNormalizationFilter>(tokenizer);
  stream = make_shared<SoraniStemFilter>(stream);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}
} // namespace org::apache::lucene::analysis::ckb