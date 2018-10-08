using namespace std;

#include "TestICUFoldingFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/icu/ICUFoldingFilter.h"

namespace org::apache::lucene::analysis::icu
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestICUFoldingFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestICUFoldingFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestICUFoldingFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUFoldingFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ICUFoldingFilter>(tokenizer));
}

void TestICUFoldingFilter::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestICUFoldingFilter::testDefaults() 
{
  // case folding
  assertAnalyzesTo(a, L"This is a test",
                   std::deque<wstring>{L"this", L"is", L"a", L"test"});

  // case folding
  assertAnalyzesTo(a, L"Ruß", std::deque<wstring>{L"russ"});

  // case folding with accent removal
  assertAnalyzesTo(a, L"ΜΆΪΟΣ", std::deque<wstring>{L"μαιοσ"});
  assertAnalyzesTo(a, L"Μάϊος", std::deque<wstring>{L"μαιοσ"});

  // supplementary case folding
  assertAnalyzesTo(a, L"𐐖", std::deque<wstring>{L"𐐾"});

  // normalization
  assertAnalyzesTo(a, L"ﴳﴺﰧ", std::deque<wstring>{L"طمطمطم"});

  // removal of default ignorables
  assertAnalyzesTo(a, L"क्‍ष", std::deque<wstring>{L"कष"});

  // removal of latin accents (composed)
  assertAnalyzesTo(a, L"résumé", std::deque<wstring>{L"resume"});

  // removal of latin accents (decomposed)
  assertAnalyzesTo(a, L"re\u0301sume\u0301", std::deque<wstring>{L"resume"});

  // fold native digits
  assertAnalyzesTo(a, L"৭০৬", std::deque<wstring>{L"706"});

  // ascii-folding-filter type stuff
  assertAnalyzesTo(a, L"đis is cræzy",
                   std::deque<wstring>{L"dis", L"is", L"craezy"});

  // proper downcasing of Turkish dotted-capital I
  // (according to default case folding rules)
  assertAnalyzesTo(a, L"ELİF", std::deque<wstring>{L"elif"});

  // handling of decomposed combining-dot-above
  assertAnalyzesTo(a, L"eli\u0307f", std::deque<wstring>{L"elif"});
}

void TestICUFoldingFilter::testRandomStrings() 
{
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
}

void TestICUFoldingFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestICUFoldingFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(shared_ptr<TestICUFoldingFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUFoldingFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ICUFoldingFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::icu