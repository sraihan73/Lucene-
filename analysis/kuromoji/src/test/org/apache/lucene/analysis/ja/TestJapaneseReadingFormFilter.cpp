using namespace std;

#include "TestJapaneseReadingFormFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/cjk/CJKWidthFilter.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseReadingFormFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizer.h"

namespace org::apache::lucene::analysis::ja
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CJKWidthFilter = org::apache::lucene::analysis::cjk::CJKWidthFilter;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestJapaneseReadingFormFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  katakanaAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  romajiAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
}

TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestJapaneseReadingFormFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, true,
      JapaneseTokenizer::Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseReadingFormFilter>(tokenizer, false));
}

TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestJapaneseReadingFormFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, true,
      JapaneseTokenizer::Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseReadingFormFilter>(tokenizer, true));
}

void TestJapaneseReadingFormFilter::tearDown() 
{
  IOUtils::close({katakanaAnalyzer, romajiAnalyzer});
  BaseTokenStreamTestCase::tearDown();
}

void TestJapaneseReadingFormFilter::testKatakanaReadings() 
{
  assertAnalyzesTo(katakanaAnalyzer, L"今夜はロバート先生と話した",
                   std::deque<wstring>{L"コンヤ", L"ハ", L"ロバート",
                                        L"センセイ", L"ト", L"ハナシ", L"タ"});
}

void TestJapaneseReadingFormFilter::testKatakanaReadingsHalfWidth() throw(
    IOException)
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  assertAnalyzesTo(a, L"今夜はﾛﾊﾞｰﾄ先生と話した",
                   std::deque<wstring>{L"コンヤ", L"ハ", L"ロバート",
                                        L"センセイ", L"ト", L"ハナシ", L"タ"});
  delete a;
}

TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestJapaneseReadingFormFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, true,
      JapaneseTokenizer::Mode::SEARCH);
  shared_ptr<TokenStream> stream = make_shared<CJKWidthFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseReadingFormFilter>(stream, false));
}

void TestJapaneseReadingFormFilter::testRomajiReadings() 
{
  assertAnalyzesTo(romajiAnalyzer, L"今夜はロバート先生と話した",
                   std::deque<wstring>{L"kon'ya", L"ha", L"robato", L"sensei",
                                        L"to", L"hanashi", L"ta"});
}

void TestJapaneseReadingFormFilter::testRomajiReadingsHalfWidth() throw(
    IOException)
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this());
  assertAnalyzesTo(a, L"今夜はﾛﾊﾞｰﾄ先生と話した",
                   std::deque<wstring>{L"kon'ya", L"ha", L"robato", L"sensei",
                                        L"to", L"hanashi", L"ta"});
  delete a;
}

TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<TestJapaneseReadingFormFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, true,
      JapaneseTokenizer::Mode::SEARCH);
  shared_ptr<TokenStream> stream = make_shared<CJKWidthFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseReadingFormFilter>(stream, true));
}

void TestJapaneseReadingFormFilter::testRandomData() 
{
  shared_ptr<Random> random = TestJapaneseReadingFormFilter::random();
  checkRandomData(random, katakanaAnalyzer, 1000 * RANDOM_MULTIPLIER);
  checkRandomData(random, romajiAnalyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestJapaneseReadingFormFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass5::
    AnalyzerAnonymousInnerClass5(
        shared_ptr<TestJapaneseReadingFormFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseReadingFormFilter::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseReadingFormFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::ja