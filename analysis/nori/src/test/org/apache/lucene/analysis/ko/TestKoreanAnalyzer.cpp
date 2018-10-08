using namespace std;

#include "TestKoreanAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanPartOfSpeechStopFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/POS.h"
#include "TestKoreanTokenizer.h"

namespace org::apache::lucene::analysis::ko
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

void TestKoreanAnalyzer::testSentence() 
{
  shared_ptr<Analyzer> a = make_shared<KoreanAnalyzer>();
  assertAnalyzesTo(a, L"한국은 대단한 나라입니다.",
                   std::deque<wstring>{L"한국", L"대단", L"나라", L"이"},
                   std::deque<int>{0, 4, 8, 10},
                   std::deque<int>{2, 6, 10, 13},
                   std::deque<int>{1, 2, 3, 1});
  delete a;
}

void TestKoreanAnalyzer::testStopTags() 
{
  shared_ptr<Set<POS::Tag>> stopTags =
      Arrays::asList(POS::Tag::NNP, POS::Tag::NNG)
          .stream()
          .collect(Collectors::toSet());
  shared_ptr<Analyzer> a = make_shared<KoreanAnalyzer>(
      nullptr, KoreanTokenizer::DecompoundMode::DISCARD, stopTags, false);
  assertAnalyzesTo(
      a, L"한국은 대단한 나라입니다.",
      std::deque<wstring>{L"은", L"대단", L"하", L"ᆫ", L"이", L"ᄇ니다"},
      std::deque<int>{2, 4, 6, 6, 10, 10},
      std::deque<int>{3, 6, 7, 7, 13, 13}, std::deque<int>{2, 1, 1, 1, 2, 1});
  delete a;
}

void TestKoreanAnalyzer::testUnknownWord() 
{
  shared_ptr<Analyzer> a = make_shared<KoreanAnalyzer>(
      nullptr, KoreanTokenizer::DecompoundMode::DISCARD,
      KoreanPartOfSpeechStopFilter::DEFAULT_STOP_TAGS, true);

  assertAnalyzesTo(a, L"2018 평창 동계올림픽대회",
                   std::deque<wstring>{L"2", L"0", L"1", L"8", L"평창",
                                        L"동계", L"올림픽", L"대회"},
                   std::deque<int>{0, 1, 2, 3, 5, 8, 10, 13},
                   std::deque<int>{1, 2, 3, 4, 7, 10, 13, 15},
                   std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1});
  delete a;

  a = make_shared<KoreanAnalyzer>(
      nullptr, KoreanTokenizer::DecompoundMode::DISCARD,
      KoreanPartOfSpeechStopFilter::DEFAULT_STOP_TAGS, false);

  assertAnalyzesTo(
      a, L"2018 평창 동계올림픽대회",
      std::deque<wstring>{L"2018", L"평창", L"동계", L"올림픽", L"대회"},
      std::deque<int>{0, 5, 8, 10, 13}, std::deque<int>{4, 7, 10, 13, 15},
      std::deque<int>{1, 1, 1, 1, 1});
  delete a;
}

void TestKoreanAnalyzer::testRandom() 
{
  shared_ptr<Random> random = TestKoreanAnalyzer::random();
  shared_ptr<Analyzer> *const a = make_shared<KoreanAnalyzer>();
  checkRandomData(random, a, atLeast(1000));
  delete a;
}

void TestKoreanAnalyzer::testRandomHugeStrings() 
{
  shared_ptr<Random> random = TestKoreanAnalyzer::random();
  shared_ptr<Analyzer> *const a = make_shared<KoreanAnalyzer>();
  checkRandomData(random, a, 2 * RANDOM_MULTIPLIER, 8192);
  delete a;
}

void TestKoreanAnalyzer::testUserDict() 
{
  shared_ptr<Analyzer> *const analyzer = make_shared<KoreanAnalyzer>(
      TestKoreanTokenizer::readDict(), KoreanTokenizer::DEFAULT_DECOMPOUND,
      KoreanPartOfSpeechStopFilter::DEFAULT_STOP_TAGS, false);
  assertAnalyzesTo(analyzer, L"c++ 프로그래밍 언어",
                   std::deque<wstring>{L"c++", L"프로그래밍", L"언어"},
                   std::deque<int>{0, 4, 10}, std::deque<int>{3, 9, 12},
                   std::deque<int>{1, 1, 1});
}
} // namespace org::apache::lucene::analysis::ko