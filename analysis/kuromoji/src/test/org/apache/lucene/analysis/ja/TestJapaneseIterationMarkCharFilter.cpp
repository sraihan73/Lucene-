using namespace std;

#include "TestJapaneseIterationMarkCharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseIterationMarkCharFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizer.h"

namespace org::apache::lucene::analysis::ja
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestJapaneseIterationMarkCharFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  keywordAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  japaneseAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
}

TestJapaneseIterationMarkCharFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestJapaneseIterationMarkCharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseIterationMarkCharFilter::AnalyzerAnonymousInnerClass::
    createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

shared_ptr<Reader>
TestJapaneseIterationMarkCharFilter::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<JapaneseIterationMarkCharFilter>(reader);
}

TestJapaneseIterationMarkCharFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestJapaneseIterationMarkCharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseIterationMarkCharFilter::AnalyzerAnonymousInnerClass2::
    createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, false,
      JapaneseTokenizer::Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

shared_ptr<Reader>
TestJapaneseIterationMarkCharFilter::AnalyzerAnonymousInnerClass2::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<JapaneseIterationMarkCharFilter>(reader);
}

void TestJapaneseIterationMarkCharFilter::tearDown() 
{
  IOUtils::close({keywordAnalyzer, japaneseAnalyzer});
  BaseTokenStreamTestCase::tearDown();
}

void TestJapaneseIterationMarkCharFilter::testKanji() 
{
  // Test single repetition
  assertAnalyzesTo(keywordAnalyzer, L"時々", std::deque<wstring>{L"時時"});
  assertAnalyzesTo(japaneseAnalyzer, L"時々", std::deque<wstring>{L"時時"});

  // Test multiple repetitions
  assertAnalyzesTo(keywordAnalyzer, L"馬鹿々々しい",
                   std::deque<wstring>{L"馬鹿馬鹿しい"});
  assertAnalyzesTo(japaneseAnalyzer, L"馬鹿々々しい",
                   std::deque<wstring>{L"馬鹿馬鹿しい"});
}

void TestJapaneseIterationMarkCharFilter::testKatakana() 
{
  // Test single repetition
  assertAnalyzesTo(keywordAnalyzer, L"ミスヾ", std::deque<wstring>{L"ミスズ"});
  assertAnalyzesTo(japaneseAnalyzer, L"ミスヾ",
                   std::deque<wstring>{L"ミ", L"スズ"}); // Side effect
}

void TestJapaneseIterationMarkCharFilter::testHiragana() 
{
  // Test single unvoiced iteration
  assertAnalyzesTo(keywordAnalyzer, L"おゝの", std::deque<wstring>{L"おおの"});
  assertAnalyzesTo(japaneseAnalyzer, L"おゝの",
                   std::deque<wstring>{L"お", L"おの"}); // Side effect

  // Test single voiced iteration
  assertAnalyzesTo(keywordAnalyzer, L"みすゞ", std::deque<wstring>{L"みすず"});
  assertAnalyzesTo(japaneseAnalyzer, L"みすゞ",
                   std::deque<wstring>{L"みすず"});

  // Test single voiced iteration
  assertAnalyzesTo(keywordAnalyzer, L"じゞ", std::deque<wstring>{L"じじ"});
  assertAnalyzesTo(japaneseAnalyzer, L"じゞ", std::deque<wstring>{L"じじ"});

  // Test single unvoiced iteration with voiced iteration
  assertAnalyzesTo(keywordAnalyzer, L"じゝ", std::deque<wstring>{L"じし"});
  assertAnalyzesTo(japaneseAnalyzer, L"じゝ", std::deque<wstring>{L"じし"});

  // Test multiple repetitions with voiced iteration
  assertAnalyzesTo(keywordAnalyzer, L"ところゞゝゝ",
                   std::deque<wstring>{L"ところどころ"});
  assertAnalyzesTo(japaneseAnalyzer, L"ところゞゝゝ",
                   std::deque<wstring>{L"ところどころ"});
}

