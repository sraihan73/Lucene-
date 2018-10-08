using namespace std;

#include "NGramTokenizerTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"

namespace org::apache::lucene::analysis::ngram
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomStrings;

void NGramTokenizerTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  input = make_shared<StringReader>(L"abcde");
}

void NGramTokenizerTest::testInvalidInput() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<NGramTokenizer> tok = make_shared<NGramTokenizer>(2, 1);
  });
}

void NGramTokenizerTest::testInvalidInput2() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<NGramTokenizer> tok = make_shared<NGramTokenizer>(0, 1);
    tok->setReader(input);
  });
}

void NGramTokenizerTest::testUnigrams() 
{
  shared_ptr<NGramTokenizer> tokenizer = make_shared<NGramTokenizer>(1, 1);
  tokenizer->setReader(input);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"a", L"b", L"c", L"d", L"e"},
      std::deque<int>{0, 1, 2, 3, 4}, std::deque<int>{1, 2, 3, 4, 5}, 5);
}

void NGramTokenizerTest::testBigrams() 
{
  shared_ptr<NGramTokenizer> tokenizer = make_shared<NGramTokenizer>(2, 2);
  tokenizer->setReader(input);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"ab", L"bc", L"cd", L"de"},
      std::deque<int>{0, 1, 2, 3}, std::deque<int>{2, 3, 4, 5}, 5);
}

void NGramTokenizerTest::testNgrams() 
{
  shared_ptr<NGramTokenizer> tokenizer = make_shared<NGramTokenizer>(1, 3);
  tokenizer->setReader(input);
  assertTokenStreamContents(
      tokenizer,
      std::deque<wstring>{L"a", L"ab", L"abc", L"b", L"bc", L"bcd", L"c",
                           L"cd", L"cde", L"d", L"de", L"e"},
      std::deque<int>{0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 4},
      std::deque<int>{1, 2, 3, 2, 3, 4, 3, 4, 5, 4, 5, 5}, nullptr, nullptr,
      nullptr, 5, false);
}

void NGramTokenizerTest::testOversizedNgrams() 
{
  shared_ptr<NGramTokenizer> tokenizer = make_shared<NGramTokenizer>(6, 7);
  tokenizer->setReader(input);
  assertTokenStreamContents(tokenizer, std::deque<wstring>(0),
                            std::deque<int>(0), std::deque<int>(0), 5);
}

void NGramTokenizerTest::testReset() 
{
  shared_ptr<NGramTokenizer> tokenizer = make_shared<NGramTokenizer>(1, 1);
  tokenizer->setReader(input);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"a", L"b", L"c", L"d", L"e"},
      std::deque<int>{0, 1, 2, 3, 4}, std::deque<int>{1, 2, 3, 4, 5}, 5);
  tokenizer->setReader(make_shared<StringReader>(L"abcde"));
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"a", L"b", L"c", L"d", L"e"},
      std::deque<int>{0, 1, 2, 3, 4}, std::deque<int>{1, 2, 3, 4, 5}, 5);
}

void NGramTokenizerTest::testRandomStrings() 
{
  int numIters = TEST_NIGHTLY ? 10 : 1;
  for (int i = 0; i < numIters; i++) {
    constexpr int min = TestUtil::nextInt(random(), 2, 10);
    constexpr int max = TestUtil::nextInt(random(), min, 20);
    shared_ptr<Analyzer> a =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), min, max);
    checkRandomData(random(), a, 200 * RANDOM_MULTIPLIER, 20);
    checkRandomData(random(), a, 10 * RANDOM_MULTIPLIER, 1027);
    delete a;
  }
}

NGramTokenizerTest::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<NGramTokenizerTest> outerInstance, int min, int max)
{
  this->outerInstance = outerInstance;
  this->min = min;
  this->max = max;
}

shared_ptr<Analyzer::TokenStreamComponents>
NGramTokenizerTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<NGramTokenizer>(min, max);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void NGramTokenizerTest::testNGrams(
    int minGram, int maxGram, int length,
    const wstring &nonTokenChars) 
{
  const wstring s = RandomStrings::randomAsciiOfLength(random(), length);
  testNGrams(minGram, maxGram, s, nonTokenChars);
}

void NGramTokenizerTest::testNGrams(
    int minGram, int maxGram, const wstring &s,
    const wstring &nonTokenChars) 
{
  testNGrams(minGram, maxGram, s, nonTokenChars, false);
}

std::deque<int> NGramTokenizerTest::toCodePoints(shared_ptr<std::wstring> s)
{
  const std::deque<int> codePoints =
      std::deque<int>(Character::codePointCount(s, 0, s->length()));
  for (int i = 0, j = 0; i < s->length(); ++j) {
    codePoints[j] = Character::codePointAt(s, i);
    i += Character::charCount(codePoints[j]);
  }
  return codePoints;
}

