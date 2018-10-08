using namespace std;

#include "TestSmartChineseAnalyzer.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../java/org/apache/lucene/analysis/cn/smart/SmartChineseAnalyzer.h"

namespace org::apache::lucene::analysis::cn::smart
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestSmartChineseAnalyzer::testChineseStopWordsDefault() throw(
    runtime_error)
{
  shared_ptr<Analyzer> ca =
      make_shared<SmartChineseAnalyzer>(); // will load stopwords
  wstring sentence = L"我购买了道具和服装。";
  std::deque<wstring> result = {L"我",   L"购买", L"了",
                                 L"道具", L"和",   L"服装"};
  assertAnalyzesTo(ca, sentence, result);
  delete ca;
  // set stop-words from the outer world - must yield same behavior
  ca = make_shared<SmartChineseAnalyzer>(
      SmartChineseAnalyzer::getDefaultStopSet());
  assertAnalyzesTo(ca, sentence, result);
  delete ca;
}

void TestSmartChineseAnalyzer::testChineseStopWordsDefaultTwoPhrases() throw(
    runtime_error)
{
  shared_ptr<Analyzer> ca =
      make_shared<SmartChineseAnalyzer>(); // will load stopwords
  wstring sentence = L"我购买了道具和服装。 我购买了道具和服装。";
  std::deque<wstring> result = {L"我", L"购买", L"了", L"道具",
                                 L"和", L"服装", L"我", L"购买",
                                 L"了", L"道具", L"和", L"服装"};
  assertAnalyzesTo(ca, sentence, result);
  delete ca;
}

void TestSmartChineseAnalyzer::testSurrogatePairCharacter() 
{
  shared_ptr<Analyzer> ca =
      make_shared<SmartChineseAnalyzer>(); // will load stopwords
  wstring sentence = Stream::of(L"\uD872\uDF3B", L"\uD872\uDF4A",
                                L"\uD872\uDF73", L"\uD872\uDF5B", L"\u9FCF",
                                L"\uD86D\uDFFC", L"\uD872\uDF2D", L"\u9FD4")
                         .collect(Collectors::joining());
  std::deque<wstring> result = {
      L"\uD872\uDF3B", L"\uD872\uDF4A", L"\uD872\uDF73", L"\uD872\uDF5B",
      L"\u9FCF",       L"\uD86D\uDFFC", L"\uD872\uDF2D", L"\u9FD4"};
  assertAnalyzesTo(ca, sentence, result);
  delete ca;
}

void TestSmartChineseAnalyzer::
    testChineseStopWordsDefaultTwoPhrasesIdeoSpace() 
{
  shared_ptr<Analyzer> ca =
      make_shared<SmartChineseAnalyzer>(); // will load stopwords
  wstring sentence = L"我购买了道具和服装　我购买了道具和服装。";
  std::deque<wstring> result = {L"我", L"购买", L"了", L"道具",
                                 L"和", L"服装", L"我", L"购买",
                                 L"了", L"道具", L"和", L"服装"};
  assertAnalyzesTo(ca, sentence, result);
  delete ca;
}

void TestSmartChineseAnalyzer::testChineseStopWordsOff() 
{
  std::deque<std::shared_ptr<Analyzer>> analyzers = {
      make_shared<SmartChineseAnalyzer>(false),
      make_shared<SmartChineseAnalyzer>(nullptr)};
  wstring sentence = L"我购买了道具和服装。";
  std::deque<wstring> result = {L"我", L"购买", L"了", L"道具",
                                 L"和", L"服装", L","};
  for (auto analyzer : analyzers) {
    assertAnalyzesTo(analyzer, sentence, result);
    assertAnalyzesTo(analyzer, sentence, result);
  }
  IOUtils::close(analyzers);
}

void TestSmartChineseAnalyzer::testChineseStopWords2() 
{
  shared_ptr<Analyzer> ca =
      make_shared<SmartChineseAnalyzer>(); // will load stopwords
  wstring sentence = L"Title:San";         // : is a stopword
  std::deque<wstring> result = {L"titl", L"san"};
  std::deque<int> startOffsets = {0, 6};
  std::deque<int> endOffsets = {5, 9};
  std::deque<int> posIncr = {1, 2};
  assertAnalyzesTo(ca, sentence, result, startOffsets, endOffsets, posIncr);
  delete ca;
}

void TestSmartChineseAnalyzer::testChineseAnalyzer() 
{
  shared_ptr<Analyzer> ca = make_shared<SmartChineseAnalyzer>(true);
  wstring sentence = L"我购买了道具和服装。";
  std::deque<wstring> result = {L"我",   L"购买", L"了",
                                 L"道具", L"和",   L"服装"};
  assertAnalyzesTo(ca, sentence, result);
  delete ca;
}

void TestSmartChineseAnalyzer::testMixedLatinChinese() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SmartChineseAnalyzer>(true);
  assertAnalyzesTo(analyzer, L"我购买 Tests 了道具和服装",
                   std::deque<wstring>{L"我", L"购买", L"test", L"了", L"道具",
                                        L"和", L"服装"});
  delete analyzer;
}

