using namespace std;

#include "TestJapaneseTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/UnicodeUtil.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockGraphTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/GraphvizFormatter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/dict/ConnectionCosts.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/dict/UserDictionary.h"

namespace org::apache::lucene::analysis::ja
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockGraphTokenFilter =
    org::apache::lucene::analysis::MockGraphTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Mode = org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode;
using ConnectionCosts =
    org::apache::lucene::analysis::ja::dict::ConnectionCosts;
using UserDictionary = org::apache::lucene::analysis::ja::dict::UserDictionary;
using namespace org::apache::lucene::analysis::ja::tokenattributes;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

shared_ptr<UserDictionary> TestJapaneseTokenizer::readDict()
{
  shared_ptr<InputStream> is =
      TestJapaneseTokenizer::typeid->getResourceAsStream(L"userdict.txt");
  if (is == nullptr) {
    throw runtime_error(L"Cannot find userdict.txt in test classpath!");
  }
  try {
    try {
      shared_ptr<Reader> reader =
          make_shared<InputStreamReader>(is, StandardCharsets::UTF_8);
      return UserDictionary::open(reader);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      is->close();
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

shared_ptr<JapaneseTokenizer>
TestJapaneseTokenizer::makeTokenizer(bool discardPunctuation, Mode mode)
{
  return make_shared<JapaneseTokenizer>(newAttributeFactory(), readDict(),
                                        discardPunctuation, mode);
}

shared_ptr<Analyzer>
TestJapaneseTokenizer::makeAnalyzer(shared_ptr<Tokenizer> t)
{
  return make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), t);
}

TestJapaneseTokenizer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestJapaneseTokenizer> outerInstance, shared_ptr<Tokenizer> t)
{
  this->outerInstance = outerInstance;
  this->t = t;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseTokenizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(t, t);
}

void TestJapaneseTokenizer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  analyzerNormal =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  analyzerNormalNBest =
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this());
  analyzerNoPunct =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this());
  extendedModeAnalyzerNoPunct =
      make_shared<AnalyzerAnonymousInnerClass6>(shared_from_this());
}

TestJapaneseTokenizer::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestJapaneseTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseTokenizer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), readDict(), false,
      Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

TestJapaneseTokenizer::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestJapaneseTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseTokenizer::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), readDict(), false,
      Mode::NORMAL);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

TestJapaneseTokenizer::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<TestJapaneseTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseTokenizer::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<JapaneseTokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), readDict(), false,
      Mode::NORMAL);
  tokenizer->setNBestCost(2000);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

TestJapaneseTokenizer::AnalyzerAnonymousInnerClass5::
    AnalyzerAnonymousInnerClass5(
        shared_ptr<TestJapaneseTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseTokenizer::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), readDict(), true,
      Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

TestJapaneseTokenizer::AnalyzerAnonymousInnerClass6::
    AnalyzerAnonymousInnerClass6(
        shared_ptr<TestJapaneseTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseTokenizer::AnalyzerAnonymousInnerClass6::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), readDict(), true,
      Mode::EXTENDED);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestJapaneseTokenizer::tearDown() 
{
  IOUtils::close(
      {analyzer, analyzerNormal, analyzerNoPunct, extendedModeAnalyzerNoPunct});
  BaseTokenStreamTestCase::tearDown();
}

void TestJapaneseTokenizer::testNormalMode() 
{
  assertAnalyzesTo(analyzerNormal, L"シニアソフトウェアエンジニア",
                   std::deque<wstring>{L"シニアソフトウェアエンジニア"});
}

void TestJapaneseTokenizer::testNormalModeNbest() 
{
  shared_ptr<JapaneseTokenizer> t = makeTokenizer(true, Mode::NORMAL);
  shared_ptr<Analyzer> a = makeAnalyzer(t);

  t->setNBestCost(2000);
  assertAnalyzesTo(a, L"シニアソフトウェアエンジニア",
                   std::deque<wstring>{L"シニア",
                                        L"シニアソフトウェアエンジニア",
                                        L"ソフトウェア", L"エンジニア"});

  t->setNBestCost(5000);
  assertAnalyzesTo(a, L"シニアソフトウェアエンジニア",
                   std::deque<wstring>{
                       L"シニア", L"シニアソフトウェアエンジニア", L"ソフト",
                       L"ソフトウェア", L"ウェア", L"エンジニア"});

  t->setNBestCost(0);
  assertAnalyzesTo(a, L"数学部長谷川",
                   std::deque<wstring>{L"数学", L"部長", L"谷川"});

  t->setNBestCost(3000);
  assertAnalyzesTo(
      a, L"数学部長谷川",
      std::deque<wstring>{L"数学", L"部", L"部長", L"長谷川", L"谷川"});

  t->setNBestCost(0);
  assertAnalyzesTo(a, L"経済学部長",
                   std::deque<wstring>{L"経済", L"学", L"部長"});

  t->setNBestCost(2000);
  assertAnalyzesTo(
      a, L"経済学部長",
      std::deque<wstring>{L"経済", L"経済学部", L"学", L"部長", L"長"});

  t->setNBestCost(0);
  assertAnalyzesTo(a, L"成田空港、米原油流出",
                   std::deque<wstring>{L"成田空港", L"米", L"原油", L"流出"});

  t->setNBestCost(4000);
  assertAnalyzesTo(a, L"成田空港、米原油流出",
                   std::deque<wstring>{L"成田空港", L"米", L"米原", L"原油",
                                        L"油", L"流出"});
}

