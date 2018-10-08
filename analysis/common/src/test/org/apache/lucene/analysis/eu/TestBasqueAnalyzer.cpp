using namespace std;

#include "TestBasqueAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/eu/BasqueAnalyzer.h"

namespace org::apache::lucene::analysis::eu
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestBasqueAnalyzer::testResourcesAvailable()
{
  delete (make_shared<BasqueAnalyzer>());
}

void TestBasqueAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<BasqueAnalyzer>();
  // stemming
  checkOneTerm(a, L"zaldi", L"zaldi");
  checkOneTerm(a, L"zaldiak", L"zaldi");
  // stopword
  assertAnalyzesTo(a, L"izan", std::deque<wstring>());
  delete a;
}

void TestBasqueAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"zaldiak"}), false);
  shared_ptr<Analyzer> a = make_shared<BasqueAnalyzer>(
      BasqueAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"zaldiak", L"zaldiak");
  checkOneTerm(a, L"mendiari", L"mendi");
  delete a;
}

void TestBasqueAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> a = make_shared<BasqueAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::eu