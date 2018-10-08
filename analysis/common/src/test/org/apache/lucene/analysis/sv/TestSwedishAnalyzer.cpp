using namespace std;

#include "TestSwedishAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/sv/SwedishAnalyzer.h"

namespace org::apache::lucene::analysis::sv
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestSwedishAnalyzer::testResourcesAvailable()
{
  delete (make_shared<SwedishAnalyzer>());
}

void TestSwedishAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<SwedishAnalyzer>();
  // stemming
  checkOneTerm(a, L"jaktkarlarne", L"jaktkarl");
  checkOneTerm(a, L"jaktkarlens", L"jaktkarl");
  // stopword
  assertAnalyzesTo(a, L"och", std::deque<wstring>());
  delete a;
}

void TestSwedishAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"jaktkarlarne"}), false);
  shared_ptr<Analyzer> a = make_shared<SwedishAnalyzer>(
      SwedishAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"jaktkarlarne", L"jaktkarlarne");
  checkOneTerm(a, L"jaktkarlens", L"jaktkarl");
  delete a;
}

void TestSwedishAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<SwedishAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::sv