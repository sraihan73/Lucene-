using namespace std;

#include "TestMockSynonymFilter.h"

namespace org::apache::lucene::analysis
{

void TestMockSynonymFilter::test() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  assertAnalyzesTo(analyzer, L"dogs", std::deque<wstring>{L"dogs", L"dog"},
                   std::deque<int>{0, 0}, std::deque<int>{4, 4}, nullptr,
                   std::deque<int>{1, 0}, std::deque<int>{1, 1},
                   true); // check that offsets are correct

  assertAnalyzesTo(
      analyzer, L"small dogs", std::deque<wstring>{L"small", L"dogs", L"dog"},
      std::deque<int>{0, 6, 6}, std::deque<int>{5, 10, 10}, nullptr,
      std::deque<int>{1, 1, 0}, std::deque<int>{1, 1, 1},
      true); // check that offsets are correct

  assertAnalyzesTo(analyzer, L"dogs running",
                   std::deque<wstring>{L"dogs", L"dog", L"running"},
                   std::deque<int>{0, 0, 5}, std::deque<int>{4, 4, 12},
                   nullptr, std::deque<int>{1, 0, 1},
                   std::deque<int>{1, 1, 1},
                   true); // check that offsets are correct

  assertAnalyzesTo(analyzer, L"small dogs running",
                   std::deque<wstring>{L"small", L"dogs", L"dog", L"running"},
                   std::deque<int>{0, 6, 6, 11},
                   std::deque<int>{5, 10, 10, 18}, nullptr,
                   std::deque<int>{1, 1, 0, 1}, std::deque<int>{1, 1, 1, 1},
                   true); // check that offsets are correct

  assertAnalyzesTo(analyzer, L"guinea", std::deque<wstring>{L"guinea"},
                   std::deque<int>{0}, std::deque<int>{6}, nullptr,
                   std::deque<int>{1}, std::deque<int>{1},
                   true); // check that offsets are correct

  assertAnalyzesTo(analyzer, L"pig", std::deque<wstring>{L"pig"},
                   std::deque<int>{0}, std::deque<int>{3}, nullptr,
                   std::deque<int>{1}, std::deque<int>{1},
                   true); // check that offsets are correct

  assertAnalyzesTo(
      analyzer, L"guinea pig", std::deque<wstring>{L"guinea", L"cavy", L"pig"},
      std::deque<int>{0, 0, 7}, std::deque<int>{6, 10, 10}, nullptr,
      std::deque<int>{1, 0, 1}, std::deque<int>{1, 2, 1},
      true); // check that offsets are correct

  assertAnalyzesTo(analyzer, L"guinea dogs",
                   std::deque<wstring>{L"guinea", L"dogs", L"dog"},
                   std::deque<int>{0, 7, 7}, std::deque<int>{6, 11, 11},
                   nullptr, std::deque<int>{1, 1, 0},
                   std::deque<int>{1, 1, 1},
                   true); // check that offsets are correct

  assertAnalyzesTo(analyzer, L"dogs guinea",
                   std::deque<wstring>{L"dogs", L"dog", L"guinea"},
                   std::deque<int>{0, 0, 5}, std::deque<int>{4, 4, 11},
                   nullptr, std::deque<int>{1, 0, 1},
                   std::deque<int>{1, 1, 1},
                   true); // check that offsets are correct

  assertAnalyzesTo(
      analyzer, L"dogs guinea pig",
      std::deque<wstring>{L"dogs", L"dog", L"guinea", L"cavy", L"pig"},
      std::deque<int>{0, 0, 5, 5, 12}, std::deque<int>{4, 4, 11, 15, 15},
      nullptr, std::deque<int>{1, 0, 1, 0, 1}, std::deque<int>{1, 1, 1, 2, 1},
      true); // check that offsets are correct

  assertAnalyzesTo(
      analyzer, L"guinea pig dogs",
      std::deque<wstring>{L"guinea", L"cavy", L"pig", L"dogs", L"dog"},
      std::deque<int>{0, 0, 7, 11, 11}, std::deque<int>{6, 10, 10, 15, 15},
      nullptr, std::deque<int>{1, 0, 1, 1, 0}, std::deque<int>{1, 2, 1, 1, 1},
      true); // check that offsets are correct

  assertAnalyzesTo(analyzer, L"small dogs and guinea pig running",
                   std::deque<wstring>{L"small", L"dogs", L"dog", L"and",
                                        L"guinea", L"cavy", L"pig", L"running"},
                   std::deque<int>{0, 6, 6, 11, 15, 15, 22, 26},
                   std::deque<int>{5, 10, 10, 14, 21, 25, 25, 33}, nullptr,
                   std::deque<int>{1, 1, 0, 1, 1, 0, 1, 1},
                   std::deque<int>{1, 1, 1, 1, 1, 2, 1, 1},
                   true); // check that offsets are correct

  assertAnalyzesTo(analyzer, L"small guinea pig and dogs running",
                   std::deque<wstring>{L"small", L"guinea", L"cavy", L"pig",
                                        L"and", L"dogs", L"dog", L"running"},
                   std::deque<int>{0, 6, 6, 13, 17, 21, 21, 26},
                   std::deque<int>{5, 12, 16, 16, 20, 25, 25, 33}, nullptr,
                   std::deque<int>{1, 1, 0, 1, 1, 1, 0, 1},
                   std::deque<int>{1, 1, 2, 1, 1, 1, 1, 1},
                   true); // check that offsets are correct
}

TestMockSynonymFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestMockSynonymFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestMockSynonymFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer = make_shared<MockTokenizer>();
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<MockSynonymFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis