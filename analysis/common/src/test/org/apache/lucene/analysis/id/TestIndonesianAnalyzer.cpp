using namespace std;

#include "TestIndonesianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/id/IndonesianAnalyzer.h"

namespace org::apache::lucene::analysis::id
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestIndonesianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<IndonesianAnalyzer>());
}

void TestIndonesianAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<IndonesianAnalyzer>();
  // stemming
  checkOneTerm(a, L"peledakan", L"ledak");
  checkOneTerm(a, L"pembunuhan", L"bunuh");
  // stopword
  assertAnalyzesTo(a, L"bahwa", std::deque<wstring>());
  delete a;
}

void TestIndonesianAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"peledakan"}), false);
  shared_ptr<Analyzer> a = make_shared<IndonesianAnalyzer>(
      IndonesianAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"peledakan", L"peledakan");
  checkOneTerm(a, L"pembunuhan", L"bunuh");
  delete a;
}

void TestIndonesianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<IndonesianAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::id