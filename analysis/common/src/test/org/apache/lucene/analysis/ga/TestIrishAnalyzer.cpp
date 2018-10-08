using namespace std;

#include "TestIrishAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/ga/IrishAnalyzer.h"

namespace org::apache::lucene::analysis::ga
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestIrishAnalyzer::testResourcesAvailable()
{
  delete (make_shared<IrishAnalyzer>());
}

void TestIrishAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<IrishAnalyzer>();
  // stemming
  checkOneTerm(a, L"siopadóireacht", L"siopadóir");
  checkOneTerm(a, L"síceapatacha", L"síceapaite");
  // stopword
  assertAnalyzesTo(a, L"le", std::deque<wstring>());
  delete a;
}

void TestIrishAnalyzer::testContractions() 
{
  shared_ptr<Analyzer> a = make_shared<IrishAnalyzer>();
  assertAnalyzesTo(a, L"b'fhearr m'athair",
                   std::deque<wstring>{L"fearr", L"athair"});
  delete a;
}

void TestIrishAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"feirmeoireacht"}), false);
  shared_ptr<Analyzer> a = make_shared<IrishAnalyzer>(
      IrishAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"feirmeoireacht", L"feirmeoireacht");
  checkOneTerm(a, L"siopadóireacht", L"siopadóir");
  delete a;
}

void TestIrishAnalyzer::testHyphens() 
{
  shared_ptr<Analyzer> a = make_shared<IrishAnalyzer>();
  assertAnalyzesTo(a, L"n-athair", std::deque<wstring>{L"athair"},
                   std::deque<int>{2});
  delete a;
}

void TestIrishAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> a = make_shared<IrishAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::ga