using namespace std;

#include "TestHindiAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/hi/HindiAnalyzer.h"

namespace org::apache::lucene::analysis::hi
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestHindiAnalyzer::testResourcesAvailable()
{
  delete (make_shared<HindiAnalyzer>());
}

void TestHindiAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<HindiAnalyzer>();
  // two ways to write 'hindi' itself.
  checkOneTerm(a, L"हिन्दी", L"हिंद");
  checkOneTerm(a, L"हिंदी", L"हिंद");
  delete a;
}

void TestHindiAnalyzer::testExclusionSet() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"हिंदी"}), false);
  shared_ptr<Analyzer> a = make_shared<HindiAnalyzer>(
      HindiAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"हिंदी", L"हिंदी");
  delete a;
}

void TestHindiAnalyzer::testDigits() 
{
  shared_ptr<HindiAnalyzer> a = make_shared<HindiAnalyzer>();
  checkOneTerm(a, L"१२३४", L"1234");
  delete a;
}

void TestHindiAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<HindiAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::hi