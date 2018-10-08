using namespace std;

#include "TestDecimalDigitFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/SparseFixedBitSet.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/DecimalDigitFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SparseFixedBitSet = org::apache::lucene::util::SparseFixedBitSet;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::util::SparseFixedBitSet>
    TestDecimalDigitFilter::DECIMAL_DIGIT_CODEPOINTS;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void
// init_DECIMAL_DIGIT_CODEPOINTS()
void TestDecimalDigitFilter::init_DECIMAL_DIGIT_CODEPOINTS()
{
  DECIMAL_DIGIT_CODEPOINTS =
      make_shared<SparseFixedBitSet>(Character::MAX_CODE_POINT);
  for (int codepoint = Character::MIN_CODE_POINT;
       codepoint < Character::MAX_CODE_POINT; codepoint++) {
    if (isdigit(codepoint)) {
      DECIMAL_DIGIT_CODEPOINTS->set(codepoint);
    }
  }
  assert(0 < DECIMAL_DIGIT_CODEPOINTS->cardinality());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void
// destroy_DECIMAL_DIGIT_CODEPOINTS()
void TestDecimalDigitFilter::destroy_DECIMAL_DIGIT_CODEPOINTS()
{
  DECIMAL_DIGIT_CODEPOINTS.reset();
}

void TestDecimalDigitFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  tokenized = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  keyword = make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
}

TestDecimalDigitFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestDecimalDigitFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDecimalDigitFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<DecimalDigitFilter>(tokenizer));
}

TestDecimalDigitFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestDecimalDigitFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDecimalDigitFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<DecimalDigitFilter>(tokenizer));
}

void TestDecimalDigitFilter::tearDown() 
{
  delete tokenized;
  delete keyword;
  BaseTokenStreamTestCase::tearDown();
}

void TestDecimalDigitFilter::testSimple() 
{
  checkOneTerm(tokenized, L"١٢٣٤", L"1234");
}

void TestDecimalDigitFilter::testDoubleStruck() 
{
  // MATHEMATICAL DOUBLE-STRUCK DIGIT ... 1, 9, 8, 4
  const wstring input = L"𝟙 𝟡 𝟠 𝟜";
  const wstring expected = L"1 9 8 4";
  checkOneTerm(keyword, input, expected);
  checkOneTerm(keyword, input.replaceAll(L"\\s", L""),
               expected.replaceAll(L"\\s", L""));
}

void TestDecimalDigitFilter::testRandomSequences() 
{

  // test numIters random strings containing a sequence of numDigits codepoints
  constexpr int numIters = atLeast(5);
  for (int iter = 0; iter < numIters; iter++) {
    constexpr int numDigits = atLeast(20);
    shared_ptr<StringBuilder> *const expected = make_shared<StringBuilder>();
    shared_ptr<StringBuilder> *const actual = make_shared<StringBuilder>();
    for (int digitCounter = 0; digitCounter < numDigits; digitCounter++) {

      // increased odds of 0 length random string prefix
      const wstring prefix = random()->nextBoolean()
                                 ? L""
                                 : TestUtil::randomSimpleString(random());
      expected->append(prefix);
      actual->append(prefix);

      int codepoint = getRandomDecimalDigit(random());

      int value = Character::getNumericValue(codepoint);
      assert(value >= 0 && value <= 9);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      expected->append(Integer::toString(value));
      actual->appendCodePoint(codepoint);
    }
    // occasional suffix, increased odds of 0 length random string
    const wstring suffix =
        random()->nextBoolean() ? L"" : TestUtil::randomSimpleString(random());
    expected->append(suffix);
    actual->append(suffix);

    checkOneTerm(keyword, actual->toString(), expected->toString());
  }
}

void TestDecimalDigitFilter::testRandom() 
{
  int numCodePointsChecked = 0; // sanity check
  for (int codepoint = DECIMAL_DIGIT_CODEPOINTS->nextSetBit(0);
       codepoint != DocIdSetIterator::NO_MORE_DOCS;
       codepoint = DECIMAL_DIGIT_CODEPOINTS->nextSetBit(codepoint + 1)) {

    assert(isdigit(codepoint));

    // add some a-z before/after the string
    wstring prefix = TestUtil::randomSimpleString(random());
    wstring suffix = TestUtil::randomSimpleString(random());

    shared_ptr<StringBuilder> expected = make_shared<StringBuilder>();
    expected->append(prefix);
    int value = Character::getNumericValue(codepoint);
    assert(value >= 0 && value <= 9);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    expected->append(Integer::toString(value));
    expected->append(suffix);

    shared_ptr<StringBuilder> actual = make_shared<StringBuilder>();
    actual->append(prefix);
    actual->appendCodePoint(codepoint);
    actual->append(suffix);

    checkOneTerm(keyword, actual->toString(), expected->toString());

    numCodePointsChecked++;
  }
  assert(DECIMAL_DIGIT_CODEPOINTS->cardinality() == numCodePointsChecked);
}

void TestDecimalDigitFilter::testEmptyTerm() 
{
  checkOneTerm(keyword, L"", L"");
}

void TestDecimalDigitFilter::testRandomStrings() 
{
  checkRandomData(random(), tokenized, 1000 * RANDOM_MULTIPLIER);
}

int TestDecimalDigitFilter::getRandomDecimalDigit(shared_ptr<Random> r)
{
  constexpr int aprox =
      TestUtil::nextInt(r, 0, DECIMAL_DIGIT_CODEPOINTS->length() - 1);

  if (DECIMAL_DIGIT_CODEPOINTS->get(aprox)) { // lucky guess
    assert(isdigit(aprox));
    return aprox;
  }

  // seek up and down for closest set bit
  constexpr int lower = DECIMAL_DIGIT_CODEPOINTS->prevSetBit(aprox);
  constexpr int higher = DECIMAL_DIGIT_CODEPOINTS->nextSetBit(aprox);

  // sanity check edge cases
  if (lower < 0) {
    assert(higher != DocIdSetIterator::NO_MORE_DOCS);
    assert(isdigit(higher));
    return higher;
  }
  if (higher == DocIdSetIterator::NO_MORE_DOCS) {
    assert(0 <= lower);
    assert(isdigit(lower));
    return lower;
  }

  // which is closer?
  constexpr int cmp = Integer::compare(aprox - lower, higher - aprox);

  if (0 == cmp) {
    // dead even, flip a coin
    constexpr int result = random()->nextBoolean() ? lower : higher;
    assert(isdigit(result));
    return result;
  }

  constexpr int result = (cmp < 0) ? lower : higher;
  assert(isdigit(result));
  return result;
}
} // namespace org::apache::lucene::analysis::core