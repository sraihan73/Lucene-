using namespace std;

#include "EdgeNGramTokenFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LetterTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ngram/EdgeNGramTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/shingle/ShingleFilter.h"

namespace org::apache::lucene::analysis::ngram
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using LetterTokenizer = org::apache::lucene::analysis::core::LetterTokenizer;
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;
using ShingleFilter = org::apache::lucene::analysis::shingle::ShingleFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TestUtil = org::apache::lucene::util::TestUtil;

void EdgeNGramTokenFilterTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  input = whitespaceMockTokenizer(L"abcde");
}

void EdgeNGramTokenFilterTest::testInvalidInput() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<EdgeNGramTokenFilter>(input, 0, 0, false);
  });
}

void EdgeNGramTokenFilterTest::testInvalidInput2() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<EdgeNGramTokenFilter>(input, 2, 1, false);
  });
}

void EdgeNGramTokenFilterTest::testInvalidInput3() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<EdgeNGramTokenFilter>(input, -1, 2, false);
  });
}

void EdgeNGramTokenFilterTest::testFrontUnigram() 
{
  shared_ptr<EdgeNGramTokenFilter> tokenizer =
      make_shared<EdgeNGramTokenFilter>(input, 1, 1, false);
  assertTokenStreamContents(tokenizer, std::deque<wstring>{L"a"},
                            std::deque<int>{0}, std::deque<int>{5});
}

void EdgeNGramTokenFilterTest::testOversizedNgrams() 
{
  shared_ptr<EdgeNGramTokenFilter> tokenizer =
      make_shared<EdgeNGramTokenFilter>(input, 6, 6, false);
  assertTokenStreamContents(tokenizer, std::deque<wstring>(0),
                            std::deque<int>(0), std::deque<int>(0));
}

void EdgeNGramTokenFilterTest::testOversizedNgramsPreserveOriginal() throw(
    runtime_error)
{
  shared_ptr<EdgeNGramTokenFilter> tokenizer =
      make_shared<EdgeNGramTokenFilter>(input, 6, 6, true);
  assertTokenStreamContents(tokenizer, std::deque<wstring>{L"abcde"},
                            std::deque<int>{0}, std::deque<int>{5});
}

void EdgeNGramTokenFilterTest::testPreserveOriginal() 
{
  const wstring inputString = L"a bcd efghi jk";

  { // preserveOriginal = false
    shared_ptr<TokenStream> ts = whitespaceMockTokenizer(inputString);
    shared_ptr<EdgeNGramTokenFilter> filter =
        make_shared<EdgeNGramTokenFilter>(ts, 2, 3, false);
    assertTokenStreamContents(
        filter, std::deque<wstring>{L"bc", L"bcd", L"ef", L"efg", L"jk"},
        std::deque<int>{2, 2, 6, 6, 12}, std::deque<int>{5, 5, 11, 11, 14},
        std::deque<int>{2, 0, 1, 0, 1});
  }

  { // preserveOriginal = true
    shared_ptr<TokenStream> ts = whitespaceMockTokenizer(inputString);
    shared_ptr<EdgeNGramTokenFilter> filter =
        make_shared<EdgeNGramTokenFilter>(ts, 2, 3, true);
    assertTokenStreamContents(filter,
                              std::deque<wstring>{L"a", L"bc", L"bcd", L"ef",
                                                   L"efg", L"efghi", L"jk"},
                              std::deque<int>{0, 2, 2, 6, 6, 6, 12},
                              std::deque<int>{1, 5, 5, 11, 11, 11, 14},
                              std::deque<int>{1, 1, 0, 1, 0, 0, 1});
  }
}

void EdgeNGramTokenFilterTest::testFrontRangeOfNgrams() 
{
  shared_ptr<EdgeNGramTokenFilter> tokenizer =
      make_shared<EdgeNGramTokenFilter>(input, 1, 3, false);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"a", L"ab", L"abc"},
      std::deque<int>{0, 0, 0}, std::deque<int>{5, 5, 5});
}