bool NGramTokenizerTest::isTokenChar(const wstring &nonTokenChars,
                                     int codePoint)
{
  for (int i = 0; i < nonTokenChars.length();) {
    constexpr int cp = nonTokenChars.codePointAt(i);
    if (cp == codePoint) {
      return false;
    }
    i += Character::charCount(cp);
  }
  return true;
}

void NGramTokenizerTest::testNGrams(int minGram, int maxGram, const wstring &s,
                                    const wstring &nonTokenChars,
                                    bool edgesOnly) 
{
  // convert the string to code points
  const std::deque<int> codePoints = toCodePoints(s);
  const std::deque<int> offsets = std::deque<int>(codePoints.size() + 1);
  for (int i = 0; i < codePoints.size(); ++i) {
    offsets[i + 1] = offsets[i] + Character::charCount(codePoints[i]);
  }
  shared_ptr<Tokenizer> *const grams =
      make_shared<NGramTokenizerAnonymousInnerClass>(minGram, maxGram,
                                                     edgesOnly, nonTokenChars);
  grams->setReader(make_shared<StringReader>(s));
  shared_ptr<CharTermAttribute> *const termAtt =
      grams->addAttribute(CharTermAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> *const posIncAtt =
      grams->addAttribute(PositionIncrementAttribute::typeid);
  shared_ptr<PositionLengthAttribute> *const posLenAtt =
      grams->addAttribute(PositionLengthAttribute::typeid);
  shared_ptr<OffsetAttribute> *const offsetAtt =
      grams->addAttribute(OffsetAttribute::typeid);
  grams->reset();
  for (int start = 0; start < codePoints.size(); ++start) {
    for (int end = start + minGram;
         end <= start + maxGram && end <= codePoints.size(); ++end) {
      if (edgesOnly && start > 0 &&
          isTokenChar(nonTokenChars, codePoints[start - 1])) {
        // not on an edge
        goto nextGramContinue;
      }
      for (int j = start; j < end; ++j) {
        if (!isTokenChar(nonTokenChars, codePoints[j])) {
          goto nextGramContinue;
        }
      }
      assertTrue(grams->incrementToken());
      assertArrayEquals(Arrays::copyOfRange(codePoints, start, end),
                        toCodePoints(termAtt));
      TestUtil::assertEquals(1, posIncAtt->getPositionIncrement());
      TestUtil::assertEquals(1, posLenAtt->getPositionLength());
      TestUtil::assertEquals(offsets[start], offsetAtt->startOffset());
      TestUtil::assertEquals(offsets[end], offsetAtt->endOffset());
    nextGramContinue:;
    }
  nextGramBreak:;
  }
  assertFalse(grams->incrementToken());
  grams->end();
  TestUtil::assertEquals(s.length(), offsetAtt->startOffset());
  TestUtil::assertEquals(s.length(), offsetAtt->endOffset());
}

NGramTokenizerTest::NGramTokenizerAnonymousInnerClass::
    NGramTokenizerAnonymousInnerClass(int minGram, int maxGram, bool edgesOnly,
                                      const wstring &nonTokenChars)
    : NGramTokenizer(minGram, maxGram, edgesOnly)
{
  this->nonTokenChars = nonTokenChars;
}

bool NGramTokenizerTest::NGramTokenizerAnonymousInnerClass::isTokenChar(int chr)
{
  return nonTokenChars.find(chr) == wstring::npos;
}

void NGramTokenizerTest::testLargeInput() 
{
  // test sliding
  constexpr int minGram = TestUtil::nextInt(random(), 1, 100);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 100);
  testNGrams(minGram, maxGram, TestUtil::nextInt(random(), 3 * 1024, 4 * 1024),
             L"");
}

void NGramTokenizerTest::testLargeMaxGram() 
{
  // test sliding with maxGram > 1024
  constexpr int minGram = TestUtil::nextInt(random(), 1290, 1300);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 1300);
  testNGrams(minGram, maxGram, TestUtil::nextInt(random(), 3 * 1024, 4 * 1024),
             L"");
}

void NGramTokenizerTest::testPreTokenization() 
{
  constexpr int minGram = TestUtil::nextInt(random(), 1, 100);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 100);
  testNGrams(minGram, maxGram, TestUtil::nextInt(random(), 0, 4 * 1024), L"a");
}

void NGramTokenizerTest::testHeavyPreTokenization() 
{
  constexpr int minGram = TestUtil::nextInt(random(), 1, 100);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 100);
  testNGrams(minGram, maxGram, TestUtil::nextInt(random(), 0, 4 * 1024),
             L"abcdef");
}

void NGramTokenizerTest::testFewTokenChars() 
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

void NGramTokenizerTest::testFullUTF8Range() 
{
  constexpr int minGram = TestUtil::nextInt(random(), 1, 100);
  constexpr int maxGram = TestUtil::nextInt(random(), minGram, 100);
  const wstring s = TestUtil::randomUnicodeString(random(), 4 * 1024);
  testNGrams(minGram, maxGram, s, L"");
  testNGrams(minGram, maxGram, s, L"abcdef");
}
} // namespace org::apache::lucene::analysis::ngram