void TestJapaneseTokenizer::testSearchModeNbest() 
{
  shared_ptr<JapaneseTokenizer> t = makeTokenizer(true, Mode::SEARCH);
  shared_ptr<Analyzer> a = makeAnalyzer(t);

  t->setNBestCost(0);
  assertAnalyzesTo(a, L"成田空港、米原油流出",
                   std::deque<wstring>{L"成田", L"成田空港", L"空港", L"米",
                                        L"原油", L"流出"});

  t->setNBestCost(4000);
  assertAnalyzesTo(a, L"成田空港、米原油流出",
                   std::deque<wstring>{L"成田", L"成田空港", L"空港", L"米",
                                        L"米原", L"原油", L"油", L"流出"});
}

deque<wstring>
TestJapaneseTokenizer::makeTokenList(shared_ptr<Analyzer> a,
                                     const wstring &in_) 
{
  deque<wstring> deque = deque<wstring>();
  shared_ptr<TokenStream> ts = a->tokenStream(L"dummy", in_);
  shared_ptr<CharTermAttribute> termAtt =
      ts->getAttribute(CharTermAttribute::typeid);

  ts->reset();
  while (ts->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    deque.push_back(termAtt->toString());
  }
  ts->end();
  delete ts;
  return deque;
}

bool TestJapaneseTokenizer::checkToken(
    shared_ptr<Analyzer> a, const wstring &in_,
    const wstring &requitedToken) 
{
  return makeTokenList(a, in_).find(requitedToken) != -1;
}

void TestJapaneseTokenizer::testNBestCost() 
{
  shared_ptr<JapaneseTokenizer> t = makeTokenizer(true, Mode::NORMAL);
  shared_ptr<Analyzer> a = makeAnalyzer(t);

  t->setNBestCost(0);
  assertFalse(L"学部 is not a token of 数学部長谷川",
              checkToken(a, L"数学部長谷川", L"学部"));

  assertTrue(L"cost calculated /数学部長谷川-学部/",
             0 <= t->calcNBestCost(L"/数学部長谷川-学部/"));
  t->setNBestCost(t->calcNBestCost(L"/数学部長谷川-学部/"));
  assertTrue(L"学部 is a token of 数学部長谷川",
             checkToken(a, L"数学部長谷川", L"学部"));

  assertTrue(L"cost calculated /数学部長谷川-数/成田空港-成/",
             0 <= t->calcNBestCost(L"/数学部長谷川-数/成田空港-成/"));
  t->setNBestCost(t->calcNBestCost(L"/数学部長谷川-数/成田空港-成/"));
  assertTrue(L"数 is a token of 数学部長谷川",
             checkToken(a, L"数学部長谷川", L"数"));
  assertTrue(L"成 is a token of 成田空港", checkToken(a, L"成田空港", L"成"));
}

void TestJapaneseTokenizer::testDecomposition1() 
{
  assertAnalyzesTo(
      analyzerNoPunct,
      wstring(L"本来は、貧困層の女性や子供に医療保護を提供するために創設された"
              L"制度である、") +
          L"アメリカ低所得者医療援助制度が、今日では、その予算の約３分の１を老"
          L"人に費やしている。",
      std::deque<wstring>{
          L"本来", L"は",   L"貧困",     L"層",   L"の",   L"女性", L"や",
          L"子供", L"に",   L"医療",     L"保護", L"を",   L"提供", L"する",
          L"ため", L"に",   L"創設",     L"さ",   L"れ",   L"た",   L"制度",
          L"で",   L"ある", L"アメリカ", L"低",   L"所得", L"者",   L"医療",
          L"援助", L"制度", L"が",       L"今日", L"で",   L"は",   L"その",
          L"予算", L"の",   L"約",       L"３",   L"分の", L"１",   L"を",
          L"老人", L"に",   L"費やし",   L"て",   L"いる"},
      std::deque<int>{0,  2,  4,  6,  7,  8,  10, 11, 13, 14, 16, 18,
                       19, 21, 23, 25, 26, 28, 29, 30, 31, 33, 34, 37,
                       41, 42, 44, 45, 47, 49, 51, 53, 55, 56, 58, 60,
                       62, 63, 64, 65, 67, 68, 69, 71, 72, 75, 76},
      std::deque<int>{2,  3,  6,  7,  8,  10, 11, 13, 14, 16, 18, 19,
                       21, 23, 25, 26, 28, 29, 30, 31, 33, 34, 36, 41,
                       42, 44, 45, 47, 49, 51, 52, 55, 56, 57, 60, 62,
                       63, 64, 65, 67, 68, 69, 71, 72, 75, 76, 78});
}

