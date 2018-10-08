using namespace std;

#include "TestCzechAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/cz/CzechAnalyzer.h"

namespace org::apache::lucene::analysis::cz
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestCzechAnalyzer::testResourcesAvailable()
{
  delete (make_shared<CzechAnalyzer>());
}

void TestCzechAnalyzer::testStopWord() 
{
  shared_ptr<Analyzer> analyzer = make_shared<CzechAnalyzer>();
  assertAnalyzesTo(analyzer, L"Pokud mluvime o volnem",
                   std::deque<wstring>{L"mluvim", L"voln"});
  delete analyzer;
}

void TestCzechAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> analyzer = make_shared<CzechAnalyzer>();
  assertAnalyzesTo(analyzer, L"Pokud mluvime o volnem",
                   std::deque<wstring>{L"mluvim", L"voln"});
  assertAnalyzesTo(analyzer, L"Česká Republika",
                   std::deque<wstring>{L"česk", L"republik"});
  delete analyzer;
}

void TestCzechAnalyzer::testWithStemExclusionSet() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"hole");
  shared_ptr<CzechAnalyzer> cz =
      make_shared<CzechAnalyzer>(CharArraySet::EMPTY_SET, set);
  assertAnalyzesTo(cz, L"hole desek", std::deque<wstring>{L"hole", L"desk"});
  delete cz;
}

void TestCzechAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<CzechAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::cz