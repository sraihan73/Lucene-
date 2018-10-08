using namespace std;

#include "TestGalicianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/gl/GalicianAnalyzer.h"

namespace org::apache::lucene::analysis::gl
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestGalicianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<GalicianAnalyzer>());
}

void TestGalicianAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<GalicianAnalyzer>();
  // stemming
  checkOneTerm(a, L"correspondente", L"correspond");
  checkOneTerm(a, L"corresponderá", L"correspond");
  // stopword
  assertAnalyzesTo(a, L"e", std::deque<wstring>());
  delete a;
}

void TestGalicianAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"correspondente"}), false);
  shared_ptr<Analyzer> a = make_shared<GalicianAnalyzer>(
      GalicianAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"correspondente", L"correspondente");
  checkOneTerm(a, L"corresponderá", L"correspond");
  delete a;
}

void TestGalicianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<GalicianAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::gl