void TestJapaneseTokenizer::testDecomposition2() 
{
  assertAnalyzesTo(
      analyzerNoPunct, L"麻薬の密売は根こそぎ絶やさなければならない",
      std::deque<wstring>{L"麻薬", L"の", L"密売", L"は", L"根こそぎ",
                           L"絶やさ", L"なけれ", L"ば", L"なら", L"ない"},
      std::deque<int>{0, 2, 3, 5, 6, 10, 13, 16, 17, 19},
      std::deque<int>{2, 3, 5, 6, 10, 13, 16, 17, 19, 21});
}

void TestJapaneseTokenizer::testDecomposition3() 
{
  assertAnalyzesTo(
      analyzerNoPunct, L"魔女狩大将マシュー・ホプキンス。",
      std::deque<wstring>{L"魔女", L"狩", L"大将", L"マシュー", L"ホプキンス"},
      std::deque<int>{0, 2, 3, 5, 10}, std::deque<int>{2, 3, 5, 9, 15});
}

void TestJapaneseTokenizer::testDecomposition4() 
{
  assertAnalyzesTo(
      analyzer, L"これは本ではない",
      std::deque<wstring>{L"これ", L"は", L"本", L"で", L"は", L"ない"},
      std::deque<int>{0, 2, 3, 4, 5, 6}, std::deque<int>{2, 3, 4, 5, 6, 8});
}

void TestJapaneseTokenizer::testDecomposition5() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("bogus",
  // "くよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよ"))
  {
    org::apache::lucene::analysis::TokenStream ts = analyzer->tokenStream(
        L"bogus", L"くよくよくよくよくよくよくよくよくよくよくよくよくよくよく"
                  L"よくよくよくよくよくよ");
    ts->reset();
    while (ts->incrementToken()) {
    }
    ts->end();
  }
}

void TestJapaneseTokenizer::testTwoSentences() 
{
  /*
  //TokenStream ts = a.tokenStream("foo",
  "妹の咲子です。俺と年子で、今受験生です。"); TokenStream ts =
  analyzer.tokenStream("foo",
  "&#x250cdf66<!--\"<!--#<!--;?><!--#<!--#><!---->?>-->;"); ts.reset();
  CharTermAttribute termAtt = ts.addAttribute(CharTermAttribute.class);
  while(ts.incrementToken()) {
    System.out.println("  " + termAtt.toString());
  }
  System.out.println("DONE PARSE\n\n");
  */

  assertAnalyzesTo(
      analyzerNoPunct,
      L"魔女狩大将マシュー・ホプキンス。 魔女狩大将マシュー・ホプキンス。",
      std::deque<wstring>{L"魔女", L"狩", L"大将", L"マシュー", L"ホプキンス",
                           L"魔女", L"狩", L"大将", L"マシュー", L"ホプキンス"},
      std::deque<int>{0, 2, 3, 5, 10, 17, 19, 20, 22, 27},
      std::deque<int>{2, 3, 5, 9, 15, 19, 20, 22, 26, 32});
}

void TestJapaneseTokenizer::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 500 * RANDOM_MULTIPLIER);
  checkRandomData(random(), analyzerNoPunct, 500 * RANDOM_MULTIPLIER);
  checkRandomData(random(), analyzerNormalNBest, 500 * RANDOM_MULTIPLIER);
}

void TestJapaneseTokenizer::testRandomHugeStrings() 
{
  shared_ptr<Random> random = TestJapaneseTokenizer::random();
  checkRandomData(random, analyzer, 20 * RANDOM_MULTIPLIER, 8192);
  checkRandomData(random, analyzerNoPunct, 20 * RANDOM_MULTIPLIER, 8192);
  checkRandomData(random, analyzerNormalNBest, 20 * RANDOM_MULTIPLIER, 8192);
}

void TestJapaneseTokenizer::testRandomHugeStringsMockGraphAfter() throw(
    runtime_error)
{
  // Randomly inject graph tokens after JapaneseTokenizer:
  shared_ptr<Random> random = TestJapaneseTokenizer::random();
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass7>(shared_from_this());
  checkRandomData(random, analyzer, 20 * RANDOM_MULTIPLIER, 8192);
  delete analyzer;
}

