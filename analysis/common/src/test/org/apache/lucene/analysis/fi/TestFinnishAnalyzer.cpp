using namespace std;

#include "TestFinnishAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/fi/FinnishAnalyzer.h"

namespace org::apache::lucene::analysis::fi
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestFinnishAnalyzer::testResourcesAvailable()
{
  delete (make_shared<FinnishAnalyzer>());
}

void TestFinnishAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<FinnishAnalyzer>();
  // stemming
  checkOneTerm(a, L"edeltäjiinsä", L"edeltäj");
  checkOneTerm(a, L"edeltäjistään", L"edeltäj");
  // stopword
  assertAnalyzesTo(a, L"olla", std::deque<wstring>());
  delete a;
}

void TestFinnishAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"edeltäjistään"}), false);
  shared_ptr<Analyzer> a = make_shared<FinnishAnalyzer>(
      FinnishAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"edeltäjiinsä", L"edeltäj");
  checkOneTerm(a, L"edeltäjistään", L"edeltäjistään");
  delete a;
}

void TestFinnishAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> a = make_shared<FinnishAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::fi