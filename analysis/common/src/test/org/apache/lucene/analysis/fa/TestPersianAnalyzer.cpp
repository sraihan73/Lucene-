using namespace std;

#include "TestPersianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/fa/PersianAnalyzer.h"

namespace org::apache::lucene::analysis::fa
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestPersianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<PersianAnalyzer>());
}

void TestPersianAnalyzer::testBehaviorVerbs() 
{
  shared_ptr<Analyzer> a = make_shared<PersianAnalyzer>();
  // active present indicative
  assertAnalyzesTo(a, L"می‌خورد", std::deque<wstring>{L"خورد"});
  // active preterite indicative
  assertAnalyzesTo(a, L"خورد", std::deque<wstring>{L"خورد"});
  // active imperfective preterite indicative
  assertAnalyzesTo(a, L"می‌خورد", std::deque<wstring>{L"خورد"});
  // active future indicative
  assertAnalyzesTo(a, L"خواهد خورد", std::deque<wstring>{L"خورد"});
  // active present progressive indicative
  assertAnalyzesTo(a, L"دارد می‌خورد",
                   std::deque<wstring>{L"خورد"});
  // active preterite progressive indicative
  assertAnalyzesTo(a, L"داشت می‌خورد",
                   std::deque<wstring>{L"خورد"});

  // active perfect indicative
  assertAnalyzesTo(a, L"خورده‌است", std::deque<wstring>{L"خورده"});
  // active imperfective perfect indicative
  assertAnalyzesTo(a, L"می‌خورده‌است",
                   std::deque<wstring>{L"خورده"});
  // active pluperfect indicative
  assertAnalyzesTo(a, L"خورده بود", std::deque<wstring>{L"خورده"});
  // active imperfective pluperfect indicative
  assertAnalyzesTo(a, L"می‌خورده بود",
                   std::deque<wstring>{L"خورده"});
  // active preterite subjunctive
  assertAnalyzesTo(a, L"خورده باشد", std::deque<wstring>{L"خورده"});
  // active imperfective preterite subjunctive
  assertAnalyzesTo(a, L"می‌خورده باشد",
                   std::deque<wstring>{L"خورده"});
  // active pluperfect subjunctive
  assertAnalyzesTo(a, L"خورده بوده باشد", std::deque<wstring>{L"خورده"});
  // active imperfective pluperfect subjunctive
  assertAnalyzesTo(a, L"می‌خورده بوده باشد",
                   std::deque<wstring>{L"خورده"});
  // passive present indicative
  assertAnalyzesTo(a, L"خورده می‌شود",
                   std::deque<wstring>{L"خورده"});
  // passive preterite indicative
  assertAnalyzesTo(a, L"خورده شد", std::deque<wstring>{L"خورده"});
  // passive imperfective preterite indicative
  assertAnalyzesTo(a, L"خورده می‌شد",
                   std::deque<wstring>{L"خورده"});
  // passive perfect indicative
  assertAnalyzesTo(a, L"خورده شده‌است",
                   std::deque<wstring>{L"خورده"});
  // passive imperfective perfect indicative
  assertAnalyzesTo(a, L"خورده می‌شده‌است",
                   std::deque<wstring>{L"خورده"});
  // passive pluperfect indicative
  assertAnalyzesTo(a, L"خورده شده بود", std::deque<wstring>{L"خورده"});
  // passive imperfective pluperfect indicative
  assertAnalyzesTo(a, L"خورده می‌شده بود",
                   std::deque<wstring>{L"خورده"});
  // passive future indicative
  assertAnalyzesTo(a, L"خورده خواهد شد", std::deque<wstring>{L"خورده"});
  // passive present progressive indicative
  assertAnalyzesTo(a, L"دارد خورده می‌شود",
                   std::deque<wstring>{L"خورده"});
  // passive preterite progressive indicative
  assertAnalyzesTo(a, L"داشت خورده می‌شد",
                   std::deque<wstring>{L"خورده"});
  // passive present subjunctive
  assertAnalyzesTo(a, L"خورده شود", std::deque<wstring>{L"خورده"});
  // passive preterite subjunctive
  assertAnalyzesTo(a, L"خورده شده باشد", std::deque<wstring>{L"خورده"});
  // passive imperfective preterite subjunctive
  assertAnalyzesTo(a, L"خورده می‌شده باشد",
                   std::deque<wstring>{L"خورده"});
  // passive pluperfect subjunctive
  assertAnalyzesTo(a, L"خورده شده بوده باشد", std::deque<wstring>{L"خورده"});
  // passive imperfective pluperfect subjunctive
  assertAnalyzesTo(a, L"خورده می‌شده بوده باشد",
                   std::deque<wstring>{L"خورده"});

  // active present subjunctive
  assertAnalyzesTo(a, L"بخورد", std::deque<wstring>{L"بخورد"});
  delete a;
}