TestJapaneseTokenizer::AnalyzerAnonymousInnerClass7::
    AnalyzerAnonymousInnerClass7(
        shared_ptr<TestJapaneseTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseTokenizer::AnalyzerAnonymousInnerClass7::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), readDict(), false,
      Mode::SEARCH);
  shared_ptr<TokenStream> graph =
      make_shared<MockGraphTokenFilter>(random(), tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, graph);
}

void TestJapaneseTokenizer::testLargeDocReliability() 
{
  for (int i = 0; i < 10; i++) {
    wstring s = TestUtil::randomUnicodeString(random(), 10000);
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts
    // = analyzer.tokenStream("foo", s))
    {
      org::apache::lucene::analysis::TokenStream ts =
          analyzer->tokenStream(L"foo", s);
      ts->reset();
      while (ts->incrementToken()) {
      }
      ts->end();
    }
  }
}

void TestJapaneseTokenizer::testSurrogates() 
{
  assertAnalyzesTo(
      analyzer, L"𩬅艱鍟䇹愯瀛",
      std::deque<wstring>{L"𩬅", L"艱", L"鍟", L"䇹", L"愯", L"瀛"});
}

void TestJapaneseTokenizer::testSurrogates2() 
{
  int numIterations = atLeast(10000);
  for (int i = 0; i < numIterations; i++) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << i << endl;
    }
    wstring s = TestUtil::randomUnicodeString(random(), 100);
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts
    // = analyzer.tokenStream("foo", s))
    {
      org::apache::lucene::analysis::TokenStream ts =
          analyzer->tokenStream(L"foo", s);
      shared_ptr<CharTermAttribute> termAtt =
          ts->addAttribute(CharTermAttribute::typeid);
      ts->reset();
      while (ts->incrementToken()) {
        assertTrue(UnicodeUtil::validUTF16String(termAtt));
      }
      ts->end();
    }
  }
}

void TestJapaneseTokenizer::testOnlyPunctuation() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzerNoPunct.tokenStream("foo", "。、。。"))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzerNoPunct->tokenStream(L"foo", L"。、。。");
    ts->reset();
    assertFalse(ts->incrementToken());
    ts->end();
  }
}

void TestJapaneseTokenizer::testOnlyPunctuationExtended() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // extendedModeAnalyzerNoPunct.tokenStream("foo", "......"))
  {
    org::apache::lucene::analysis::TokenStream ts =
        extendedModeAnalyzerNoPunct->tokenStream(L"foo", L"......");
    ts->reset();
    assertFalse(ts->incrementToken());
    ts->end();
  }
}

void TestJapaneseTokenizer::testEnd() 
{
  assertTokenStreamContents(
      analyzerNoPunct->tokenStream(L"foo", L"これは本ではない"),
      std::deque<wstring>{L"これ", L"は", L"本", L"で", L"は", L"ない"},
      std::deque<int>{0, 2, 3, 4, 5, 6}, std::deque<int>{2, 3, 4, 5, 6, 8},
      optional<int>(8));

  assertTokenStreamContents(
      analyzerNoPunct->tokenStream(L"foo", L"これは本ではない    "),
      std::deque<wstring>{L"これ", L"は", L"本", L"で", L"は", L"ない"},
      std::deque<int>{0, 2, 3, 4, 5, 6, 8},
      std::deque<int>{2, 3, 4, 5, 6, 8, 9}, optional<int>(12));
}

void TestJapaneseTokenizer::testUserDict() 
{
  // Not a great test because w/o userdict.txt the
  // segmentation is the same:
  assertTokenStreamContents(
      analyzer->tokenStream(L"foo", L"関西国際空港に行った"),
      std::deque<wstring>{L"関西", L"国際", L"空港", L"に", L"行っ", L"た"},
      std::deque<int>{0, 2, 4, 6, 7, 9}, std::deque<int>{2, 4, 6, 7, 9, 10},
      optional<int>(10));
}

void TestJapaneseTokenizer::testUserDict2() 
{
  // Better test: w/o userdict the segmentation is different:
  assertTokenStreamContents(
      analyzer->tokenStream(L"foo", L"朝青龍"), std::deque<wstring>{L"朝青龍"},
      std::deque<int>{0}, std::deque<int>{3}, optional<int>(3));
}

void TestJapaneseTokenizer::testUserDict3() 
{
  // Test entry that breaks into multiple tokens:
  assertTokenStreamContents(analyzer->tokenStream(L"foo", L"abcd"),
                            std::deque<wstring>{L"a", L"b", L"cd"},
                            std::deque<int>{0, 1, 2},
                            std::deque<int>{1, 2, 4}, optional<int>(4));
}

