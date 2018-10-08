using namespace std;

#include "TestWithCJKBigramFilter.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/cjk/CJKBigramFilter.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/ICUNormalizer2Filter.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/DefaultICUTokenizerConfig.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/ICUTokenizer.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CJKBigramFilter = org::apache::lucene::analysis::cjk::CJKBigramFilter;
using ICUNormalizer2Filter =
    org::apache::lucene::analysis::icu::ICUNormalizer2Filter;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestWithCJKBigramFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  /*
   * ICUTokenizer+CJKBigramFilter
   */
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  /*
   * ICUTokenizer+ICUNormalizer2Filter+CJKBigramFilter.
   *
   * ICUNormalizer2Filter uses nfkc_casefold by default, so this is a
   * language-independent superset of CJKWidthFilter's foldings.
   */
  analyzer2 = make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
}

TestWithCJKBigramFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestWithCJKBigramFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestWithCJKBigramFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source = make_shared<ICUTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(),
      make_shared<DefaultICUTokenizerConfig>(false, true));
  shared_ptr<TokenStream> result = make_shared<CJKBigramFilter>(source);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<StopFilter>(result, CharArraySet::EMPTY_SET));
}

TestWithCJKBigramFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestWithCJKBigramFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestWithCJKBigramFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source = make_shared<ICUTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(),
      make_shared<DefaultICUTokenizerConfig>(false, true));
  // we put this before the CJKBigramFilter, because the normalization might
  // combine some halfwidth katakana forms, which will affect the bigramming.
  shared_ptr<TokenStream> result = make_shared<ICUNormalizer2Filter>(source);
  result = make_shared<CJKBigramFilter>(result);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<StopFilter>(result, CharArraySet::EMPTY_SET));
}

void TestWithCJKBigramFilter::tearDown() 
{
  IOUtils::close({analyzer, analyzer2});
  BaseTokenStreamTestCase::tearDown();
}

void TestWithCJKBigramFilter::testJa1() 
{
  assertAnalyzesTo(analyzer, L"一二三四五六七八九十",
                   std::deque<wstring>{L"一二", L"二三", L"三四", L"四五",
                                        L"五六", L"六七", L"七八", L"八九",
                                        L"九十"},
                   std::deque<int>{0, 1, 2, 3, 4, 5, 6, 7, 8},
                   std::deque<int>{2, 3, 4, 5, 6, 7, 8, 9, 10},
                   std::deque<wstring>{L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>",
                                        L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>",
                                        L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>"},
                   std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1});
}

void TestWithCJKBigramFilter::testJa2() 
{
  assertAnalyzesTo(analyzer, L"一 二三四 五六七八九 十",
                   std::deque<wstring>{L"一", L"二三", L"三四", L"五六",
                                        L"六七", L"七八", L"八九", L"十"},
                   std::deque<int>{0, 2, 3, 6, 7, 8, 9, 12},
                   std::deque<int>{1, 4, 5, 8, 9, 10, 11, 13},
                   std::deque<wstring>{L"<SINGLE>", L"<DOUBLE>", L"<DOUBLE>",
                                        L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>",
                                        L"<DOUBLE>", L"<SINGLE>"},
                   std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1});
}

void TestWithCJKBigramFilter::testC() 
{
  assertAnalyzesTo(analyzer, L"abc defgh ijklmn opqrstu vwxy z",
                   std::deque<wstring>{L"abc", L"defgh", L"ijklmn", L"opqrstu",
                                        L"vwxy", L"z"},
                   std::deque<int>{0, 4, 10, 17, 25, 30},
                   std::deque<int>{3, 9, 16, 24, 29, 31},
                   std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>",
                                        L"<ALPHANUM>", L"<ALPHANUM>",
                                        L"<ALPHANUM>", L"<ALPHANUM>"},
                   std::deque<int>{1, 1, 1, 1, 1, 1});
}

void TestWithCJKBigramFilter::testFinalOffset() 
{
  assertAnalyzesTo(analyzer, L"あい", std::deque<wstring>{L"あい"},
                   std::deque<int>{0}, std::deque<int>{2},
                   std::deque<wstring>{L"<DOUBLE>"}, std::deque<int>{1});

  assertAnalyzesTo(analyzer, L"あい   ", std::deque<wstring>{L"あい"},
                   std::deque<int>{0}, std::deque<int>{2},
                   std::deque<wstring>{L"<DOUBLE>"}, std::deque<int>{1});

  assertAnalyzesTo(analyzer, L"test", std::deque<wstring>{L"test"},
                   std::deque<int>{0}, std::deque<int>{4},
                   std::deque<wstring>{L"<ALPHANUM>"}, std::deque<int>{1});

  assertAnalyzesTo(analyzer, L"test   ", std::deque<wstring>{L"test"},
                   std::deque<int>{0}, std::deque<int>{4},
                   std::deque<wstring>{L"<ALPHANUM>"}, std::deque<int>{1});

  assertAnalyzesTo(
      analyzer, L"あいtest", std::deque<wstring>{L"あい", L"test"},
      std::deque<int>{0, 2}, std::deque<int>{2, 6},
      std::deque<wstring>{L"<DOUBLE>", L"<ALPHANUM>"}, std::deque<int>{1, 1});

  assertAnalyzesTo(
      analyzer, L"testあい    ", std::deque<wstring>{L"test", L"あい"},
      std::deque<int>{0, 4}, std::deque<int>{4, 6},
      std::deque<wstring>{L"<ALPHANUM>", L"<DOUBLE>"}, std::deque<int>{1, 1});
}

