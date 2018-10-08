using namespace std;

#include "TestICUTransformFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/icu/ICUTransformFilter.h"

namespace org::apache::lucene::analysis::icu
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using com::ibm::icu::text::Transliterator;
using com::ibm::icu::text::UnicodeSet;

void TestICUTransformFilter::testBasicFunctionality() 
{
  checkToken(Transliterator::getInstance(L"Traditional-Simplified"), L"簡化字",
             L"简化字");
  checkToken(Transliterator::getInstance(L"Katakana-Hiragana"), L"ヒラガナ",
             L"ひらがな");
  checkToken(Transliterator::getInstance(L"Fullwidth-Halfwidth"),
             L"アルアノリウ", L"ｱﾙｱﾉﾘｳ");
  checkToken(Transliterator::getInstance(L"Any-Latin"),
             L"Αλφαβητικός Κατάλογος", L"Alphabētikós Katálogos");
  checkToken(Transliterator::getInstance(L"NFD; [:Nonspacing Mark:] Remove"),
             L"Alphabētikós Katálogos", L"Alphabetikos Katalogos");
  checkToken(Transliterator::getInstance(L"Han-Latin"), L"中国", L"zhōng guó");
}

void TestICUTransformFilter::testCustomFunctionality() 
{
  wstring rules = L"a > b; b > c;"; // convert a's to b's and b's to c's
  checkToken(
      Transliterator::createFromRules(L"test", rules, Transliterator::FORWARD),
      L"abacadaba", L"bcbcbdbcb");
}

void TestICUTransformFilter::testCustomFunctionality2() 
{
  wstring rules = L"c { a > b; a > d;"; // convert a's to b's and b's to c's
  checkToken(
      Transliterator::createFromRules(L"test", rules, Transliterator::FORWARD),
      L"caa", L"cbd");
}

void TestICUTransformFilter::testOptimizer() 
{
  wstring rules = L"a > b; b > c;"; // convert a's to b's and b's to c's
  shared_ptr<Transliterator> custom =
      Transliterator::createFromRules(L"test", rules, Transliterator::FORWARD);
  assertTrue(custom->getFilter() == nullptr);
  shared_ptr<KeywordTokenizer> *const input = make_shared<KeywordTokenizer>();
  input->setReader(make_shared<StringReader>(L""));
  make_shared<ICUTransformFilter>(input, custom);
  assertTrue(custom->getFilter().equals(make_shared<UnicodeSet>(L"[ab]")));
}

void TestICUTransformFilter::testOptimizer2() 
{
  checkToken(Transliterator::getInstance(L"Traditional-Simplified; CaseFold"),
             L"ABCDE", L"abcde");
}

void TestICUTransformFilter::testOptimizerSurrogate() 
{
  wstring rules =
      L"\\U00020087 > x;"; // convert CJK UNIFIED IDEOGRAPH-20087 to an x
  shared_ptr<Transliterator> custom =
      Transliterator::createFromRules(L"test", rules, Transliterator::FORWARD);
  assertTrue(custom->getFilter() == nullptr);
  shared_ptr<KeywordTokenizer> *const input = make_shared<KeywordTokenizer>();
  input->setReader(make_shared<StringReader>(L""));
  make_shared<ICUTransformFilter>(input, custom);
  assertTrue(
      custom->getFilter().equals(make_shared<UnicodeSet>(L"[\\U00020087]")));
}

void TestICUTransformFilter::checkToken(
    shared_ptr<Transliterator> transform, const wstring &input,
    const wstring &expected) 
{
  shared_ptr<KeywordTokenizer> *const input1 = make_shared<KeywordTokenizer>();
  input1->setReader(make_shared<StringReader>(input));
  shared_ptr<TokenStream> ts =
      make_shared<ICUTransformFilter>(input1, transform);
  assertTokenStreamContents(ts, std::deque<wstring>{expected});
}

void TestICUTransformFilter::testRandomStrings() 
{
  shared_ptr<Transliterator> *const transform =
      Transliterator::getInstance(L"Any-Latin");
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), transform);
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

TestICUTransformFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestICUTransformFilter> outerInstance,
        shared_ptr<Transliterator> transform)
{
  this->outerInstance = outerInstance;
  this->transform = transform;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUTransformFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ICUTransformFilter>(tokenizer, transform));
}

void TestICUTransformFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestICUTransformFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestICUTransformFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUTransformFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ICUTransformFilter>(
                     tokenizer, Transliterator::getInstance(L"Any-Latin")));
}
} // namespace org::apache::lucene::analysis::icu