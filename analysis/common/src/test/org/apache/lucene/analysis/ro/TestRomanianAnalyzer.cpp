using namespace std;

#include "TestRomanianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/ro/RomanianAnalyzer.h"

namespace org::apache::lucene::analysis::ro
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestRomanianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<RomanianAnalyzer>());
}

void TestRomanianAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<RomanianAnalyzer>();
  // stemming
  checkOneTerm(a, L"absenţa", L"absenţ");
  checkOneTerm(a, L"absenţi", L"absenţ");
  // stopword
  assertAnalyzesTo(a, L"îl", std::deque<wstring>());
  delete a;
}

void TestRomanianAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"absenţa"}), false);
  shared_ptr<Analyzer> a = make_shared<RomanianAnalyzer>(
      RomanianAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"absenţa", L"absenţa");
  checkOneTerm(a, L"absenţi", L"absenţ");
  delete a;
}

void TestRomanianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<RomanianAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::ro