void TestWithCJKBigramFilter::testMix() 
{
  assertAnalyzesTo(analyzer, L"あいうえおabcかきくけこ",
                   std::deque<wstring>{L"あい", L"いう", L"うえ", L"えお",
                                        L"abc", L"かき", L"きく", L"くけ",
                                        L"けこ"},
                   std::deque<int>{0, 1, 2, 3, 5, 8, 9, 10, 11},
                   std::deque<int>{2, 3, 4, 5, 8, 10, 11, 12, 13},
                   std::deque<wstring>{L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>",
                                        L"<DOUBLE>", L"<ALPHANUM>", L"<DOUBLE>",
                                        L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>"},
                   std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1});
}

void TestWithCJKBigramFilter::testMix2() 
{
  assertAnalyzesTo(
      analyzer, L"あいうえおabんcかきくけ こ",
      std::deque<wstring>{L"あい", L"いう", L"うえ", L"えお", L"ab", L"ん",
                           L"c", L"かき", L"きく", L"くけ", L"こ"},
      std::deque<int>{0, 1, 2, 3, 5, 7, 8, 9, 10, 11, 14},
      std::deque<int>{2, 3, 4, 5, 7, 8, 9, 11, 12, 13, 15},
      std::deque<wstring>{L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>",
                           L"<ALPHANUM>", L"<SINGLE>", L"<ALPHANUM>",
                           L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>", L"<SINGLE>"},
      std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
}

void TestWithCJKBigramFilter::testNonIdeographic() 
{
  assertAnalyzesTo(
      analyzer, L"一 روبرت موير",
      std::deque<wstring>{L"一", L"روبرت", L"موير"}, std::deque<int>{0, 2, 8},
      std::deque<int>{1, 7, 12},
      std::deque<wstring>{L"<SINGLE>", L"<ALPHANUM>", L"<ALPHANUM>"},
      std::deque<int>{1, 1, 1});
}

void TestWithCJKBigramFilter::testNonIdeographicNonLetter() 
{
  assertAnalyzesTo(
      analyzer, L"一 رُوبرت موير",
      std::deque<wstring>{L"一", L"رُوبرت", L"موير"}, std::deque<int>{0, 2, 9},
      std::deque<int>{1, 8, 13},
      std::deque<wstring>{L"<SINGLE>", L"<ALPHANUM>", L"<ALPHANUM>"},
      std::deque<int>{1, 1, 1});
}

void TestWithCJKBigramFilter::testSurrogates() 
{
  assertAnalyzesTo(
      analyzer, L"𩬅艱鍟䇹愯瀛",
      std::deque<wstring>{L"𩬅艱", L"艱鍟", L"鍟䇹", L"䇹愯", L"愯瀛"},
      std::deque<int>{0, 2, 3, 4, 5}, std::deque<int>{3, 4, 5, 6, 7},
      std::deque<wstring>{L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>",
                           L"<DOUBLE>"},
      std::deque<int>{1, 1, 1, 1, 1});
}

void TestWithCJKBigramFilter::testReusableTokenStream() 
{
  assertAnalyzesTo(analyzer, L"あいうえおabcかきくけこ",
                   std::deque<wstring>{L"あい", L"いう", L"うえ", L"えお",
                                        L"abc", L"かき", L"きく", L"くけ",
                                        L"けこ"},
                   std::deque<int>{0, 1, 2, 3, 5, 8, 9, 10, 11},
                   std::deque<int>{2, 3, 4, 5, 8, 10, 11, 12, 13},
                   std::deque<wstring>{L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>",
                                        L"<DOUBLE>", L"<ALPHANUM>", L"<DOUBLE>",
                                        L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>"},
                   std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1});

  assertAnalyzesTo(
      analyzer, L"あいうえおabんcかきくけ こ",
      std::deque<wstring>{L"あい", L"いう", L"うえ", L"えお", L"ab", L"ん",
                           L"c", L"かき", L"きく", L"くけ", L"こ"},
      std::deque<int>{0, 1, 2, 3, 5, 7, 8, 9, 10, 11, 14},
      std::deque<int>{2, 3, 4, 5, 7, 8, 9, 11, 12, 13, 15},
      std::deque<wstring>{L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>",
                           L"<ALPHANUM>", L"<SINGLE>", L"<ALPHANUM>",
                           L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>", L"<SINGLE>"},
      std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
}

void TestWithCJKBigramFilter::testSingleChar() 
{
  assertAnalyzesTo(analyzer, L"一", std::deque<wstring>{L"一"},
                   std::deque<int>{0}, std::deque<int>{1},
                   std::deque<wstring>{L"<SINGLE>"}, std::deque<int>{1});
}

void TestWithCJKBigramFilter::testTokenStream() 
{
  assertAnalyzesTo(analyzer, L"一丁丂", std::deque<wstring>{L"一丁", L"丁丂"},
                   std::deque<int>{0, 1}, std::deque<int>{2, 3},
                   std::deque<wstring>{L"<DOUBLE>", L"<DOUBLE>"},
                   std::deque<int>{1, 1});
}
} // namespace org::apache::lucene::analysis::icu::segmentation