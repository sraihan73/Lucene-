using namespace std;

#include "TestCJKAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include "../../../../../../java/org/apache/lucene/analysis/cjk/CJKAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/cjk/CJKBigramFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"

namespace org::apache::lucene::analysis::cjk
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using MappingCharFilter =
    org::apache::lucene::analysis::charfilter::MappingCharFilter;
using NormalizeCharMap =
    org::apache::lucene::analysis::charfilter::NormalizeCharMap;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

void TestCJKAnalyzer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<CJKAnalyzer>();
}

void TestCJKAnalyzer::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestCJKAnalyzer::testJa1() 
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

void TestCJKAnalyzer::testJa2() 
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

void TestCJKAnalyzer::testC() 
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

void TestCJKAnalyzer::testFinalOffset() 
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

void TestCJKAnalyzer::testMix() 
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

void TestCJKAnalyzer::testMix2() 
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

void TestCJKAnalyzer::testNonIdeographic() 
{
  assertAnalyzesTo(
      analyzer, L"一 روبرت موير",
      std::deque<wstring>{L"一", L"روبرت", L"موير"}, std::deque<int>{0, 2, 8},
      std::deque<int>{1, 7, 12},
      std::deque<wstring>{L"<SINGLE>", L"<ALPHANUM>", L"<ALPHANUM>"},
      std::deque<int>{1, 1, 1});
}

void TestCJKAnalyzer::testNonIdeographicNonLetter() 
{
  assertAnalyzesTo(
      analyzer, L"一 رُوبرت موير",
      std::deque<wstring>{L"一", L"رُوبرت", L"موير"}, std::deque<int>{0, 2, 9},
      std::deque<int>{1, 8, 13},
      std::deque<wstring>{L"<SINGLE>", L"<ALPHANUM>", L"<ALPHANUM>"},
      std::deque<int>{1, 1, 1});
}

void TestCJKAnalyzer::testSurrogates() 
{
  assertAnalyzesTo(
      analyzer, L"𩬅艱鍟䇹愯瀛",
      std::deque<wstring>{L"𩬅艱", L"艱鍟", L"鍟䇹", L"䇹愯", L"愯瀛"},
      std::deque<int>{0, 2, 3, 4, 5}, std::deque<int>{3, 4, 5, 6, 7},
      std::deque<wstring>{L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>", L"<DOUBLE>",
                           L"<DOUBLE>"},
      std::deque<int>{1, 1, 1, 1, 1});
}

void TestCJKAnalyzer::testReusableTokenStream() 
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

void TestCJKAnalyzer::testSingleChar() 
{
  assertAnalyzesTo(analyzer, L"一", std::deque<wstring>{L"一"},
                   std::deque<int>{0}, std::deque<int>{1},
                   std::deque<wstring>{L"<SINGLE>"}, std::deque<int>{1});
}

void TestCJKAnalyzer::testTokenStream() 
{
  assertAnalyzesTo(analyzer, L"一丁丂", std::deque<wstring>{L"一丁", L"丁丂"},
                   std::deque<int>{0, 1}, std::deque<int>{2, 3},
                   std::deque<wstring>{L"<DOUBLE>", L"<DOUBLE>"},
                   std::deque<int>{1, 1});
}

void TestCJKAnalyzer::testChangedOffsets() 
{
  shared_ptr<NormalizeCharMap::Builder> *const builder =
      make_shared<NormalizeCharMap::Builder>();
  builder->add(L"a", L"一二");
  builder->add(L"b", L"二三");
  shared_ptr<NormalizeCharMap> *const norm = builder->build();
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), norm);

  assertAnalyzesTo(analyzer, L"ab",
                   std::deque<wstring>{L"一二", L"二二", L"二三"},
                   std::deque<int>{0, 0, 1}, std::deque<int>{1, 1, 2});

  // note: offsets are strange since this is how the charfilter maps them...
  // before bigramming, the 4 tokens look like:
  //   { 0, 0, 1, 1 },
  //   { 0, 1, 1, 2 }

  delete analyzer;
}

TestCJKAnalyzer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestCJKAnalyzer> outerInstance,
    shared_ptr<NormalizeCharMap> norm)
{
  this->outerInstance = outerInstance;
  this->norm = norm;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKAnalyzer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<StandardTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CJKBigramFilter>(tokenizer));
}

shared_ptr<Reader> TestCJKAnalyzer::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<MappingCharFilter>(norm, reader);
}

TestCJKAnalyzer::FakeStandardTokenizer::FakeStandardTokenizer(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TestCJKAnalyzer::FakeStandardTokenizer::incrementToken() 
{
  if (input->incrementToken()) {
    typeAtt->setType(
        StandardTokenizer::TOKEN_TYPES[StandardTokenizer::IDEOGRAPHIC]);
    return true;
  } else {
    return false;
  }
}

void TestCJKAnalyzer::testSingleChar2() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  assertAnalyzesTo(analyzer, L"一", std::deque<wstring>{L"一"},
                   std::deque<int>{0}, std::deque<int>{1},
                   std::deque<wstring>{L"<SINGLE>"}, std::deque<int>{1});
  delete analyzer;
}

TestCJKAnalyzer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestCJKAnalyzer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKAnalyzer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenFilter> filter =
      make_shared<FakeStandardTokenizer>(tokenizer);
  filter = make_shared<StopFilter>(filter, CharArraySet::EMPTY_SET);
  filter = make_shared<CJKBigramFilter>(filter);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

void TestCJKAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> a = make_shared<CJKAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

void TestCJKAnalyzer::testRandomHugeStrings() 
{
  shared_ptr<Analyzer> a = make_shared<CJKAnalyzer>();
  checkRandomData(random(), a, 100 * RANDOM_MULTIPLIER, 8192);
  delete a;
}

void TestCJKAnalyzer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestCJKAnalyzer::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestCJKAnalyzer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKAnalyzer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CJKBigramFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::cjk