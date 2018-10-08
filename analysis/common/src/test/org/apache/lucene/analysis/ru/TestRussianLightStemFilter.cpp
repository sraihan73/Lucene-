using namespace std;

#include "TestRussianLightStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/VocabularyAssert.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ru/RussianLightStemFilter.h"

namespace org::apache::lucene::analysis::ru
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
//    import static org.apache.lucene.analysis.VocabularyAssert.*;

void TestRussianLightStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestRussianLightStemFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestRussianLightStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestRussianLightStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<RussianLightStemFilter>(source));
}

void TestRussianLightStemFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestRussianLightStemFilter::testVocabulary() 
{
  VocabularyAssert::assertVocabulary(
      analyzer, getDataPath(L"rulighttestdata.zip"), L"rulight.txt");
}

void TestRussianLightStemFilter::testKeyword() 
{
  shared_ptr<CharArraySet> *const exclusionSet =
      make_shared<CharArraySet>(asSet({L"энергии"}), false);
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), exclusionSet);
  checkOneTerm(a, L"энергии", L"энергии");
  delete a;
}

TestRussianLightStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestRussianLightStemFilter> outerInstance,
        shared_ptr<CharArraySet> exclusionSet)
{
  this->outerInstance = outerInstance;
  this->exclusionSet = exclusionSet;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestRussianLightStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> sink =
      make_shared<SetKeywordMarkerFilter>(source, exclusionSet);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<RussianLightStemFilter>(sink));
}

void TestRussianLightStemFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestRussianLightStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestRussianLightStemFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestRussianLightStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestRussianLightStemFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<RussianLightStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::ru