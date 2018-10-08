using namespace std;

#include "EdgeNGramTokenizerTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/ngram/EdgeNGramTokenizer.h"
#include "NGramTokenizerTest.h"

namespace org::apache::lucene::analysis::ngram
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomStrings;

void EdgeNGramTokenizerTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  input = make_shared<StringReader>(L"abcde");
}

void EdgeNGramTokenizerTest::testInvalidInput() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    (make_shared<EdgeNGramTokenizer>(0, 0))->setReader(input);
  });
}

void EdgeNGramTokenizerTest::testInvalidInput2() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    (make_shared<EdgeNGramTokenizer>(2, 1))->setReader(input);
  });
}

void EdgeNGramTokenizerTest::testInvalidInput3() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    (make_shared<EdgeNGramTokenizer>(-1, 2))->setReader(input);
  });
}

void EdgeNGramTokenizerTest::testFrontUnigram() 
{
  shared_ptr<EdgeNGramTokenizer> tokenizer =
      make_shared<EdgeNGramTokenizer>(1, 1);
  tokenizer->setReader(input);
  assertTokenStreamContents(tokenizer, std::deque<wstring>{L"a"},
                            std::deque<int>{0}, std::deque<int>{1}, 5);
}

void EdgeNGramTokenizerTest::testOversizedNgrams() 
{
  shared_ptr<EdgeNGramTokenizer> tokenizer =
      make_shared<EdgeNGramTokenizer>(6, 6);
  tokenizer->setReader(input);
  assertTokenStreamContents(tokenizer, std::deque<wstring>(0),
                            std::deque<int>(0), std::deque<int>(0), 5);
}

void EdgeNGramTokenizerTest::testFrontRangeOfNgrams() 
{
  shared_ptr<EdgeNGramTokenizer> tokenizer =
      make_shared<EdgeNGramTokenizer>(1, 3);
  tokenizer->setReader(input);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"a", L"ab", L"abc"},
      std::deque<int>{0, 0, 0}, std::deque<int>{1, 2, 3}, 5);
}

void EdgeNGramTokenizerTest::testReset() 
{
  shared_ptr<EdgeNGramTokenizer> tokenizer =
      make_shared<EdgeNGramTokenizer>(1, 3);
  tokenizer->setReader(input);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"a", L"ab", L"abc"},
      std::deque<int>{0, 0, 0}, std::deque<int>{1, 2, 3}, 5);
  tokenizer->setReader(make_shared<StringReader>(L"abcde"));
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"a", L"ab", L"abc"},
      std::deque<int>{0, 0, 0}, std::deque<int>{1, 2, 3}, 5);
}

void EdgeNGramTokenizerTest::testRandomStrings() 
{
  int numIters = TEST_NIGHTLY ? 10 : 1;
  for (int i = 0; i < numIters; i++) {
    constexpr int min = TestUtil::nextInt(random(), 2, 10);
    constexpr int max = TestUtil::nextInt(random(), min, 20);

    shared_ptr<Analyzer> a =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), min, max);
    checkRandomData(random(), a, 100 * RANDOM_MULTIPLIER, 20);
    checkRandomData(random(), a, 10 * RANDOM_MULTIPLIER, 8192);
    delete a;
  }
}

EdgeNGramTokenizerTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<EdgeNGramTokenizerTest> outerInstance, int min, int max)
{
  this->outerInstance = outerInstance;
  this->min = min;
  this->max = max;
}

shared_ptr<Analyzer::TokenStreamComponents>
EdgeNGramTokenizerTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<EdgeNGramTokenizer>(min, max);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void EdgeNGramTokenizerTest::testTokenizerPositions() 
{
  shared_ptr<EdgeNGramTokenizer> tokenizer =
      make_shared<EdgeNGramTokenizer>(1, 3);
  tokenizer->setReader(make_shared<StringReader>(L"abcde"));
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"a", L"ab", L"abc"},
      std::deque<int>{0, 0, 0}, std::deque<int>{1, 2, 3}, nullptr,
      std::deque<int>{1, 1, 1}, nullptr, nullopt, false);
}

void EdgeNGramTokenizerTest::testNGrams(
    int minGram, int maxGram, int length,
    const wstring &nonTokenChars) 
{
  const wstring s = RandomStrings::randomAsciiOfLength(random(), length);
  testNGrams(minGram, maxGram, s, nonTokenChars);
}

void EdgeNGramTokenizerTest::testNGrams(
    int minGram, int maxGram, const wstring &s,
    const wstring &nonTokenChars) 
{
  NGramTokenizerTest::testNGrams(minGram, maxGram, s, nonTokenChars, true);
}

void EdgeNGramTokenizerTest::testLargeInput() 
{
  // test sliding
  constexpr int minGram = TestUtil::nextInt(random(), 1, 100);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 100);
  testNGrams(minGram, maxGram, TestUtil::nextInt(random(), 3 * 1024, 4 * 1024),
             L"");
}

void EdgeNGramTokenizerTest::testLargeMaxGram() 
{
  // test sliding with maxGram > 1024
  constexpr int minGram = TestUtil::nextInt(random(), 1290, 1300);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 1300);
  testNGrams(minGram, maxGram, TestUtil::nextInt(random(), 3 * 1024, 4 * 1024),
             L"");
}

void EdgeNGramTokenizerTest::testPreTokenization() 
{
  constexpr int minGram = TestUtil::nextInt(random(), 1, 100);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 100);
  testNGrams(minGram, maxGram, TestUtil::nextInt(random(), 0, 4 * 1024), L"a");
}

void EdgeNGramTokenizerTest::testHeavyPreTokenization() 
{
  constexpr int minGram = TestUtil::nextInt(random(), 1, 100);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 100);
  testNGrams(minGram, maxGram, TestUtil::nextInt(random(), 0, 4 * 1024),
             L"abcdef");
}

void EdgeNGramTokenizerTest::testFewTokenChars() 
{
  const std::deque<wchar_t> chrs =
      std::deque<wchar_t>(TestUtil::nextInt(random(), 4000, 5000));
  Arrays::fill(chrs, L' ');
  for (int i = 0; i < chrs.size(); ++i) {
    if (random()->nextFloat() < 0.1) {
      chrs[i] = L'a';
    }
  }
  constexpr int minGram = TestUtil::nextInt(random(), 1, 2);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 2);
  testNGrams(minGram, maxGram, wstring(chrs), L" ");
}

void EdgeNGramTokenizerTest::testFullUTF8Range() 
{
  constexpr int minGram = TestUtil::nextInt(random(), 1, 100);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 100);
  const wstring s = TestUtil::randomUnicodeString(random(), 4 * 1024);
  testNGrams(minGram, maxGram, s, L"");
  testNGrams(minGram, maxGram, s, L"abcdef");
}
} // namespace org::apache::lucene::analysis::ngram