using namespace std;

#include "TestHungarianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/hu/HungarianAnalyzer.h"

namespace org::apache::lucene::analysis::hu
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestHungarianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<HungarianAnalyzer>());
}

void TestHungarianAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<HungarianAnalyzer>();
  // stemming
  checkOneTerm(a, L"babakocsi", L"babakocs");
  checkOneTerm(a, L"babakocsijáért", L"babakocs");
  // stopword
  assertAnalyzesTo(a, L"által", std::deque<wstring>());
  delete a;
}

void TestHungarianAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"babakocsi"}), false);
  shared_ptr<Analyzer> a = make_shared<HungarianAnalyzer>(
      HungarianAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"babakocsi", L"babakocsi");
  checkOneTerm(a, L"babakocsijáért", L"babakocs");
  delete a;
}

void TestHungarianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<HungarianAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::hu