void TestJapaneseTokenizer::testSegmentation() 
{
  // Skip tests for Michelle Kwan -- UniDic segments Kwan as ク ワン
  //   std::wstring input =
  //   "ミシェル・クワンが優勝しました。スペースステーションに行きます。うたがわしい。";
  //   std::wstring[] surfaceForms = {
  //        "ミシェル", "・", "クワン", "が", "優勝", "し", "まし", "た", "。",
  //        "スペース", "ステーション", "に", "行き", "ます", "。",
  //        "うたがわしい", "。"
  //   };
  wstring input = L"スペースステーションに行きます。うたがわしい。";
  std::deque<wstring> surfaceForms = {
      L"スペース", L"ステーション", L"に",           L"行き",
      L"ます",     L"。",           L"うたがわしい", L"。"};
  assertAnalyzesTo(analyzer, input, surfaceForms);
}

void TestJapaneseTokenizer::testLatticeToDot() 
{
  shared_ptr<GraphvizFormatter> *const gv2 =
      make_shared<GraphvizFormatter>(ConnectionCosts::getInstance());
  shared_ptr<Analyzer> *const analyzer =
      make_shared<AnalyzerAnonymousInnerClass8>(shared_from_this(), gv2);

  wstring input = L"スペースステーションに行きます。うたがわしい。";
  std::deque<wstring> surfaceForms = {
      L"スペース", L"ステーション", L"に",           L"行き",
      L"ます",     L"。",           L"うたがわしい", L"。"};
  assertAnalyzesTo(analyzer, input, surfaceForms);

  assertTrue(gv2->finish().find(L"22.0") != wstring::npos);
  delete analyzer;
}

TestJapaneseTokenizer::AnalyzerAnonymousInnerClass8::
    AnalyzerAnonymousInnerClass8(
        shared_ptr<TestJapaneseTokenizer> outerInstance,
        shared_ptr<org::apache::lucene::analysis::ja::GraphvizFormatter> gv2)
{
  this->outerInstance = outerInstance;
  this->gv2 = gv2;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseTokenizer::AnalyzerAnonymousInnerClass8::createComponents(
    const wstring &fieldName)
{
  shared_ptr<JapaneseTokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), readDict(), false,
      Mode::SEARCH);
  tokenizer->setGraphvizFormatter(gv2);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestJapaneseTokenizer::assertReadings(
    const wstring &input, deque<wstring> &readings) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("ignored", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"ignored", input);
    shared_ptr<ReadingAttribute> readingAtt =
        ts->addAttribute(ReadingAttribute::typeid);
    ts->reset();
    for (wstring reading : readings) {
      assertTrue(ts->incrementToken());
      TestUtil::assertEquals(reading, readingAtt->getReading());
    }
    assertFalse(ts->incrementToken());
    ts->end();
  }
}

void TestJapaneseTokenizer::assertPronunciations(
    const wstring &input, deque<wstring> &pronunciations) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("ignored", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"ignored", input);
    shared_ptr<ReadingAttribute> readingAtt =
        ts->addAttribute(ReadingAttribute::typeid);
    ts->reset();
    for (wstring pronunciation : pronunciations) {
      assertTrue(ts->incrementToken());
      TestUtil::assertEquals(pronunciation, readingAtt->getPronunciation());
    }
    assertFalse(ts->incrementToken());
    ts->end();
  }
}

void TestJapaneseTokenizer::assertBaseForms(
    const wstring &input, deque<wstring> &baseForms) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("ignored", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"ignored", input);
    shared_ptr<BaseFormAttribute> baseFormAtt =
        ts->addAttribute(BaseFormAttribute::typeid);
    ts->reset();
    for (wstring baseForm : baseForms) {
      assertTrue(ts->incrementToken());
      TestUtil::assertEquals(baseForm, baseFormAtt->getBaseForm());
    }
    assertFalse(ts->incrementToken());
    ts->end();
  }
}

void TestJapaneseTokenizer::assertInflectionTypes(
    const wstring &input, deque<wstring> &inflectionTypes) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("ignored", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"ignored", input);
    shared_ptr<InflectionAttribute> inflectionAtt =
        ts->addAttribute(InflectionAttribute::typeid);
    ts->reset();
    for (wstring inflectionType : inflectionTypes) {
      assertTrue(ts->incrementToken());
      TestUtil::assertEquals(inflectionType,
                             inflectionAtt->getInflectionType());
    }
    assertFalse(ts->incrementToken());
    ts->end();
  }
}

void TestJapaneseTokenizer::assertInflectionForms(
    const wstring &input, deque<wstring> &inflectionForms) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("ignored", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"ignored", input);
    shared_ptr<InflectionAttribute> inflectionAtt =
        ts->addAttribute(InflectionAttribute::typeid);
    ts->reset();
    for (wstring inflectionForm : inflectionForms) {
      assertTrue(ts->incrementToken());
      TestUtil::assertEquals(inflectionForm,
                             inflectionAtt->getInflectionForm());
    }
    assertFalse(ts->incrementToken());
    ts->end();
  }
}

