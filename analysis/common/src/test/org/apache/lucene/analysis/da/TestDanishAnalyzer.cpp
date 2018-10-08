using namespace std;

#include "TestDanishAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/da/DanishAnalyzer.h"

namespace org::apache::lucene::analysis::da
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestDanishAnalyzer::testResourcesAvailable()
{
  delete (make_shared<DanishAnalyzer>());
}

void TestDanishAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<DanishAnalyzer>();
  // stemming
  checkOneTerm(a, L"undersøg", L"undersøg");
  checkOneTerm(a, L"undersøgelse", L"undersøg");
  // stopword
  assertAnalyzesTo(a, L"på", std::deque<wstring>());
  delete a;
}

void TestDanishAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"undersøgelse"}), false);
  shared_ptr<Analyzer> a = make_shared<DanishAnalyzer>(
      DanishAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"undersøgelse", L"undersøgelse");
  checkOneTerm(a, L"undersøg", L"undersøg");
  delete a;
}

void TestDanishAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<DanishAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::da