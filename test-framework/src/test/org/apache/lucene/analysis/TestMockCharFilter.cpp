using namespace std;

#include "TestMockCharFilter.h"

namespace org::apache::lucene::analysis
{

void TestMockCharFilter::test() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  assertAnalyzesTo(analyzer, L"ab", std::deque<wstring>{L"aab"},
                   std::deque<int>{0}, std::deque<int>{2});

  assertAnalyzesTo(analyzer, L"aba", std::deque<wstring>{L"aabaa"},
                   std::deque<int>{0}, std::deque<int>{3});

  assertAnalyzesTo(analyzer, L"abcdefga", std::deque<wstring>{L"aabcdefgaa"},
                   std::deque<int>{0}, std::deque<int>{8});
}

TestMockCharFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestMockCharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestMockCharFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(tokenizer, tokenizer);
}

shared_ptr<Reader> TestMockCharFilter::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<MockCharFilter>(reader, 7);
}
} // namespace org::apache::lucene::analysis