void TestJapaneseTokenizer::assertPartsOfSpeech(
    const wstring &input, deque<wstring> &partsOfSpeech) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream("ignored", input))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(L"ignored", input);
    shared_ptr<PartOfSpeechAttribute> partOfSpeechAtt =
        ts->addAttribute(PartOfSpeechAttribute::typeid);
    ts->reset();
    for (wstring partOfSpeech : partsOfSpeech) {
      assertTrue(ts->incrementToken());
      TestUtil::assertEquals(partOfSpeech, partOfSpeechAtt->getPartOfSpeech());
    }
    assertFalse(ts->incrementToken());
    ts->end();
  }
}

void TestJapaneseTokenizer::testReadings() 
{
  assertReadings(L"寿司が食べたいです。",
                 {L"スシ", L"ガ", L"タベ", L"タイ", L"デス", L"。"});
}

void TestJapaneseTokenizer::testReadings2() 
{
  assertReadings(L"多くの学生が試験に落ちた。",
                 {L"オオク", L"ノ", L"ガクセイ", L"ガ", L"シケン", L"ニ",
                  L"オチ", L"タ", L"。"});
}

void TestJapaneseTokenizer::testPronunciations() 
{
  assertPronunciations(L"寿司が食べたいです。",
                       {L"スシ", L"ガ", L"タベ", L"タイ", L"デス", L"。"});
}

void TestJapaneseTokenizer::testPronunciations2() 
{
  // pronunciation differs from reading here
  assertPronunciations(L"多くの学生が試験に落ちた。",
                       {L"オーク", L"ノ", L"ガクセイ", L"ガ", L"シケン", L"ニ",
                        L"オチ", L"タ", L"。"});
}

void TestJapaneseTokenizer::testBasicForms() 
{
  assertBaseForms(L"それはまだ実験段階にあります。",
                  {L"", L"", L"", L"", L"", L"", L"ある", L"", L""});
}

void TestJapaneseTokenizer::testInflectionTypes() 
{
  assertInflectionTypes(
      L"それはまだ実験段階にあります。",
      {L"", L"", L"", L"", L"", L"", L"五段・ラ行", L"特殊・マス", L""});
}

void TestJapaneseTokenizer::testInflectionForms() 
{
  assertInflectionForms(
      L"それはまだ実験段階にあります。",
      {L"", L"", L"", L"", L"", L"", L"連用形", L"基本形", L""});
}

void TestJapaneseTokenizer::testPartOfSpeech() 
{
  assertPartsOfSpeech(L"それはまだ実験段階にあります。",
                      {L"名詞-代名詞-一般", L"助詞-係助詞", L"副詞-助詞類接続",
                       L"名詞-サ変接続", L"名詞-一般", L"助詞-格助詞-一般",
                       L"動詞-自立", L"助動詞", L"記号-句点"});
}

void TestJapaneseTokenizer::testYabottai() 
{
  assertAnalyzesTo(analyzer, L"やぼったい",
                   std::deque<wstring>{L"やぼったい"});
}

void TestJapaneseTokenizer::testTsukitosha() 
{
  assertAnalyzesTo(analyzer, L"突き通しゃ",
                   std::deque<wstring>{L"突き通しゃ"});
}

void TestJapaneseTokenizer::testBocchan() 
{
  doTestBocchan(1);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testBocchanBig() throws Exception
void TestJapaneseTokenizer::testBocchanBig() 
{
  doTestBocchan(100);
}

void TestJapaneseTokenizer::doTestBocchan(int numIterations) throw(
    runtime_error)
{
  shared_ptr<LineNumberReader> reader =
      make_shared<LineNumberReader>(make_shared<InputStreamReader>(
          this->getClass().getResourceAsStream(L"bocchan.utf-8"),
          StandardCharsets::UTF_8));
  wstring line = reader->readLine();
  reader->close();

  if (VERBOSE) {
    wcout << L"Test for Bocchan without pre-splitting sentences" << endl;
  }

  /*
  if (numIterations > 1) {
    // warmup
    for (int i = 0; i < numIterations; i++) {
      final TokenStream ts = analyzer.tokenStream("ignored", line);
      ts.reset();
      while(ts.incrementToken());
    }
  }
  */

  int64_t totalStart = System::currentTimeMillis();
  for (int i = 0; i < numIterations; i++) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts
    // = analyzer.tokenStream("ignored", line))
    {
      org::apache::lucene::analysis::TokenStream ts =
          analyzer->tokenStream(L"ignored", line);
      ts->reset();
      while (ts->incrementToken()) {
        ;
      }
      ts->end();
    }
  }
  std::deque<wstring> sentences = line.split(L"、|。");
  if (VERBOSE) {
    wcout << L"Total time : " << (System::currentTimeMillis() - totalStart)
          << endl;
    wcout << L"Test for Bocchan with pre-splitting sentences ("
          << sentences.size() << L" sentences)" << endl;
  }
  totalStart = System::currentTimeMillis();
  for (int i = 0; i < numIterations; i++) {
    for (auto sentence : sentences) {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try
      // (org.apache.lucene.analysis.TokenStream ts =
      // analyzer.tokenStream("ignored", sentence))
      {
        org::apache::lucene::analysis::TokenStream ts =
            analyzer->tokenStream(L"ignored", sentence);
        ts->reset();
        while (ts->incrementToken()) {
          ;
        }
        ts->end();
      }
    }
  }
  if (VERBOSE) {
    wcout << L"Total time : " << (System::currentTimeMillis() - totalStart)
          << endl;
  }
}

