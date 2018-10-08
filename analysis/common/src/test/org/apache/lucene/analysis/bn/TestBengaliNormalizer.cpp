using namespace std;

#include "TestBengaliNormalizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/bn/BengaliNormalizationFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/bn/BengaliNormalizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"

namespace org::apache::lucene::analysis::bn
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestBengaliNormalizer::testChndrobindu() 
{
  check(L"চাঁদ", L"চাদ");
}

void TestBengaliNormalizer::testRosshoIKar() 
{
  check(L"বাড়ী", L"বারি");
  check(L"তীর", L"তির");
}

void TestBengaliNormalizer::testRosshoUKar() 
{
  check(L"ভূল", L"ভুল");
  check(L"অনূপ", L"অনুপ");
}

void TestBengaliNormalizer::testNga() 
{
  check(L"বাঙলা", L"বাংলা");
}

void TestBengaliNormalizer::testJaPhaala() 
{
  check(L"ব্যাক্তি", L"বেক্তি");
  check(L"সন্ধ্যা", L"সন্ধা");
}

void TestBengaliNormalizer::testBaPhalaa() 
{
  check(L"স্বদেশ", L"সদেস");
  check(L"তত্ত্ব", L"তত্ত");
  check(L"বিশ্ব", L"বিসস");
}

void TestBengaliNormalizer::testVisarga() 
{
  check(L"দুঃখ", L"দুখখ");
  check(L"উঃ", L"উহ");
  check(L"পুনঃ", L"পুন");
}

void TestBengaliNormalizer::testBasics() 
{
  check(L"কণা", L"কনা");
  check(L"শরীর", L"সরির");
  check(L"বাড়ি", L"বারি");
}

void TestBengaliNormalizer::testRandom() 
{
  shared_ptr<BengaliNormalizer> normalizer = make_shared<BengaliNormalizer>();
  for (int i = 0; i < 100000; i++) {
    wstring randomBengali =
        TestUtil::randomSimpleStringRange(random(), L'\u0980', L'\u09FF', 7);
    try {
      int newLen = normalizer->normalize(randomBengali.toCharArray(),
                                         randomBengali.length());
      assertTrue(newLen >= 0); // should not return negative length
      assertTrue(
          newLen <=
          randomBengali.length()); // should not increase length of string
    } catch (const runtime_error &e) {
      System::err::println(L"normalizer failed on input: '" + randomBengali +
                           L"' (" + escape(randomBengali) + L")");
      throw e;
    }
  }
}

void TestBengaliNormalizer::check(const wstring &input,
                                  const wstring &output) 
{
  shared_ptr<Tokenizer> tokenizer = whitespaceMockTokenizer(input);
  shared_ptr<TokenFilter> tf =
      make_shared<BengaliNormalizationFilter>(tokenizer);
  assertTokenStreamContents(tf, std::deque<wstring>{output});
}

void TestBengaliNormalizer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestBengaliNormalizer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestBengaliNormalizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBengaliNormalizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<BengaliNormalizationFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::bn