using namespace std;

#include "NGramTokenFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ASCIIFoldingFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ngram/NGramTokenFilter.h"

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
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;
using ASCIIFoldingFilter =
    org::apache::lucene::analysis::miscellaneous::ASCIIFoldingFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TestUtil = org::apache::lucene::util::TestUtil;

void NGramTokenFilterTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  input = whitespaceMockTokenizer(L"abcde");
}

void NGramTokenFilterTest::testInvalidInput() 
{
  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<NGramTokenFilter>(input, 2, 1, false); });
}

void NGramTokenFilterTest::testInvalidInput2() 
{
  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<NGramTokenFilter>(input, 0, 1, false); });
}

void NGramTokenFilterTest::testUnigrams() 
{
  shared_ptr<NGramTokenFilter> filter =
      make_shared<NGramTokenFilter>(input, 1, 1, false);
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"a", L"b", L"c", L"d", L"e"},
      std::deque<int>{0, 0, 0, 0, 0}, std::deque<int>{5, 5, 5, 5, 5},
      std::deque<int>{1, 0, 0, 0, 0});
}

void NGramTokenFilterTest::testBigrams() 
{
  shared_ptr<NGramTokenFilter> filter =
      make_shared<NGramTokenFilter>(input, 2, 2, false);
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"ab", L"bc", L"cd", L"de"},
      std::deque<int>{0, 0, 0, 0}, std::deque<int>{5, 5, 5, 5},
      std::deque<int>{1, 0, 0, 0});
}

void NGramTokenFilterTest::testNgrams() 
{
  shared_ptr<NGramTokenFilter> filter =
      make_shared<NGramTokenFilter>(input, 1, 3, false);
  assertTokenStreamContents(
      filter,
      std::deque<wstring>{L"a", L"ab", L"abc", L"b", L"bc", L"bcd", L"c",
                           L"cd", L"cde", L"d", L"de", L"e"},
      std::deque<int>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      std::deque<int>{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}, nullptr,
      std::deque<int>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, nullptr, nullopt,
      false);
}

void NGramTokenFilterTest::testNgramsNoIncrement() 
{
  shared_ptr<NGramTokenFilter> filter =
      make_shared<NGramTokenFilter>(input, 1, 3, false);
  assertTokenStreamContents(
      filter,
      std::deque<wstring>{L"a", L"ab", L"abc", L"b", L"bc", L"bcd", L"c",
                           L"cd", L"cde", L"d", L"de", L"e"},
      std::deque<int>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      std::deque<int>{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}, nullptr,
      std::deque<int>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, nullptr, nullopt,
      false);
}

void NGramTokenFilterTest::testOversizedNgrams() 
{
  shared_ptr<NGramTokenFilter> filter =
      make_shared<NGramTokenFilter>(input, 6, 7, false);
  assertTokenStreamContents(filter, std::deque<wstring>(0),
                            std::deque<int>(0), std::deque<int>(0));
}

void NGramTokenFilterTest::testOversizedNgramsPreserveOriginal() throw(
    runtime_error)
{
  shared_ptr<NGramTokenFilter> tokenizer =
      make_shared<NGramTokenFilter>(input, 6, 6, true);
  assertTokenStreamContents(tokenizer, std::deque<wstring>{L"abcde"},
                            std::deque<int>{0}, std::deque<int>{5});
}

void NGramTokenFilterTest::testSmallTokenInStream() 
{
  input = whitespaceMockTokenizer(L"abc de fgh");
  shared_ptr<NGramTokenFilter> tokenizer =
      make_shared<NGramTokenFilter>(input, 3, 3, false);
  assertTokenStreamContents(tokenizer, std::deque<wstring>{L"abc", L"fgh"},
                            std::deque<int>{0, 7}, std::deque<int>{3, 10},
                            std::deque<int>{1, 2});
}

void NGramTokenFilterTest::testSmallTokenInStreamPreserveOriginal() throw(
    runtime_error)
{
  input = whitespaceMockTokenizer(L"abc de fgh");
  shared_ptr<NGramTokenFilter> tokenizer =
      make_shared<NGramTokenFilter>(input, 3, 3, true);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"abc", L"de", L"fgh"},
      std::deque<int>{0, 4, 7}, std::deque<int>{3, 6, 10},
      std::deque<int>{1, 1, 1});
}

void NGramTokenFilterTest::testReset() 
{
  shared_ptr<WhitespaceTokenizer> tokenizer =
      make_shared<WhitespaceTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(L"abcde"));
  shared_ptr<NGramTokenFilter> filter =
      make_shared<NGramTokenFilter>(tokenizer, 1, 1, false);
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"a", L"b", L"c", L"d", L"e"},
      std::deque<int>{0, 0, 0, 0, 0}, std::deque<int>{5, 5, 5, 5, 5},
      std::deque<int>{1, 0, 0, 0, 0});
  tokenizer->setReader(make_shared<StringReader>(L"abcde"));
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"a", L"b", L"c", L"d", L"e"},
      std::deque<int>{0, 0, 0, 0, 0}, std::deque<int>{5, 5, 5, 5, 5},
      std::deque<int>{1, 0, 0, 0, 0});
}