void TestJapaneseTokenizer::testWithPunctuation() 
{
  assertAnalyzesTo(analyzerNoPunct, L"羽田。空港",
                   std::deque<wstring>{L"羽田", L"空港"},
                   std::deque<int>{1, 1});
}

void TestJapaneseTokenizer::testCompoundOverPunctuation() 
{
  assertAnalyzesToPositions(
      analyzerNoPunct, L"dεε϶ϢϏΎϷΞͺ羽田",
      std::deque<wstring>{L"d", L"ε", L"ε", L"ϢϏΎϷΞͺ", L"羽田"},
      std::deque<int>{1, 1, 1, 1, 1}, std::deque<int>{1, 1, 1, 1, 1});
}

void TestJapaneseTokenizer::testEmptyUserDict() 
{
  shared_ptr<Reader> emptyReader =
      make_shared<StringReader>(L"\n# This is an empty user dictionary\n\n");
  shared_ptr<UserDictionary> emptyDict = UserDictionary::open(emptyReader);

  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass9>(shared_from_this(), emptyDict);

  assertAnalyzesTo(
      analyzer, L"これは本ではない",
      std::deque<wstring>{L"これ", L"は", L"本", L"で", L"は", L"ない"},
      std::deque<int>{0, 2, 3, 4, 5, 6}, std::deque<int>{2, 3, 4, 5, 6, 8});
  delete analyzer;
}

