using namespace std;

#include "TestJapaneseBaseFormFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseBaseFormFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizer.h"

namespace org::apache::lucene::analysis::ja
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;

void TestJapaneseBaseFormFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestJapaneseBaseFormFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestJapaneseBaseFormFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseBaseFormFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, true,
      JapaneseTokenizer::DEFAULT_MODE);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseBaseFormFilter>(tokenizer));
}

void TestJapaneseBaseFormFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestJapaneseBaseFormFilter::testBasics() 
{
  assertAnalyzesTo(analyzer, L"それはまだ実験段階にあります",
                   std::deque<wstring>{L"それ", L"は", L"まだ", L"実験",
                                        L"段階", L"に", L"ある", L"ます"});
}

void TestJapaneseBaseFormFilter::testKeyword() 
{
  shared_ptr<CharArraySet> *const exclusionSet =
      make_shared<CharArraySet>(asSet({L"あり"}), false);
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), exclusionSet);
  assertAnalyzesTo(a, L"それはまだ実験段階にあります",
                   std::deque<wstring>{L"それ", L"は", L"まだ", L"実験",
                                        L"段階", L"に", L"あり", L"ます"});
  delete a;
}

TestJapaneseBaseFormFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestJapaneseBaseFormFilter> outerInstance,
        shared_ptr<CharArraySet> exclusionSet)
{
  this->outerInstance = outerInstance;
  this->exclusionSet = exclusionSet;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseBaseFormFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, true,
      JapaneseTokenizer::DEFAULT_MODE);
  shared_ptr<TokenStream> sink =
      make_shared<SetKeywordMarkerFilter>(source, exclusionSet);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<JapaneseBaseFormFilter>(sink));
}

void TestJapaneseBaseFormFilter::testEnglish() 
{
  assertAnalyzesTo(analyzer, L"this atest",
                   std::deque<wstring>{L"this", L"atest"});
}

void TestJapaneseBaseFormFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, atLeast(1000));
}

void TestJapaneseBaseFormFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestJapaneseBaseFormFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestJapaneseBaseFormFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseBaseFormFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseBaseFormFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::ja