void TestJapaneseIterationMarkCharFilter::testMalformed() 
{
  // We can't iterate c here, so emit as it is
  assertAnalyzesTo(keywordAnalyzer, L"abcところゝゝゝゝ",
                   std::deque<wstring>{L"abcところcところ"});

  // We can't iterate c (with dakuten change) here, so emit it as-is
  assertAnalyzesTo(keywordAnalyzer, L"abcところゞゝゝゝ",
                   std::deque<wstring>{L"abcところcところ"});

  // We can't iterate before beginning of stream, so emit characters as-is
  assertAnalyzesTo(keywordAnalyzer, L"ところゞゝゝゞゝゞ",
                   std::deque<wstring>{L"ところどころゞゝゞ"});

  // We can't iterate an iteration mark only, so emit as-is
  assertAnalyzesTo(keywordAnalyzer, L"々", std::deque<wstring>{L"々"});
  assertAnalyzesTo(keywordAnalyzer, L"ゞ", std::deque<wstring>{L"ゞ"});
  assertAnalyzesTo(keywordAnalyzer, L"ゞゝ", std::deque<wstring>{L"ゞゝ"});

  // We can't iterate a full stop punctuation mark (because we use it as a flush
  // marker)
  assertAnalyzesTo(keywordAnalyzer, L"。ゝ", std::deque<wstring>{L"。ゝ"});
  assertAnalyzesTo(keywordAnalyzer, L"。。ゝゝ",
                   std::deque<wstring>{L"。。ゝゝ"});

  // We can iterate other punctuation marks
  assertAnalyzesTo(keywordAnalyzer, L"？ゝ", std::deque<wstring>{L"？？"});

  // We can not get a dakuten variant of ぽ -- this is also a corner case test
  // for inside()
  assertAnalyzesTo(keywordAnalyzer, L"ねやぽゞつむぴ",
                   std::deque<wstring>{L"ねやぽぽつむぴ"});
  assertAnalyzesTo(keywordAnalyzer, L"ねやぽゝつむぴ",
                   std::deque<wstring>{L"ねやぽぽつむぴ"});
}

void TestJapaneseIterationMarkCharFilter::testEmpty() 
{
  // Empty input stays empty
  assertAnalyzesTo(keywordAnalyzer, L"", std::deque<wstring>(0));
  assertAnalyzesTo(japaneseAnalyzer, L"", std::deque<wstring>(0));
}

void TestJapaneseIterationMarkCharFilter::testFullStop() 
{
  // Test full stops
  assertAnalyzesTo(keywordAnalyzer, L"。", std::deque<wstring>{L"。"});
  assertAnalyzesTo(keywordAnalyzer, L"。。", std::deque<wstring>{L"。。"});
  assertAnalyzesTo(keywordAnalyzer, L"。。。", std::deque<wstring>{L"。。。"});
}

void TestJapaneseIterationMarkCharFilter::testKanjiOnly() 
{
  // Test kanji only repetition marks
  shared_ptr<CharFilter> filter = make_shared<JapaneseIterationMarkCharFilter>(
      make_shared<StringReader>(
          L"時々、おゝのさんと一緒にお寿司が食べたいです。abcところゞゝゝ。"),
      true, false);
  assertCharFilterEquals(
      filter,
      L"時時、おゝのさんと一緒にお寿司が食べたいです。abcところゞゝゝ。");
}

void TestJapaneseIterationMarkCharFilter::testKanaOnly() 
{
  // Test kana only repetition marks
  shared_ptr<CharFilter> filter = make_shared<JapaneseIterationMarkCharFilter>(
      make_shared<StringReader>(
          L"時々、おゝのさんと一緒にお寿司が食べたいです。abcところゞゝゝ。"),
      false, true);
  assertCharFilterEquals(
      filter,
      L"時々、おおのさんと一緒にお寿司が食べたいです。abcところどころ。");
}

void TestJapaneseIterationMarkCharFilter::testNone() 
{
  // Test no repetition marks
  shared_ptr<CharFilter> filter = make_shared<JapaneseIterationMarkCharFilter>(
      make_shared<StringReader>(
          L"時々、おゝのさんと一緒にお寿司が食べたいです。abcところゞゝゝ。"),
      false, false);
  assertCharFilterEquals(
      filter,
      L"時々、おゝのさんと一緒にお寿司が食べたいです。abcところゞゝゝ。");
}

void TestJapaneseIterationMarkCharFilter::testCombinations() 
{
  assertAnalyzesTo(keywordAnalyzer,
                   L"時々、おゝのさんと一緒にお寿司を食べに行きます。",
                   std::deque<wstring>{
                       L"時時、おおのさんと一緒にお寿司を食べに行きます。"});
}