void EdgeNGramTokenFilterTest::testFilterPositions() 
{
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(L"abcde vwxyz");
  shared_ptr<EdgeNGramTokenFilter> tokenizer =
      make_shared<EdgeNGramTokenFilter>(ts, 1, 3, false);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"a", L"ab", L"abc", L"v", L"vw", L"vwx"},
      std::deque<int>{0, 0, 0, 6, 6, 6},
      std::deque<int>{5, 5, 5, 11, 11, 11});
}

EdgeNGramTokenFilterTest::PositionFilter::PositionFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool EdgeNGramTokenFilterTest::PositionFilter::incrementToken() throw(
    IOException)
{
  if (input->incrementToken()) {
    if (started) {
      posIncrAtt->setPositionIncrement(0);
    } else {
      started = true;
    }
    return true;
  } else {
    return false;
  }
}

void EdgeNGramTokenFilterTest::PositionFilter::reset() 
{
  TokenFilter::reset();
  started = false;
}

void EdgeNGramTokenFilterTest::testFirstTokenPositionIncrement() throw(
    runtime_error)
{
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(L"a abc");
  ts = make_shared<PositionFilter>(
      ts); // All but first token will get 0 position increment
  shared_ptr<EdgeNGramTokenFilter> filter =
      make_shared<EdgeNGramTokenFilter>(ts, 2, 3, false);
  // The first token "a" will not be output, since it's smaller than the mingram
  // size of 2. The second token on input to EdgeNGramTokenFilter will have
  // position increment of 0, which should be increased to 1, since this is the
  // first output token in the stream.
  assertTokenStreamContents(filter, std::deque<wstring>{L"ab", L"abc"},
                            std::deque<int>{2, 2}, std::deque<int>{5, 5},
                            std::deque<int>{1, 0});
}

void EdgeNGramTokenFilterTest::testSmallTokenInStream() 
{
  input = whitespaceMockTokenizer(L"abc de fgh");
  shared_ptr<EdgeNGramTokenFilter> tokenizer =
      make_shared<EdgeNGramTokenFilter>(input, 3, 3, false);
  assertTokenStreamContents(tokenizer, std::deque<wstring>{L"abc", L"fgh"},
                            std::deque<int>{0, 7}, std::deque<int>{3, 10});
}

void EdgeNGramTokenFilterTest::testReset() 
{
  shared_ptr<WhitespaceTokenizer> tokenizer =
      make_shared<WhitespaceTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(L"abcde"));
  shared_ptr<EdgeNGramTokenFilter> filter =
      make_shared<EdgeNGramTokenFilter>(tokenizer, 1, 3, false);
  assertTokenStreamContents(filter, std::deque<wstring>{L"a", L"ab", L"abc"},
                            std::deque<int>{0, 0, 0},
                            std::deque<int>{5, 5, 5});
  tokenizer->setReader(make_shared<StringReader>(L"abcde"));
  assertTokenStreamContents(filter, std::deque<wstring>{L"a", L"ab", L"abc"},
                            std::deque<int>{0, 0, 0},
                            std::deque<int>{5, 5, 5});
}

void EdgeNGramTokenFilterTest::testRandomStrings() 
{
  for (int i = 0; i < 10; i++) {
    constexpr int min = TestUtil::nextInt(random(), 2, 10);
    constexpr int max = TestUtil::nextInt(random(), min, 20);
    constexpr bool preserveOriginal =
        TestUtil::nextInt(random(), 0, 1) % 2 == 0;

    shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass>(
        shared_from_this(), min, max, preserveOriginal);
    checkRandomData(random(), a, 100 * RANDOM_MULTIPLIER);
    delete a;
  }
}

EdgeNGramTokenFilterTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<EdgeNGramTokenFilterTest> outerInstance, int min, int max,
        bool preserveOriginal)
{
  this->outerInstance = outerInstance;
  this->min = min;
  this->max = max;
  this->preserveOriginal = preserveOriginal;
}

shared_ptr<Analyzer::TokenStreamComponents>
EdgeNGramTokenFilterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<EdgeNGramTokenFilter>(tokenizer, min, max, preserveOriginal));
}

void EdgeNGramTokenFilterTest::testEmptyTerm() 
{
  shared_ptr<Random> random = EdgeNGramTokenFilterTest::random();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkAnalysisConsistency(random, a, random->nextBoolean(), L"");
  delete a;
}

