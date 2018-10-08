using namespace std;

#include "TestItalianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/it/ItalianAnalyzer.h"

namespace org::apache::lucene::analysis::it
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestItalianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<ItalianAnalyzer>());
}

void TestItalianAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<ItalianAnalyzer>();
  // stemming
  checkOneTerm(a, L"abbandonata", L"abbandonat");
  checkOneTerm(a, L"abbandonati", L"abbandonat");
  // stopword
  assertAnalyzesTo(a, L"dallo", std::deque<wstring>());
  delete a;
}

void TestItalianAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"abbandonata"}), false);
  shared_ptr<Analyzer> a = make_shared<ItalianAnalyzer>(
      ItalianAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"abbandonata", L"abbandonata");
  checkOneTerm(a, L"abbandonati", L"abbandonat");
  delete a;
}

void TestItalianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<ItalianAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}

void TestItalianAnalyzer::testContractions() 
{
  shared_ptr<Analyzer> a = make_shared<ItalianAnalyzer>();
  assertAnalyzesTo(a, L"dell'Italia", std::deque<wstring>{L"ital"});
  assertAnalyzesTo(a, L"l'Italiano", std::deque<wstring>{L"italian"});
  delete a;
}
} // namespace org::apache::lucene::analysis::it