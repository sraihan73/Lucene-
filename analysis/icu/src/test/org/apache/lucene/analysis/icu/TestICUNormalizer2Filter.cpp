using namespace std;

#include "TestICUNormalizer2Filter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/icu/ICUNormalizer2Filter.h"

namespace org::apache::lucene::analysis::icu
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using com::ibm::icu::text::Normalizer2;

void TestICUNormalizer2Filter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestICUNormalizer2Filter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestICUNormalizer2Filter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUNormalizer2Filter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ICUNormalizer2Filter>(tokenizer));
}

void TestICUNormalizer2Filter::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestICUNormalizer2Filter::testDefaults() 
{
  // case folding
  assertAnalyzesTo(a, L"This is a test",
                   std::deque<wstring>{L"this", L"is", L"a", L"test"});

  // case folding
  assertAnalyzesTo(a, L"Ruß", std::deque<wstring>{L"russ"});

  // case folding
  assertAnalyzesTo(a, L"ΜΆΪΟΣ", std::deque<wstring>{L"μάϊοσ"});
  assertAnalyzesTo(a, L"Μάϊος", std::deque<wstring>{L"μάϊοσ"});

  // supplementary case folding
  assertAnalyzesTo(a, L"𐐖", std::deque<wstring>{L"𐐾"});

  // normalization
  assertAnalyzesTo(a, L"ﴳﴺﰧ", std::deque<wstring>{L"طمطمطم"});

  // removal of default ignorables
  assertAnalyzesTo(a, L"क्‍ष", std::deque<wstring>{L"क्ष"});
}

void TestICUNormalizer2Filter::testAlternate() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

  // decompose EAcute into E + combining Acute
  assertAnalyzesTo(a, L"\u00E9", std::deque<wstring>{L"\u0065\u0301"});
  delete a;
}

TestICUNormalizer2Filter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestICUNormalizer2Filter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUNormalizer2Filter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<ICUNormalizer2Filter>(
          tokenizer, Normalizer2::getInstance(nullptr, L"nfc",
                                              Normalizer2::Mode::DECOMPOSE)));
}

void TestICUNormalizer2Filter::testRandomStrings() 
{
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
}

void TestICUNormalizer2Filter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestICUNormalizer2Filter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestICUNormalizer2Filter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUNormalizer2Filter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ICUNormalizer2Filter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::icu