using namespace std;

#include "TestTurkishAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/tr/TurkishAnalyzer.h"

namespace org::apache::lucene::analysis::tr
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestTurkishAnalyzer::testResourcesAvailable()
{
  delete (make_shared<TurkishAnalyzer>());
}

void TestTurkishAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<TurkishAnalyzer>();
  // stemming
  checkOneTerm(a, L"ağacı", L"ağaç");
  checkOneTerm(a, L"ağaç", L"ağaç");
  // stopword
  assertAnalyzesTo(a, L"dolayı", std::deque<wstring>());
  // apostrophes
  checkOneTerm(a, L"Kıbrıs'ta", L"kıbrıs");
  assertAnalyzesTo(a, L"Van Gölü'ne", std::deque<wstring>{L"van", L"göl"});
  delete a;
}

void TestTurkishAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"ağacı"}), false);
  shared_ptr<Analyzer> a = make_shared<TurkishAnalyzer>(
      TurkishAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"ağacı", L"ağacı");
  checkOneTerm(a, L"ağaç", L"ağaç");
  delete a;
}

void TestTurkishAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<TurkishAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::tr