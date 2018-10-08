using namespace std;

#include "TestSpanishAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/es/SpanishAnalyzer.h"

namespace org::apache::lucene::analysis::es
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestSpanishAnalyzer::testResourcesAvailable()
{
  delete (make_shared<SpanishAnalyzer>());
}

void TestSpanishAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<SpanishAnalyzer>();
  // stemming
  checkOneTerm(a, L"chicana", L"chican");
  checkOneTerm(a, L"chicano", L"chican");
  // stopword
  assertAnalyzesTo(a, L"los", std::deque<wstring>());
  delete a;
}

void TestSpanishAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"chicano"}), false);
  shared_ptr<Analyzer> a = make_shared<SpanishAnalyzer>(
      SpanishAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"chicana", L"chican");
  checkOneTerm(a, L"chicano", L"chicano");
  delete a;
}

void TestSpanishAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> a = make_shared<SpanishAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::es