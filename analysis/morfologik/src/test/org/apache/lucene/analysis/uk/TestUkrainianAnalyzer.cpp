using namespace std;

#include "TestUkrainianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/uk/UkrainianMorfologikAnalyzer.h"

namespace org::apache::lucene::analysis::uk
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

void TestUkrainianAnalyzer::testDigitsInUkrainianCharset() 
{
  shared_ptr<UkrainianMorfologikAnalyzer> ra =
      make_shared<UkrainianMorfologikAnalyzer>();
  assertAnalyzesTo(ra, L"text 1000", std::deque<wstring>{L"text", L"1000"});
  delete ra;
}

void TestUkrainianAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<UkrainianMorfologikAnalyzer>();
  assertAnalyzesTo(a,
                   L"Ця п'єса, у свою чергу, рухається по емоційно-напруженому "
                   L"колу за ритм-енд-блюзом.",
                   std::deque<wstring>{L"п'єса", L"черга", L"рухатися",
                                        L"емоційно", L"напружений", L"кола",
                                        L"коло", L"кіл", L"ритм", L"енд",
                                        L"блюз"});
  delete a;
}

void TestUkrainianAnalyzer::testSpecialCharsTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<UkrainianMorfologikAnalyzer>();
  assertAnalyzesTo(a,
                   L"м'яса м'я\u0301са м\u02BCяса м\u2019яса м\u2018яса м`яса",
                   std::deque<wstring>{L"м'ясо", L"м'ясо", L"м'ясо", L"м'ясо",
                                        L"м'ясо", L"м'ясо"});
  delete a;
}

void TestUkrainianAnalyzer::testCapsTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<UkrainianMorfologikAnalyzer>();
  assertAnalyzesTo(
      a, L"Цих Чайковського і Ґете.",
      std::deque<wstring>{L"Чайковське", L"Чайковський", L"Гете"});
  delete a;
}

void TestUkrainianAnalyzer::testCharNormalization() 
{
  shared_ptr<Analyzer> a = make_shared<UkrainianMorfologikAnalyzer>();
  assertAnalyzesTo(a, L"Ґюмрі та Гюмрі.",
                   std::deque<wstring>{L"Гюмрі", L"Гюмрі"});
  delete a;
}

void TestUkrainianAnalyzer::testSampleSentence() 
{
  shared_ptr<Analyzer> a = make_shared<UkrainianMorfologikAnalyzer>();
  assertAnalyzesTo(a,
                   L"Це — проект генерування словника з тегами частин мови для "
                   L"української мови.",
                   std::deque<wstring>{L"проект", L"генерування", L"словник",
                                        L"тег", L"частина", L"мова",
                                        L"українська", L"український",
                                        L"Українська", L"мова"});
  delete a;
}

void TestUkrainianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<UkrainianMorfologikAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::uk