void NGramTokenFilterTest::testKeepShortTermKeepLongTerm() 
{
  const wstring inputString = L"a bcd efghi jk";

  { // preserveOriginal = false
    shared_ptr<TokenStream> ts = whitespaceMockTokenizer(inputString);
    shared_ptr<NGramTokenFilter> filter =
        make_shared<NGramTokenFilter>(ts, 2, 3, false);
    assertTokenStreamContents(
        filter,
        std::deque<wstring>{L"bc", L"bcd", L"cd", L"ef", L"efg", L"fg", L"fgh",
                             L"gh", L"ghi", L"hi", L"jk"},
        std::deque<int>{2, 2, 2, 6, 6, 6, 6, 6, 6, 6, 12},
        std::deque<int>{5, 5, 5, 11, 11, 11, 11, 11, 11, 11, 14},
        std::deque<int>{2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1});
  }

  { // preserveOriginal = true
    shared_ptr<TokenStream> ts = whitespaceMockTokenizer(inputString);
    shared_ptr<NGramTokenFilter> filter =
        make_shared<NGramTokenFilter>(ts, 2, 3, true);
    assertTokenStreamContents(
        filter,
        std::deque<wstring>{L"a", L"bc", L"bcd", L"cd", L"ef", L"efg", L"fg",
                             L"fgh", L"gh", L"ghi", L"hi", L"efghi", L"jk"},
        std::deque<int>{0, 2, 2, 2, 6, 6, 6, 6, 6, 6, 6, 6, 12},
        std::deque<int>{1, 5, 5, 5, 11, 11, 11, 11, 11, 11, 11, 11, 14},
        std::deque<int>{1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1});
  }
}

void NGramTokenFilterTest::testInvalidOffsets() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  assertAnalyzesTo(analyzer, L"mosfellsbær",
                   std::deque<wstring>{L"mo", L"os", L"sf", L"fe", L"el",
                                        L"ll", L"ls", L"sb", L"ba", L"ae",
                                        L"er"},
                   std::deque<int>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                   std::deque<int>{11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
                   std::deque<int>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
  delete analyzer;
}

NGramTokenFilterTest::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<NGramTokenFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
NGramTokenFilterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenFilter> filters = make_shared<ASCIIFoldingFilter>(tokenizer);
  filters = make_shared<NGramTokenFilter>(filters, 2, 2, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filters);
}

void NGramTokenFilterTest::testEndPositionIncrement() 
{
  shared_ptr<TokenStream> source =
      whitespaceMockTokenizer(L"seventeen one two three four");
  shared_ptr<TokenStream> input =
      make_shared<NGramTokenFilter>(source, 8, 8, false);
  shared_ptr<PositionIncrementAttribute> posIncAtt =
      input->addAttribute(PositionIncrementAttribute::typeid);
  input->reset();
  while (input->incrementToken()) {
  }
  input->end();
  TestUtil::assertEquals(4, posIncAtt->getPositionIncrement());
}

void NGramTokenFilterTest::testRandomStrings() 
{
  for (int i = 0; i < 10; i++) {
    constexpr int min = TestUtil::nextInt(random(), 2, 10);
    constexpr int max = TestUtil::nextInt(random(), min, 20);
    constexpr bool preserveOriginal =
        TestUtil::nextInt(random(), 0, 1) % 2 == 0;

    shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
        shared_from_this(), min, max, preserveOriginal);
    checkRandomData(random(), a, 200 * RANDOM_MULTIPLIER, 20);
    delete a;
  }
}

NGramTokenFilterTest::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(shared_ptr<NGramTokenFilterTest> outerInstance,
                                 int min, int max, bool preserveOriginal)
{
  this->outerInstance = outerInstance;
  this->min = min;
  this->max = max;
  this->preserveOriginal = preserveOriginal;
}

shared_ptr<Analyzer::TokenStreamComponents>
NGramTokenFilterTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<NGramTokenFilter>(tokenizer, min, max, preserveOriginal));
}

void NGramTokenFilterTest::testEmptyTerm() 
{
  shared_ptr<Random> random = NGramTokenFilterTest::random();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkAnalysisConsistency(random, a, random->nextBoolean(), L"");
  delete a;
}

NGramTokenFilterTest::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(shared_ptr<NGramTokenFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
NGramTokenFilterTest::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<NGramTokenFilter>(tokenizer, 2, 15, false));
}

void NGramTokenFilterTest::testSupplementaryCharacters() 
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
    tk = make_shared<NGramTokenFilter>(tk, minGram, maxGram, preserveOriginal);
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

    for (int start = 0; start < codePointCount; ++start) {
      for (int end = start + minGram;
           end <= min(codePointCount, start + maxGram); ++end) {
        assertTrue(tk->incrementToken());
        TestUtil::assertEquals(0, offsetAtt->startOffset());
        TestUtil::assertEquals(s.length(), offsetAtt->endOffset());
        constexpr int startIndex = Character::offsetByCodePoints(s, 0, start);
        constexpr int endIndex = Character::offsetByCodePoints(s, 0, end);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        TestUtil::assertEquals(s.substr(startIndex, endIndex - startIndex),
                               termAtt->toString());
      }
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
} // namespace org::apache::lucene::analysis::ngram