using namespace std;

#include "TestLithuanianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/lt/LithuanianAnalyzer.h"

namespace org::apache::lucene::analysis::lt
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestLithuanianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<LithuanianAnalyzer>());
}

void TestLithuanianAnalyzer::testStopWord() 
{
  shared_ptr<Analyzer> a = make_shared<LithuanianAnalyzer>();
  assertAnalyzesTo(a, L"man", std::deque<wstring>());
}

void TestLithuanianAnalyzer::testStemExclusion() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"vaikų");
  shared_ptr<Analyzer> a =
      make_shared<LithuanianAnalyzer>(CharArraySet::EMPTY_SET, set);
  assertAnalyzesTo(a, L"vaikų", std::deque<wstring>{L"vaikų"});
}

void TestLithuanianAnalyzer::testRandomStrings() 
{
  checkRandomData(random(), make_shared<LithuanianAnalyzer>(),
                  1000 * RANDOM_MULTIPLIER);
}
} // namespace org::apache::lucene::analysis::lt