using namespace std;

#include "TestEnglishAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/en/EnglishAnalyzer.h"

namespace org::apache::lucene::analysis::en
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestEnglishAnalyzer::testResourcesAvailable()
{
  delete (make_shared<EnglishAnalyzer>());
}

void TestEnglishAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<EnglishAnalyzer>();
  // stemming
  checkOneTerm(a, L"books", L"book");
  checkOneTerm(a, L"book", L"book");
  // stopword
  assertAnalyzesTo(a, L"the", std::deque<wstring>());
  // possessive removal
  checkOneTerm(a, L"steven's", L"steven");
  checkOneTerm(a, L"steven\u2019s", L"steven");
  checkOneTerm(a, L"steven\uFF07s", L"steven");
  delete a;
}

void TestEnglishAnalyzer::testExclude() 
{
  shared_ptr<CharArraySet> exclusionSet =
      make_shared<CharArraySet>(asSet({L"books"}), false);
  shared_ptr<Analyzer> a = make_shared<EnglishAnalyzer>(
      EnglishAnalyzer::getDefaultStopSet(), exclusionSet);
  checkOneTerm(a, L"books", L"books");
  checkOneTerm(a, L"book", L"book");
  delete a;
}

void TestEnglishAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> a = make_shared<EnglishAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::en