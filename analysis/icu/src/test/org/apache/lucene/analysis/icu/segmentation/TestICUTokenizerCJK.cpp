using namespace std;

#include "TestICUTokenizerCJK.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/DefaultICUTokenizerConfig.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/ICUTokenizer.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using org::apache::lucene::util::LuceneTestCase::AwaitsFix;

void TestICUTokenizerCJK::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestICUTokenizerCJK::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestICUTokenizerCJK> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUTokenizerCJK::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(make_shared<ICUTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(),
      make_shared<DefaultICUTokenizerConfig>(true, true)));
}

void TestICUTokenizerCJK::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestICUTokenizerCJK::testSimpleChinese() 
{
  assertAnalyzesTo(
      a, L"我购买了道具和服装。",
      std::deque<wstring>{L"我", L"购买", L"了", L"道具", L"和", L"服装"});
}

void TestICUTokenizerCJK::testTraditionalChinese() 
{
  assertAnalyzesTo(
      a, L"我購買了道具和服裝。",
      std::deque<wstring>{L"我", L"購買", L"了", L"道具", L"和", L"服裝"});
  assertAnalyzesTo(a, L"定義切分字串的基本單位是訂定分詞標準的首要工作",
                   std::deque<wstring>{L"定義", L"切", L"分", L"字串", L"的",
                                        L"基本", L"單位", L"是", L"訂定",
                                        L"分詞", L"標準", L"的", L"首要",
                                        L"工作"});
}

void TestICUTokenizerCJK::testChineseNumerics() 
{
  assertAnalyzesTo(a, L"９４８３", std::deque<wstring>{L"９４８３"});
  assertAnalyzesTo(a, L"院內分機９４８３。",
                   std::deque<wstring>{L"院", L"內", L"分機", L"９４８３"});
  assertAnalyzesTo(a, L"院內分機9483。",
                   std::deque<wstring>{L"院", L"內", L"分機", L"9483"});
}

void TestICUTokenizerCJK::testSimpleJapanese() 
{
  assertAnalyzesTo(a, L"それはまだ実験段階にあります",
                   std::deque<wstring>{L"それ", L"は", L"まだ", L"実験",
                                        L"段階", L"に", L"あり", L"ます"});
}

void TestICUTokenizerCJK::testSimpleJapaneseWithEmoji() 
{
  assertAnalyzesTo(a, L"それはまだ実験段階にあります💩",
                   std::deque<wstring>{L"それ", L"は", L"まだ", L"実験",
                                        L"段階", L"に", L"あり", L"ます",
                                        L"💩"});
}

void TestICUTokenizerCJK::testJapaneseTypes() 
{
  assertAnalyzesTo(a, L"仮名遣い カタカナ",
                   std::deque<wstring>{L"仮名遣い", L"カタカナ"},
                   std::deque<wstring>{L"<IDEOGRAPHIC>", L"<IDEOGRAPHIC>"});
}

void TestICUTokenizerCJK::testKorean() 
{
  // Korean words
  assertAnalyzesTo(a, L"안녕하세요 한글입니다",
                   std::deque<wstring>{L"안녕하세요", L"한글입니다"});
}

void TestICUTokenizerCJK::testKoreanTypes() 
{
  assertAnalyzesTo(a, L"훈민정음", std::deque<wstring>{L"훈민정음"},
                   std::deque<wstring>{L"<HANGUL>"});
}

void TestICUTokenizerCJK::testRandomStrings() 
{
  checkRandomData(random(), a, 10000 * RANDOM_MULTIPLIER);
}

void TestICUTokenizerCJK::testRandomHugeStrings() 
{
  shared_ptr<Random> random = TestICUTokenizerCJK::random();
  checkRandomData(random, a, 100 * RANDOM_MULTIPLIER, 8192);
}
} // namespace org::apache::lucene::analysis::icu::segmentation