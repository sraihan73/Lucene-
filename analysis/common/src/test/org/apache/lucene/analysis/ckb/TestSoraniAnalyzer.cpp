using namespace std;

#include "TestSoraniAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/ckb/SoraniAnalyzer.h"

namespace org::apache::lucene::analysis::ckb
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestSoraniAnalyzer::testResourcesAvailable()
{
  delete (make_shared<SoraniAnalyzer>());
}

void TestSoraniAnalyzer::testStopwords() 
{
  shared_ptr<Analyzer> a = make_shared<SoraniAnalyzer>();
  assertAnalyzesTo(a, L"ئەم پیاوە", std::deque<wstring>{L"پیاو"});
  delete a;
}

void TestSoraniAnalyzer::testCustomStopwords() 
{
  shared_ptr<Analyzer> a = make_shared<SoraniAnalyzer>(CharArraySet::EMPTY_SET);
  assertAnalyzesTo(a, L"ئەم پیاوە", std::deque<wstring>{L"ئەم", L"پیاو"});
  delete a;
}

void TestSoraniAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<SoraniAnalyzer>();
  assertAnalyzesTo(a, L"پیاوە", std::deque<wstring>{L"پیاو"});
  assertAnalyzesTo(a, L"پیاو", std::deque<wstring>{L"پیاو"});
  delete a;
}

void TestSoraniAnalyzer::testWithStemExclusionSet() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"پیاوە");
  shared_ptr<Analyzer> a =
      make_shared<SoraniAnalyzer>(CharArraySet::EMPTY_SET, set);
  assertAnalyzesTo(a, L"پیاوە", std::deque<wstring>{L"پیاوە"});
  delete a;
}

void TestSoraniAnalyzer::testDigits() 
{
  shared_ptr<SoraniAnalyzer> a = make_shared<SoraniAnalyzer>();
  checkOneTerm(a, L"١٢٣٤", L"1234");
  delete a;
}

void TestSoraniAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> a = make_shared<SoraniAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::ckb