void TestSmartChineseAnalyzer::testNumerics() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SmartChineseAnalyzer>(true);
  assertAnalyzesTo(analyzer, L"我购买 Tests 了道具和服装1234",
                   std::deque<wstring>{L"我", L"购买", L"test", L"了", L"道具",
                                        L"和", L"服装", L"1234"});
  delete analyzer;
}

void TestSmartChineseAnalyzer::testFullWidth() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SmartChineseAnalyzer>(true);
  assertAnalyzesTo(analyzer, L"我购买 Ｔｅｓｔｓ 了道具和服装１２３４",
                   std::deque<wstring>{L"我", L"购买", L"test", L"了", L"道具",
                                        L"和", L"服装", L"1234"});
  delete analyzer;
}

void TestSmartChineseAnalyzer::testDelimiters() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SmartChineseAnalyzer>(true);
  assertAnalyzesTo(analyzer, L"我购买︱ Tests 了道具和服装",
                   std::deque<wstring>{L"我", L"购买", L"test", L"了", L"道具",
                                        L"和", L"服装"});
  delete analyzer;
}

void TestSmartChineseAnalyzer::testNonChinese() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SmartChineseAnalyzer>(true);
  assertAnalyzesTo(analyzer, L"我购买 روبرتTests 了道具和服装",
                   std::deque<wstring>{L"我", L"购买", L"ر", L"و", L"ب", L"ر",
                                        L"ت", L"test", L"了", L"道具", L"和",
                                        L"服装"});
  delete analyzer;
}

void TestSmartChineseAnalyzer::testOOV() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SmartChineseAnalyzer>(true);
  assertAnalyzesTo(analyzer, L"优素福·拉扎·吉拉尼",
                   std::deque<wstring>{L"优", L"素", L"福", L"拉", L"扎",
                                        L"吉", L"拉", L"尼"});

  assertAnalyzesTo(analyzer, L"优素福拉扎吉拉尼",
                   std::deque<wstring>{L"优", L"素", L"福", L"拉", L"扎",
                                        L"吉", L"拉", L"尼"});
  delete analyzer;
}

void TestSmartChineseAnalyzer::testOffsets() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SmartChineseAnalyzer>(true);
  assertAnalyzesTo(
      analyzer, L"我购买了道具和服装",
      std::deque<wstring>{L"我", L"购买", L"了", L"道具", L"和", L"服装"},
      std::deque<int>{0, 1, 3, 4, 6, 7}, std::deque<int>{1, 3, 4, 6, 7, 9});
  delete analyzer;
}

void TestSmartChineseAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<SmartChineseAnalyzer>();
  assertAnalyzesTo(a, L"我购买 Tests 了道具和服装",
                   std::deque<wstring>{L"我", L"购买", L"test", L"了", L"道具",
                                        L"和", L"服装"},
                   std::deque<int>{0, 1, 4, 10, 11, 13, 14},
                   std::deque<int>{1, 3, 9, 11, 13, 14, 16});
  assertAnalyzesTo(
      a, L"我购买了道具和服装。",
      std::deque<wstring>{L"我", L"购买", L"了", L"道具", L"和", L"服装"},
      std::deque<int>{0, 1, 3, 4, 6, 7}, std::deque<int>{1, 3, 4, 6, 7, 9});
  delete a;
}

void TestSmartChineseAnalyzer::testLargeDocument() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < 5000; i++) {
    sb->append(L"我购买了道具和服装。");
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer analyzer
  // = new SmartChineseAnalyzer(); org.apache.lucene.analysis.TokenStream stream
  // = analyzer.tokenStream("", sb.toString()))
  {
    org::apache::lucene::analysis::Analyzer analyzer = SmartChineseAnalyzer();
    org::apache::lucene::analysis::TokenStream stream =
        analyzer->tokenStream(L"", sb->toString());
    stream->reset();
    while (stream->incrementToken()) {
    }
    stream->end();
  }
}

void TestSmartChineseAnalyzer::testLargeSentence() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < 5000; i++) {
    sb->append(L"我购买了道具和服装");
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer analyzer
  // = new SmartChineseAnalyzer(); org.apache.lucene.analysis.TokenStream stream
  // = analyzer.tokenStream("", sb.toString()))
  {
    org::apache::lucene::analysis::Analyzer analyzer = SmartChineseAnalyzer();
    org::apache::lucene::analysis::TokenStream stream =
        analyzer->tokenStream(L"", sb->toString());
    stream->reset();
    while (stream->incrementToken()) {
    }
    stream->end();
  }
}

void TestSmartChineseAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SmartChineseAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}

void TestSmartChineseAnalyzer::testRandomHugeStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SmartChineseAnalyzer>();
  checkRandomData(random(), analyzer, 100 * RANDOM_MULTIPLIER, 8192);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::cn::smart