void TestPersianAnalyzer::testBehaviorVerbsDefective() 
{
  shared_ptr<Analyzer> a = make_shared<PersianAnalyzer>();
  // active present indicative
  assertAnalyzesTo(a, L"مي خورد", std::deque<wstring>{L"خورد"});
  // active preterite indicative
  assertAnalyzesTo(a, L"خورد", std::deque<wstring>{L"خورد"});
  // active imperfective preterite indicative
  assertAnalyzesTo(a, L"مي خورد", std::deque<wstring>{L"خورد"});
  // active future indicative
  assertAnalyzesTo(a, L"خواهد خورد", std::deque<wstring>{L"خورد"});
  // active present progressive indicative
  assertAnalyzesTo(a, L"دارد مي خورد", std::deque<wstring>{L"خورد"});
  // active preterite progressive indicative
  assertAnalyzesTo(a, L"داشت مي خورد", std::deque<wstring>{L"خورد"});

  // active perfect indicative
  assertAnalyzesTo(a, L"خورده است", std::deque<wstring>{L"خورده"});
  // active imperfective perfect indicative
  assertAnalyzesTo(a, L"مي خورده است", std::deque<wstring>{L"خورده"});
  // active pluperfect indicative
  assertAnalyzesTo(a, L"خورده بود", std::deque<wstring>{L"خورده"});
  // active imperfective pluperfect indicative
  assertAnalyzesTo(a, L"مي خورده بود", std::deque<wstring>{L"خورده"});
  // active preterite subjunctive
  assertAnalyzesTo(a, L"خورده باشد", std::deque<wstring>{L"خورده"});
  // active imperfective preterite subjunctive
  assertAnalyzesTo(a, L"مي خورده باشد", std::deque<wstring>{L"خورده"});
  // active pluperfect subjunctive
  assertAnalyzesTo(a, L"خورده بوده باشد", std::deque<wstring>{L"خورده"});
  // active imperfective pluperfect subjunctive
  assertAnalyzesTo(a, L"مي خورده بوده باشد", std::deque<wstring>{L"خورده"});
  // passive present indicative
  assertAnalyzesTo(a, L"خورده مي شود", std::deque<wstring>{L"خورده"});
  // passive preterite indicative
  assertAnalyzesTo(a, L"خورده شد", std::deque<wstring>{L"خورده"});
  // passive imperfective preterite indicative
  assertAnalyzesTo(a, L"خورده مي شد", std::deque<wstring>{L"خورده"});
  // passive perfect indicative
  assertAnalyzesTo(a, L"خورده شده است", std::deque<wstring>{L"خورده"});
  // passive imperfective perfect indicative
  assertAnalyzesTo(a, L"خورده مي شده است", std::deque<wstring>{L"خورده"});
  // passive pluperfect indicative
  assertAnalyzesTo(a, L"خورده شده بود", std::deque<wstring>{L"خورده"});
  // passive imperfective pluperfect indicative
  assertAnalyzesTo(a, L"خورده مي شده بود", std::deque<wstring>{L"خورده"});
  // passive future indicative
  assertAnalyzesTo(a, L"خورده خواهد شد", std::deque<wstring>{L"خورده"});
  // passive present progressive indicative
  assertAnalyzesTo(a, L"دارد خورده مي شود", std::deque<wstring>{L"خورده"});
  // passive preterite progressive indicative
  assertAnalyzesTo(a, L"داشت خورده مي شد", std::deque<wstring>{L"خورده"});
  // passive present subjunctive
  assertAnalyzesTo(a, L"خورده شود", std::deque<wstring>{L"خورده"});
  // passive preterite subjunctive
  assertAnalyzesTo(a, L"خورده شده باشد", std::deque<wstring>{L"خورده"});
  // passive imperfective preterite subjunctive
  assertAnalyzesTo(a, L"خورده مي شده باشد", std::deque<wstring>{L"خورده"});
  // passive pluperfect subjunctive
  assertAnalyzesTo(a, L"خورده شده بوده باشد", std::deque<wstring>{L"خورده"});
  // passive imperfective pluperfect subjunctive
  assertAnalyzesTo(a, L"خورده مي شده بوده باشد",
                   std::deque<wstring>{L"خورده"});

  // active present subjunctive
  assertAnalyzesTo(a, L"بخورد", std::deque<wstring>{L"بخورد"});
  delete a;
}

void TestPersianAnalyzer::testBehaviorNouns() 
{
  shared_ptr<Analyzer> a = make_shared<PersianAnalyzer>();
  assertAnalyzesTo(a, L"برگ ها", std::deque<wstring>{L"برگ"});
  assertAnalyzesTo(a, L"برگ‌ها", std::deque<wstring>{L"برگ"});
  delete a;
}

void TestPersianAnalyzer::testBehaviorNonPersian() 
{
  shared_ptr<Analyzer> a = make_shared<PersianAnalyzer>();
  assertAnalyzesTo(a, L"English test.",
                   std::deque<wstring>{L"english", L"test"});
  delete a;
}

void TestPersianAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<PersianAnalyzer>();
  assertAnalyzesTo(a, L"خورده مي شده بوده باشد",
                   std::deque<wstring>{L"خورده"});
  assertAnalyzesTo(a, L"برگ‌ها", std::deque<wstring>{L"برگ"});
  delete a;
}

void TestPersianAnalyzer::testCustomStopwords() 
{
  shared_ptr<PersianAnalyzer> a = make_shared<PersianAnalyzer>(
      make_shared<CharArraySet>(asSet({L"the", L"and", L"a"}), false));
  assertAnalyzesTo(a, L"The quick brown fox.",
                   std::deque<wstring>{L"quick", L"brown", L"fox"});
  delete a;
}

void TestPersianAnalyzer::testDigits() 
{
  shared_ptr<PersianAnalyzer> a = make_shared<PersianAnalyzer>();
  checkOneTerm(a, L"۱۲۳۴", L"1234");
  delete a;
}

void TestPersianAnalyzer::testRandomStrings() 
{
  shared_ptr<PersianAnalyzer> a = make_shared<PersianAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::fa