void TestJapaneseIterationMarkCharFilter::testHiraganaCoverage() throw(
    IOException)
{
  // Test all hiragana iteration variants
  wstring source = L"かゝがゝきゝぎゝくゝぐゝけゝげゝこゝごゝさゝざゝしゝじゝす"
                   L"ゝずゝせゝぜゝそゝぞゝたゝだゝちゝぢゝつゝづゝてゝでゝとゝ"
                   L"どゝはゝばゝひゝびゝふゝぶゝへゝべゝほゝぼゝ";
  wstring target = L"かかがかききぎきくくぐくけけげけここごこささざさししじしす"
                   L"すずすせせぜせそそぞそたただたちちぢちつつづつててでてとと"
                   L"どとははばはひひびひふふぶふへへべへほほぼほ";
  assertAnalyzesTo(keywordAnalyzer, source, std::deque<wstring>{target});

  // Test all hiragana iteration variants with dakuten
  source = L"かゞがゞきゞぎゞくゞぐゞけゞげゞこゞごゞさゞざゞしゞじゞすゞずゞせ"
           L"ゞぜゞそゞぞゞたゞだゞちゞぢゞつゞづゞてゞでゞとゞどゞはゞばゞひゞ"
           L"びゞふゞぶゞへゞべゞほゞぼゞ";
  target = L"かがががきぎぎぎくぐぐぐけげげげこごごごさざざざしじじじすずずずせ"
           L"ぜぜぜそぞぞぞただだだちぢぢぢつづづづてでででとどどどはばばばひび"
           L"びびふぶぶぶへべべべほぼぼぼ";
  assertAnalyzesTo(keywordAnalyzer, source, std::deque<wstring>{target});
}

void TestJapaneseIterationMarkCharFilter::testKatakanaCoverage() throw(
    IOException)
{
  // Test all katakana iteration variants
  wstring source = L"カヽガヽキヽギヽクヽグヽケヽゲヽコヽゴヽサヽザヽシヽジヽス"
                   L"ヽズヽセヽゼヽソヽゾヽタヽダヽチヽヂヽツヽヅヽテヽデヽトヽ"
                   L"ドヽハヽバヽヒヽビヽフヽブヽヘヽベヽホヽボヽ";
  wstring target = L"カカガカキキギキククグクケケゲケココゴコササザサシシジシス"
                   L"スズスセセゼセソソゾソタタダタチチヂチツツヅツテテデテトト"
                   L"ドトハハバハヒヒビヒフフブフヘヘベヘホホボホ";
  assertAnalyzesTo(keywordAnalyzer, source, std::deque<wstring>{target});

  // Test all katakana iteration variants with dakuten
  source = L"カヾガヾキヾギヾクヾグヾケヾゲヾコヾゴヾサヾザヾシヾジヾスヾズヾセ"
           L"ヾゼヾソヾゾヾタヾダヾチヾヂヾツヾヅヾテヾデヾトヾドヾハヾバヾヒヾ"
           L"ビヾフヾブヾヘヾベヾホヾボヾ";
  target = L"カガガガキギギギクグググケゲゲゲコゴゴゴサザザザシジジジスズズズセ"
           L"ゼゼゼソゾゾゾタダダダチヂヂヂツヅヅヅテデデデトドドドハバババヒビ"
           L"ビビフブブブヘベベベホボボボ";
  assertAnalyzesTo(keywordAnalyzer, source, std::deque<wstring>{target});
}

void TestJapaneseIterationMarkCharFilter::testRandomStrings() throw(
    runtime_error)
{
  // Blast some random strings through
  checkRandomData(random(), keywordAnalyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestJapaneseIterationMarkCharFilter::testRandomHugeStrings() throw(
    runtime_error)
{
  // Blast some random strings through
  checkRandomData(random(), keywordAnalyzer, 100 * RANDOM_MULTIPLIER, 8192);
}

void TestJapaneseIterationMarkCharFilter::assertCharFilterEquals(
    shared_ptr<CharFilter> filter, const wstring &expected) 
{
  wstring actual = readFully(filter);
  assertEquals(expected, actual);
}

wstring TestJapaneseIterationMarkCharFilter::readFully(
    shared_ptr<Reader> stream) 
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  int ch;
  while ((ch = stream->read()) != -1) {
    buffer->append(static_cast<wchar_t>(ch));
  }
  return buffer->toString();
}
} // namespace org::apache::lucene::analysis::ja