using namespace std;

#include "TestLatvianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/lv/LatvianAnalyzer.h"

namespace org::apache::lucene::analysis::lv
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestLatvianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<LatvianAnalyzer>());
}

void TestLatvianAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<LatvianAnalyzer>();
  // stemming
  checkOneTerm(a, L"tirgiem", L"tirg");
  checkOneTerm(a, L"tirgus", L"tirg");
  // stopword
  assertAnalyzesTo(a, L"un", std::deque<wstring>());
  delete a;
}

void TestLatvianAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"tirgiem"}), false);
  shared_ptr<Analyzer> a = make_shared<LatvianAnalyzer>(
      LatvianAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"tirgiem", L"tirgiem");
  checkOneTerm(a, L"tirgus", L"tirg");
  delete a;
}

void TestLatvianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<LatvianAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::lv