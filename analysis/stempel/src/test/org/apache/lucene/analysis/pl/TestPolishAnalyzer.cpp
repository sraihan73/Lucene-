using namespace std;

#include "TestPolishAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/pl/PolishAnalyzer.h"

namespace org::apache::lucene::analysis::pl
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestPolishAnalyzer::testResourcesAvailable()
{
  delete (make_shared<PolishAnalyzer>());
}

void TestPolishAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<PolishAnalyzer>();
  // stemming
  checkOneTerm(a, L"studenta", L"student");
  checkOneTerm(a, L"studenci", L"student");
  // stopword
  assertAnalyzesTo(a, L"był", std::deque<wstring>());
  delete a;
}

void TestPolishAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"studenta"}), false);
  shared_ptr<Analyzer> a = make_shared<PolishAnalyzer>(
      PolishAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"studenta", L"studenta");
  checkOneTerm(a, L"studenci", L"student");
  delete a;
}

void TestPolishAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<PolishAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::pl