TestJapaneseTokenizer::AnalyzerAnonymousInnerClass9::
    AnalyzerAnonymousInnerClass9(
        shared_ptr<TestJapaneseTokenizer> outerInstance,
        shared_ptr<UserDictionary> emptyDict)
{
  this->outerInstance = outerInstance;
  this->emptyDict = emptyDict;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseTokenizer::AnalyzerAnonymousInnerClass9::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), emptyDict, false,
      Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestJapaneseTokenizer::testBigDocument() 
{
  wstring doc =
      L"商品の購入・詳細(サイズ、画像)は商品名をクリックしてください！[L."
      L"B　CANDY　STOCK]フラワービジューベアドレス[L.B　DAILY　STOCK]"
      L"ボーダーニットトップス［L.B　DAILY　STOCK］ボーダーロングニットOP［L."
      L"B　DAILY　STOCK］ロゴトートBAG［L."
      L"B　DAILY　STOCK］裏毛ロゴプリントプルオーバー【TVドラマ着用】アンゴラワ"
      L"ッフルカーディガン【TVドラマ着用】グラフィティーバックリボンワンピース"
      L"【TVドラマ着用】ボーダーハイネックトップス【TVドラマ着用】レオパードミ"
      L"ッドカーフスカート【セットアップ対応商品】起毛ニットスカート【セットア"
      L"ップ対応商品】起毛ニットプルオーバー2wayサングラス33ナンバーリングニッ"
      L"ト3Dショルダーフレアードレス3周年スリッパ3周年ラグマット3周年ロックグラ"
      L"スキャンドルLily　Brown　2015年　福袋MIXニットプルオーバーPeckhamロゴニ"
      L"ットアンゴラジャガードプルオーバーアンゴラタートルアンゴラチュニックア"
      L"ンゴラニットカーディガンアンゴラニットプルオーバーアンゴラフレアワンピ"
      L"ースアンゴラロングカーディガンアンゴラワッフルカーディガンヴィンテージ"
      L"ファー付コートヴィンテージボーダーニットヴィンテージレースハイネックト"
      L"ップスヴィンテージレースブラウスウエストシースルーボーダーワンピースオ"
      L"ーガンジーラインフレアスカートオープンショルダーニットトップスオフショ"
      L"ルシャーリングワンピースオフショルニットオフショルニットプルオーバーオ"
      L"フショルボーダーロンパースオフショルワイドコンビネゾンオルテガ柄ニット"
      L"プルオーバーカシュクールオフショルワンピースカットアシンメトリードレス"
      L"カットサテンプリーツフレアースカートカラースーパーハイウェストスキニー"
      L"カラーブロックドレスカラーブロックニットチュニックギャザーフレアスカー"
      L"トキラキラストライプタイトスカートキラキラストライプドレスキルティング"
      L"ファーコートグラデーションベアドレスグラデーションラウンドサングラスグ"
      L"ラフティーオフショルトップスグラフティーキュロットグリッターリボンヘア"
      L"ゴムクロップドブラウスケーブルハイウエストスカートコーデュロイ×スエード"
      L"パネルスカートコーデュロイタイトスカートゴールドバックルベルト付スカー"
      L"トゴシックヒールショートブーツゴシック柄ニットワンピコンビスタジャンサ"
      L"イドステッチボーイズデニムパンツサスペつきショートパンツサスペンダー付"
      L"プリーツロングスカートシャーリングタイトスカートジャガードタックワンピ"
      L"ーススエードフリルフラワーパンツスエード裏毛肩空きトップススクエアショ"
      L"ルダーBAGスクエアバックルショルダースクエアミニバッグストーンビーチサン"
      L"ダルストライプサスペ付きスキニーストライプバックスリットシャツスライバ"
      L"ーシャギーコートタートル×レースタイトスカートタートルニットプルオーバー"
      L"タイトジャンパースカートダブルクロスチュールフレアスカートダブルストラ"
      L"ップパンプスダブルハートリングダブルフェイスチェックストールチェーンコ"
      L"ンビビジューネックレスチェーンコンビビジューピアスチェーンコンビビジュ"
      L"ーブレスチェーンツバ広HATチェーンビジューピアスチェックニットプルオーバ"
      L"ーチェックネルミディアムスカートチェック柄スキニーパンツチュールコンビ"
      L"アシメトップスデニムフレアースカートドットオフショルフリルブラウスドッ"
      L"トジャガードドレスドットニットプルオーバードットレーストップスニット×オ"
      L"ーガンジースカートセットニットキャミソールワンピースニットスヌードパー"
      L"ルコンビフープピアスハイウエストショートデニムハイウエストタイトスカー"
      L"トハイウエストデニムショートパンツハイウエストプリーツスカートハイウエ"
      L"ストミッドカーフスカートハイゲージタートルニットハイゲージラインニット"
      L"ハイネック切り替えスウェットバタフライネックレスバタフライミニピアスバ"
      L"タフライリングバックタンクリブワンピースバックリボンスキニーデニムパン"
      L"ツバックリボン深Vワンピースビジューストラップサンダルビスチェコンビオフ"
      L"ショルブラウスブークレジャガードニットフェイクムートンショートコートフ"
      L"ェレットカーディガンフェレットビックタートルニットブラウジングクルーブ"
      L"ラウスプリーツブラウスフリルニットプルオーバーフリンジニットプルオーバ"
      L"ーフレアニットスカートブロウ型サングラスベーシックフェレットプルオーバ"
      L"ーベルト付ガウチョパンツベルト付ショートパンツベルト付タックスカートベ"
      L"ルト付タックパンツベルベットインヒールパンプスベロアウェッジパンプスベ"
      L"ロアミッドカーフワンピースベロアワンピースベロア風ニットカーディガンボ"
      L"ア付コートボーダーVネックTシャツボーダーオフショルカットソーボーダーカ"
      L"ットソーワンピースボーダータイトカットソーボーダートップスボーダートッ"
      L"プス×スカートセットボストンメガネマオカラーシャツニットセットミックスニ"
      L"ットプルオーバーミッドカーフ丈ポンチスカートミリタリーギャザーショート"
      L"パンツメッシュハイネックトップスメルトンPコートメルトンダッフルコートメ"
      L"ルトンダブルコートモヘアニットカーディガンモヘアニットタートルユリ柄プ"
      L"リーツフレアースカートライダースデニムジャケットライナー付チェスターコ"
      L"ートラッフルプリーツブラウスラメジャガードハイゲージニットリブニットワ"
      L"ンピリボン×パールバレッタリボンバレッタリボンベルトハイウエストパンツリ"
      L"リー刺繍開襟ブラウスレースビスチェローファーサボロゴニットキャップロゴ"
      L"刺繍ニットワッチロングニットガウンワッフルアンゴラプルオーバーワンショ"
      L"ルダワーワンピース光沢ラメニットカーディガン刺繍シフォンブラウス台形ミ"
      L"ニスカート配色ニットプルオーバー裏毛プルオーバー×オーガンジースカートセ"
      L"ット";

  shared_ptr<JapaneseTokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      newAttributeFactory(), readDict(), false, Mode::NORMAL);
  tokenizer->setReader(make_shared<StringReader>(doc));
  tokenizer->reset();
  while (tokenizer->incrementToken()) {
    ;
  }
}
} // namespace org::apache::lucene::analysis::ja