EdgeNGramTokenFilterTest::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<EdgeNGramTokenFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
EdgeNGramTokenFilterTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<EdgeNGramTokenFilter>(tokenizer, 2, 15, false));
}

void EdgeNGramTokenFilterTest::testGraphs() 
{
  shared_ptr<TokenStream> tk = make_shared<LetterTokenizer>();
  (std::static_pointer_cast<Tokenizer>(tk))
      ->setReader(make_shared<StringReader>(L"abc d efgh ij klmno p q"));
  tk = make_shared<ShingleFilter>(tk);
  tk = make_shared<EdgeNGramTokenFilter>(tk, 7, 10, false);
  assertTokenStreamContents(
      tk, std::deque<wstring>{L"efgh ij", L"ij klmn", L"ij klmno", L"klmno p"},
      std::deque<int>{6, 11, 11, 14}, std::deque<int>{13, 19, 19, 21},
      std::deque<int>{3, 1, 0, 1}, std::deque<int>{2, 2, 2, 2}, 23);
}

void EdgeNGramTokenFilterTest::testSupplementaryCharacters() 
{
  for (int i = 0; i < 20; i++) {
    const wstring s = TestUtil::randomUnicodeString(random(), 10);
    constexpr int codePointCount = s.codePointCount(0, s.length());
    constexpr int minGram = TestUtil::nextInt(random(), 1, 3);
    constexpr int maxGram = TestUtil::nextInt(random(), minGram, 10);
    constexpr bool preserveOriginal =
        TestUtil::nextInt(random(), 0, 1) % 2 == 0;

    shared_ptr<TokenStream> tk = make_shared<KeywordTokenizer>();
    (std::static_pointer_cast<Tokenizer>(tk))
        ->setReader(make_shared<StringReader>(s));
    tk = make_shared<EdgeNGramTokenFilter>(tk, minGram, maxGram,
                                           preserveOriginal);
    shared_ptr<CharTermAttribute> *const termAtt =
        tk->addAttribute(CharTermAttribute::typeid);
    shared_ptr<OffsetAttribute> *const offsetAtt =
        tk->addAttribute(OffsetAttribute::typeid);
    tk->reset();

    if (codePointCount < minGram && preserveOriginal) {
      assertTrue(tk->incrementToken());
      TestUtil::assertEquals(0, offsetAtt->startOffset());
      TestUtil::assertEquals(s.length(), offsetAtt->endOffset());
      // C++ TODO: There is no native C++ equivalent to 'toString':
      TestUtil::assertEquals(s, termAtt->toString());
    }

    for (int j = minGram; j <= min(codePointCount, maxGram); j++) {
      assertTrue(tk->incrementToken());
      TestUtil::assertEquals(0, offsetAtt->startOffset());
      TestUtil::assertEquals(s.length(), offsetAtt->endOffset());
      constexpr int end = Character::offsetByCodePoints(s, 0, j);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      TestUtil::assertEquals(s.substr(0, end), termAtt->toString());
    }

    if (codePointCount > maxGram && preserveOriginal) {
      assertTrue(tk->incrementToken());
      TestUtil::assertEquals(0, offsetAtt->startOffset());
      TestUtil::assertEquals(s.length(), offsetAtt->endOffset());
      // C++ TODO: There is no native C++ equivalent to 'toString':
      TestUtil::assertEquals(s, termAtt->toString());
    }

    assertFalse(tk->incrementToken());
    delete tk;
  }
}

void EdgeNGramTokenFilterTest::testEndPositionIncrement() 
{
  shared_ptr<TokenStream> source =
      whitespaceMockTokenizer(L"seventeen one two three four");
  shared_ptr<TokenStream> input =
      make_shared<EdgeNGramTokenFilter>(source, 8, 8, false);
  shared_ptr<PositionIncrementAttribute> posIncAtt =
      input->addAttribute(PositionIncrementAttribute::typeid);
  input->reset();
  while (input->incrementToken()) {
  }
  input->end();
  TestUtil::assertEquals(4, posIncAtt->getPositionIncrement());
}
} // namespace org::apache::lucene::analysis::ngram