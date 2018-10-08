using namespace std;

#include "TestPortugueseAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/pt/PortugueseAnalyzer.h"

namespace org::apache::lucene::analysis::pt
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestPortugueseAnalyzer::testResourcesAvailable()
{
  delete (make_shared<PortugueseAnalyzer>());
}

void TestPortugueseAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<PortugueseAnalyzer>();
  // stemming
  checkOneTerm(a, L"quilométricas", L"quilometric");
  checkOneTerm(a, L"quilométricos", L"quilometric");
  // stopword
  assertAnalyzesTo(a, L"não", std::deque<wstring>());
  delete a;
}

void TestPortugueseAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"quilométricas"}), false);
  shared_ptr<Analyzer> a = make_shared<PortugueseAnalyzer>(
      PortugueseAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"quilométricas", L"quilométricas");
  checkOneTerm(a, L"quilométricos", L"quilometric");
  delete a;
}

void TestPortugueseAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<PortugueseAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::pt