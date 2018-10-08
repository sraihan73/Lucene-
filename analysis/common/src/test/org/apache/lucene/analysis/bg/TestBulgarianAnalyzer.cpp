using namespace std;

#include "TestBulgarianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/bg/BulgarianAnalyzer.h"

namespace org::apache::lucene::analysis::bg
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestBulgarianAnalyzer::testResourcesAvailable()
{
  delete (make_shared<BulgarianAnalyzer>());
}

void TestBulgarianAnalyzer::testStopwords() 
{
  shared_ptr<Analyzer> a = make_shared<BulgarianAnalyzer>();
  assertAnalyzesTo(a, L"Как се казваш?", std::deque<wstring>{L"казваш"});
  delete a;
}

void TestBulgarianAnalyzer::testCustomStopwords() 
{
  shared_ptr<Analyzer> a =
      make_shared<BulgarianAnalyzer>(CharArraySet::EMPTY_SET);
  assertAnalyzesTo(a, L"Как се казваш?",
                   std::deque<wstring>{L"как", L"се", L"казваш"});
  delete a;
}

void TestBulgarianAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<BulgarianAnalyzer>();
  assertAnalyzesTo(a, L"документи", std::deque<wstring>{L"документ"});
  assertAnalyzesTo(a, L"документ", std::deque<wstring>{L"документ"});
  delete a;
}

void TestBulgarianAnalyzer::testBasicExamples() 
{
  shared_ptr<Analyzer> a = make_shared<BulgarianAnalyzer>();
  assertAnalyzesTo(a, L"енергийни кризи",
                   std::deque<wstring>{L"енергийн", L"криз"});
  assertAnalyzesTo(a, L"Атомната енергия",
                   std::deque<wstring>{L"атомн", L"енерг"});

  assertAnalyzesTo(a, L"компютри", std::deque<wstring>{L"компютр"});
  assertAnalyzesTo(a, L"компютър", std::deque<wstring>{L"компютр"});

  assertAnalyzesTo(a, L"градове", std::deque<wstring>{L"град"});
  delete a;
}

void TestBulgarianAnalyzer::testWithStemExclusionSet() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"строеве");
  shared_ptr<Analyzer> a =
      make_shared<BulgarianAnalyzer>(CharArraySet::EMPTY_SET, set);
  assertAnalyzesTo(a, L"строевете строеве",
                   std::deque<wstring>{L"строй", L"строеве"});
  delete a;
}

void TestBulgarianAnalyzer::testRandomStrings() 
{
  shared_ptr<BulgarianAnalyzer> a = make_shared<BulgarianAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::bg