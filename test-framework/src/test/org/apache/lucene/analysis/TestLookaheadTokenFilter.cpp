using namespace std;

#include "TestLookaheadTokenFilter.h"

namespace org::apache::lucene::analysis
{

void TestLookaheadTokenFilter::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  int maxLength = TEST_NIGHTLY ? 8192 : 1024;
  checkRandomData(random(), a, 50 * RANDOM_MULTIPLIER, maxLength);
}

TestLookaheadTokenFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestLookaheadTokenFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestLookaheadTokenFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Random> random = TestLookaheadTokenFilter::random();
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>(
      MockTokenizer::WHITESPACE, random->nextBoolean());
  shared_ptr<TokenStream> output =
      make_shared<MockRandomLookaheadTokenFilter>(random, tokenizer);
  return make_shared<TokenStreamComponents>(tokenizer, output);
}

TestLookaheadTokenFilter::NeverPeeksLookaheadTokenFilter::
    NeverPeeksLookaheadTokenFilter(shared_ptr<TokenStream> input)
    : LookaheadTokenFilter<LookaheadTokenFilter::Position>(input)
{
}

shared_ptr<Position>
TestLookaheadTokenFilter::NeverPeeksLookaheadTokenFilter::newPosition()
{
  return make_shared<Position>();
}

bool TestLookaheadTokenFilter::NeverPeeksLookaheadTokenFilter::
    incrementToken() 
{
  return nextToken();
}

void TestLookaheadTokenFilter::testNeverCallingPeek() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  int maxLength = TEST_NIGHTLY ? 8192 : 1024;
  checkRandomData(random(), a, 50 * RANDOM_MULTIPLIER, maxLength);
}

TestLookaheadTokenFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestLookaheadTokenFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestLookaheadTokenFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>(
      MockTokenizer::WHITESPACE, random()->nextBoolean());
  shared_ptr<TokenStream> output =
      make_shared<NeverPeeksLookaheadTokenFilter>(tokenizer);
  return make_shared<TokenStreamComponents>(tokenizer, output);
}

void TestLookaheadTokenFilter::testMissedFirstToken() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

  assertAnalyzesTo(analyzer, L"Only he who is running knows .",
                   std::deque<wstring>{L"Only", L"Only-huh?", L"he",
                                        L"he-huh?", L"who", L"who-huh?", L"is",
                                        L"is-huh?", L"running", L"running-huh?",
                                        L"knows", L"knows-huh?", L".",
                                        L".-huh?"});
}

TestLookaheadTokenFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestLookaheadTokenFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
TestLookaheadTokenFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TrivialLookaheadFilter> filter =
      make_shared<TrivialLookaheadFilter>(source);
  return make_shared<TokenStreamComponents>(source, filter);
}
} // namespace org::apache::lucene::analysis