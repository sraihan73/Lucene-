using namespace std;

#include "TestJapaneseNumberFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseNumberFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizer.h"

namespace org::apache::lucene::analysis::ja
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using org::junit::Ignore;
using org::junit::Test;

void TestJapaneseNumberFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestJapaneseNumberFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestJapaneseNumberFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseNumberFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, false,
      JapaneseTokenizer::Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseNumberFilter>(tokenizer));
}

void TestJapaneseNumberFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasics() throws java.io.IOException
void TestJapaneseNumberFilter::testBasics() 
{

  assertAnalyzesTo(analyzer, L"本日十万二千五百円のワインを買った",
                   std::deque<wstring>{L"本日", L"102500", L"円", L"の",
                                        L"ワイン", L"を", L"買っ", L"た"},
                   std::deque<int>{0, 2, 8, 9, 10, 13, 14, 16},
                   std::deque<int>{2, 8, 9, 10, 13, 14, 16, 17});

  assertAnalyzesTo(analyzer, L"昨日のお寿司は１０万円でした。",
                   std::deque<wstring>{L"昨日", L"の", L"お", L"寿司", L"は",
                                        L"100000", L"円", L"でし", L"た",
                                        L"。"},
                   std::deque<int>{0, 2, 3, 4, 6, 7, 10, 11, 13, 14},
                   std::deque<int>{2, 3, 4, 6, 7, 10, 11, 13, 14, 15});

  assertAnalyzesTo(analyzer, L"アティリカの資本金は６００万円です",
                   std::deque<wstring>{L"アティリカ", L"の", L"資本", L"金",
                                        L"は", L"6000000", L"円", L"です"},
                   std::deque<int>{0, 5, 6, 8, 9, 10, 14, 15},
                   std::deque<int>{5, 6, 8, 9, 10, 14, 15, 17});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testVariants() throws java.io.IOException
void TestJapaneseNumberFilter::testVariants() 
{
  // Test variants of three
  assertAnalyzesTo(analyzer, L"3", std::deque<wstring>{L"3"});
  assertAnalyzesTo(analyzer, L"３", std::deque<wstring>{L"3"});
  assertAnalyzesTo(analyzer, L"三", std::deque<wstring>{L"3"});

  // Test three variations with trailing zero
  assertAnalyzesTo(analyzer, L"03", std::deque<wstring>{L"3"});
  assertAnalyzesTo(analyzer, L"０３", std::deque<wstring>{L"3"});
  assertAnalyzesTo(analyzer, L"〇三", std::deque<wstring>{L"3"});
  assertAnalyzesTo(analyzer, L"003", std::deque<wstring>{L"3"});
  assertAnalyzesTo(analyzer, L"００３", std::deque<wstring>{L"3"});
  assertAnalyzesTo(analyzer, L"〇〇三", std::deque<wstring>{L"3"});

  // Test thousand variants
  assertAnalyzesTo(analyzer, L"千", std::deque<wstring>{L"1000"});
  assertAnalyzesTo(analyzer, L"1千", std::deque<wstring>{L"1000"});
  assertAnalyzesTo(analyzer, L"１千", std::deque<wstring>{L"1000"});
  assertAnalyzesTo(analyzer, L"一千", std::deque<wstring>{L"1000"});
  assertAnalyzesTo(analyzer, L"一〇〇〇", std::deque<wstring>{L"1000"});
  assertAnalyzesTo(analyzer, L"１０百",
                   std::deque<wstring>{L"1000"}); // Strange, but supported
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLargeVariants() throws
// java.io.IOException
void TestJapaneseNumberFilter::testLargeVariants() 
{
  // Test large numbers
  assertAnalyzesTo(analyzer, L"三五七八九", std::deque<wstring>{L"35789"});
  assertAnalyzesTo(analyzer, L"六百二万五千一",
                   std::deque<wstring>{L"6025001"});
  assertAnalyzesTo(analyzer, L"兆六百万五千一",
                   std::deque<wstring>{L"1000006005001"});
  assertAnalyzesTo(analyzer, L"十兆六百万五千一",
                   std::deque<wstring>{L"10000006005001"});
  assertAnalyzesTo(analyzer, L"一京一",
                   std::deque<wstring>{L"10000000000000001"});
  assertAnalyzesTo(analyzer, L"十京十",
                   std::deque<wstring>{L"100000000000000010"});
  assertAnalyzesTo(analyzer, L"垓京兆億万千百十一",
                   std::deque<wstring>{L"100010001000100011111"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNegative() throws java.io.IOException
void TestJapaneseNumberFilter::testNegative() 
{
  assertAnalyzesTo(analyzer, L"-100万", std::deque<wstring>{L"-", L"1000000"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMixed() throws java.io.IOException
void TestJapaneseNumberFilter::testMixed() 
{
  // Test mixed numbers
  assertAnalyzesTo(analyzer, L"三千2百２十三", std::deque<wstring>{L"3223"});
  assertAnalyzesTo(analyzer, L"３２二三", std::deque<wstring>{L"3223"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNininsankyaku() throws
// java.io.IOException
void TestJapaneseNumberFilter::testNininsankyaku() 
{
  // Unstacked tokens
  assertAnalyzesTo(analyzer, L"二", std::deque<wstring>{L"2"});
  assertAnalyzesTo(analyzer, L"二人", std::deque<wstring>{L"2", L"人"});
  assertAnalyzesTo(analyzer, L"二人三",
                   std::deque<wstring>{L"2", L"人", L"3"});
  // Stacked tokens - emit tokens as they are
  assertAnalyzesTo(
      analyzer, L"二人三脚",
      std::deque<wstring>{L"二", L"二人三脚", L"人", L"三", L"脚"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFujiyaichinisanu() throws
// java.io.IOException
void TestJapaneseNumberFilter::testFujiyaichinisanu() 
{
  // Stacked tokens with a numeral partial
  assertAnalyzesTo(
      analyzer, L"不二家一二三",
      std::deque<wstring>{L"不", L"不二家", L"二", L"家", L"123"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFunny() throws java.io.IOException
void TestJapaneseNumberFilter::testFunny() 
{
  // Test some oddities for inconsistent input
  assertAnalyzesTo(analyzer, L"十十", std::deque<wstring>{L"20"}); // 100?
  assertAnalyzesTo(analyzer, L"百百百",
                   std::deque<wstring>{L"300"}); // 10,000?
  assertAnalyzesTo(analyzer, L"千千千千",
                   std::deque<wstring>{L"4000"}); // 1,000,000,000,000?
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testKanjiArabic() throws java.io.IOException
void TestJapaneseNumberFilter::testKanjiArabic() 
{
  // Test kanji numerals used as Arabic numbers (with head zero)
  assertAnalyzesTo(analyzer, L"〇一二三四五六七八九九八七六五四三二一〇",
                   std::deque<wstring>{L"1234567899876543210"});

  // I'm Bond, James "normalized" Bond...
  assertAnalyzesTo(analyzer, L"〇〇七", std::deque<wstring>{L"7"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDoubleZero() throws java.io.IOException
void TestJapaneseNumberFilter::testDoubleZero() 
{
  assertAnalyzesTo(analyzer, L"〇〇", std::deque<wstring>{L"0"},
                   std::deque<int>{0}, std::deque<int>{2},
                   std::deque<int>{1});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testName() throws java.io.IOException
void TestJapaneseNumberFilter::testName() 
{
  // Test name that normalises to number
  assertAnalyzesTo(analyzer, L"田中京一",
                   std::deque<wstring>{L"田中", L"10000000000000001"},
                   std::deque<int>{0, 2}, std::deque<int>{2, 4},
                   std::deque<int>{1, 1});

  // An analyzer that marks 京一 as a keyword
  shared_ptr<Analyzer> keywordMarkingAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

  assertAnalyzesTo(keywordMarkingAnalyzer, L"田中京一",
                   std::deque<wstring>{L"田中", L"京一"},
                   std::deque<int>{0, 2}, std::deque<int>{2, 4},
                   std::deque<int>{1, 1});
  delete keywordMarkingAnalyzer;
}

TestJapaneseNumberFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestJapaneseNumberFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseNumberFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, false);
  set->add(L"京一");

  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, false,
      JapaneseTokenizer::Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<JapaneseNumberFilter>(
                     make_shared<SetKeywordMarkerFilter>(tokenizer, set)));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDecimal() throws java.io.IOException
void TestJapaneseNumberFilter::testDecimal() 
{
  // Test Arabic numbers with punctuation, i.e. 3.2 thousands
  assertAnalyzesTo(analyzer, L"１．２万３４５．６７",
                   std::deque<wstring>{L"12345.67"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDecimalPunctuation() throws
// java.io.IOException
void TestJapaneseNumberFilter::testDecimalPunctuation() 
{
  // Test Arabic numbers with punctuation, i.e. 3.2 thousands yen
  assertAnalyzesTo(analyzer, L"３．２千円",
                   std::deque<wstring>{L"3200", L"円"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testThousandSeparator() throws
// java.io.IOException
void TestJapaneseNumberFilter::testThousandSeparator() 
{
  assertAnalyzesTo(analyzer, L"4,647", std::deque<wstring>{L"4647"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDecimalThousandSeparator() throws
// java.io.IOException
void TestJapaneseNumberFilter::testDecimalThousandSeparator() 
{
  assertAnalyzesTo(analyzer, L"4,647.0010", std::deque<wstring>{L"4647.001"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCommaDecimalSeparator() throws
// java.io.IOException
void TestJapaneseNumberFilter::testCommaDecimalSeparator() 
{
  assertAnalyzesTo(analyzer, L"15,7", std::deque<wstring>{L"157"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTrailingZeroStripping() throws
// java.io.IOException
void TestJapaneseNumberFilter::testTrailingZeroStripping() 
{
  assertAnalyzesTo(analyzer, L"1000.1000", std::deque<wstring>{L"1000.1"});
  assertAnalyzesTo(analyzer, L"1000.0000", std::deque<wstring>{L"1000"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmpty() throws java.io.IOException
void TestJapaneseNumberFilter::testEmpty() 
{
  assertAnalyzesTo(analyzer, L"", std::deque<wstring>());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRandomHugeStrings() throws Exception
void TestJapaneseNumberFilter::testRandomHugeStrings() 
{
  checkRandomData(random(), analyzer, 50 * RANDOM_MULTIPLIER, 8192);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRandomSmallStrings() throws Exception
void TestJapaneseNumberFilter::testRandomSmallStrings() 
{
  checkRandomData(random(), analyzer, 500 * RANDOM_MULTIPLIER, 128);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFunnyIssue() throws Exception
void TestJapaneseNumberFilter::testFunnyIssue() 
{
  BaseTokenStreamTestCase::checkAnalysisConsistency(
      random(), analyzer, true, L"〇〇\u302f\u3029\u3039\u3023\u3033\u302bB",
      true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore("This test is used during development when analyze
// normalizations in large amounts of text") @Test public void testLargeData()
// throws java.io.IOException
void TestJapaneseNumberFilter::testLargeData() 
{
  shared_ptr<Path> input = Paths->get(L"/tmp/test.txt");
  shared_ptr<Path> tokenizedOutput = Paths->get(L"/tmp/test.tok.txt");
  shared_ptr<Path> normalizedOutput = Paths->get(L"/tmp/test.norm.txt");

  shared_ptr<Analyzer> plainAnalyzer =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());

  analyze(plainAnalyzer,
          Files::newBufferedReader(input, StandardCharsets::UTF_8),
          Files::newBufferedWriter(tokenizedOutput, StandardCharsets::UTF_8));

  analyze(analyzer, Files::newBufferedReader(input, StandardCharsets::UTF_8),
          Files::newBufferedWriter(normalizedOutput, StandardCharsets::UTF_8));
  delete plainAnalyzer;
}

TestJapaneseNumberFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestJapaneseNumberFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestJapaneseNumberFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<JapaneseTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr, false,
      JapaneseTokenizer::Mode::SEARCH);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

void TestJapaneseNumberFilter::analyze(
    shared_ptr<Analyzer> analyzer, shared_ptr<Reader> reader,
    shared_ptr<Writer> writer) 
{
  shared_ptr<TokenStream> stream = analyzer->tokenStream(L"dummy", reader);
  stream->reset();

  shared_ptr<CharTermAttribute> termAttr =
      stream->addAttribute(CharTermAttribute::typeid);

  while (stream->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->write(termAttr->toString());
    writer->write(L"\n");
  }

  reader->close();
  writer->close();
}
} // namespace org::apache::lucene::analysis::ja