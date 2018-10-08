using namespace std;

#include "TestNorwegianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/no/NorwegianAnalyzer.h"

namespace org::apache::lucene::analysis::no
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestNorwegianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<NorwegianAnalyzer>());
}

void TestNorwegianAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<NorwegianAnalyzer>();
  // stemming
  checkOneTerm(a, L"havnedistriktene", L"havnedistrikt");
  checkOneTerm(a, L"havnedistrikter", L"havnedistrikt");
  // stopword
  assertAnalyzesTo(a, L"det", std::deque<wstring>());
  delete a;
}

void TestNorwegianAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"havnedistriktene"}), false);
  shared_ptr<Analyzer> a = make_shared<NorwegianAnalyzer>(
      NorwegianAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"havnedistriktene", L"havnedistriktene");
  checkOneTerm(a, L"havnedistrikter", L"havnedistrikt");
  delete a;
}

void TestNorwegianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<NorwegianAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::no