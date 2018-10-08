using namespace std;

#include "TestJapaneseKatakanaStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseKatakanaStemFilter.h"

namespace org::apache::lucene::analysis::ja
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;

void TestJapaneseKatakanaStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestJapaneseKatakanaStemFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestJapaneseKatakanaStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseKatakanaStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  // Use a MockTokenizer here since this filter doesn't really depend on
  // Kuromoji
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<JapaneseKatakanaStemFilter>(source));
}

void TestJapaneseKatakanaStemFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestJapaneseKatakanaStemFilter::testStemVariants() 
{
  assertAnalyzesTo(analyzer,
                   L"コピー コーヒー タクシー パーティー パーティ センター",
                   std::deque<wstring>{L"コピー", L"コーヒ", L"タクシ",
                                        L"パーティ", L"パーティ", L"センタ"},
                   std::deque<int>{0, 4, 9, 14, 20, 25},
                   std::deque<int>{3, 8, 13, 19, 24, 29});
}

void TestJapaneseKatakanaStemFilter::testKeyword() 
{
  shared_ptr<CharArraySet> *const exclusionSet =
      make_shared<CharArraySet>(asSet({L"コーヒー"}), false);
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), exclusionSet);
  checkOneTerm(a, L"コーヒー", L"コーヒー");
  delete a;
}

TestJapaneseKatakanaStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestJapaneseKatakanaStemFilter> outerInstance,
        shared_ptr<CharArraySet> exclusionSet)
{
  this->outerInstance = outerInstance;
  this->exclusionSet = exclusionSet;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseKatakanaStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> sink =
      make_shared<SetKeywordMarkerFilter>(source, exclusionSet);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<JapaneseKatakanaStemFilter>(sink));
}

void TestJapaneseKatakanaStemFilter::testUnsupportedHalfWidthVariants() throw(
    IOException)
{
  // The below result is expected since only full-width katakana is supported
  assertAnalyzesTo(analyzer, L"ﾀｸｼｰ", std::deque<wstring>{L"ﾀｸｼｰ"});
}

void TestJapaneseKatakanaStemFilter::testRandomData() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestJapaneseKatakanaStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestJapaneseKatakanaStemFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestJapaneseKatakanaStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseKatakanaStemFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseKatakanaStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::ja