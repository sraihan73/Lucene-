using namespace std;

#include "TestCatalanAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/ca/CatalanAnalyzer.h"

namespace org::apache::lucene::analysis::ca
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestCatalanAnalyzer::testResourcesAvailable()
{
  delete (make_shared<CatalanAnalyzer>());
}

void TestCatalanAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<CatalanAnalyzer>();
  // stemming
  checkOneTerm(a, L"llengües", L"llengu");
  checkOneTerm(a, L"llengua", L"llengu");
  // stopword
  assertAnalyzesTo(a, L"un", std::deque<wstring>());
  delete a;
}

void TestCatalanAnalyzer::testContractions() 
{
  shared_ptr<Analyzer> a = make_shared<CatalanAnalyzer>();
  assertAnalyzesTo(
      a, L"Diccionari de l'Institut d'Estudis Catalans",
      std::deque<wstring>{L"diccion", L"inst", L"estud", L"catalan"});
  delete a;
}

void TestCatalanAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"llengües"}), false);
  shared_ptr<Analyzer> a = make_shared<CatalanAnalyzer>(
      CatalanAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"llengües", L"llengües");
  checkOneTerm(a, L"llengua", L"llengu");
  delete a;
}

void TestCatalanAnalyzer::testRandomStrings() 
{
  shared_ptr<CatalanAnalyzer> a = make_shared<CatalanAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::ca