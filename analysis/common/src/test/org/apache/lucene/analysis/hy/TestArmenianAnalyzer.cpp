using namespace std;

#include "TestArmenianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/hy/ArmenianAnalyzer.h"

namespace org::apache::lucene::analysis::hy
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestArmenianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<ArmenianAnalyzer>());
}

void TestArmenianAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<ArmenianAnalyzer>();
  // stemming
  checkOneTerm(a, L"արծիվ", L"արծ");
  checkOneTerm(a, L"արծիվներ", L"արծ");
  // stopword
  assertAnalyzesTo(a, L"է", std::deque<wstring>());
  delete a;
}

void TestArmenianAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"արծիվներ"}), false);
  shared_ptr<Analyzer> a = make_shared<ArmenianAnalyzer>(
      ArmenianAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"արծիվներ", L"արծիվներ");
  checkOneTerm(a, L"արծիվ", L"արծ");
  delete a;
}

void TestArmenianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<ArmenianAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::hy