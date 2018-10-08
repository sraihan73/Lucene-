using namespace std;

#include "TestCapitalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/CapitalizationFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using org::junit::Test;
using namespace org::apache::lucene::analysis::miscellaneous;
//    import static
//    org.apache.lucene.analysis.miscellaneous.CapitalizationFilter.*;

void TestCapitalizationFilter::testCapitalization() 
{
  shared_ptr<CharArraySet> keep = make_shared<CharArraySet>(
      Arrays::asList(L"and", L"the", L"it", L"BIG"), false);

  assertCapitalizesTo(L"kiTTEN", std::deque<wstring>{L"Kitten"}, true, keep,
                      true, nullptr, 0,
                      CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
                      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  assertCapitalizesTo(L"and", std::deque<wstring>{L"And"}, true, keep, true,
                      nullptr, 0, CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
                      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  assertCapitalizesTo(L"AnD", std::deque<wstring>{L"And"}, true, keep, true,
                      nullptr, 0, CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
                      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  // first is not forced, but it's not a keep word, either
  assertCapitalizesTo(L"AnD", std::deque<wstring>{L"And"}, true, keep, false,
                      nullptr, 0, CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
                      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  assertCapitalizesTo(L"big", std::deque<wstring>{L"Big"}, true, keep, true,
                      nullptr, 0, CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
                      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  assertCapitalizesTo(L"BIG", std::deque<wstring>{L"BIG"}, true, keep, true,
                      nullptr, 0, CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
                      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  assertCapitalizesToKeyword(
      L"Hello thEre my Name is Ryan", L"Hello there my name is ryan", true,
      keep, true, nullptr, 0, CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  // now each token
  assertCapitalizesTo(
      L"Hello thEre my Name is Ryan",
      std::deque<wstring>{L"Hello", L"There", L"My", L"Name", L"Is", L"Ryan"},
      false, keep, true, nullptr, 0,
      CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  // now only the long words
  assertCapitalizesTo(
      L"Hello thEre my Name is Ryan",
      std::deque<wstring>{L"Hello", L"There", L"my", L"Name", L"is", L"Ryan"},
      false, keep, true, nullptr, 3,
      CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  // without prefix
  assertCapitalizesTo(L"McKinley", std::deque<wstring>{L"Mckinley"}, true,
                      keep, true, nullptr, 0,
                      CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
                      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  // Now try some prefixes
  deque<std::deque<wchar_t>> okPrefix = deque<std::deque<wchar_t>>();
  okPrefix.push_back((wstring(L"McK")).toCharArray());

  assertCapitalizesTo(L"McKinley", std::deque<wstring>{L"McKinley"}, true,
                      keep, true, okPrefix, 0,
                      CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
                      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  // now try some stuff with numbers
  assertCapitalizesTo(
      L"1st 2nd third", std::deque<wstring>{L"1st", L"2nd", L"Third"}, false,
      keep, false, nullptr, 0, CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);

  assertCapitalizesToKeyword(L"the The the", L"The The the", false, keep, true,
                             nullptr, 0,
                             CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
                             MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);
}

void TestCapitalizationFilter::assertCapitalizesTo(
    shared_ptr<Tokenizer> tokenizer, std::deque<wstring> &expected,
    bool onlyFirstWord, shared_ptr<CharArraySet> keep, bool forceFirstLetter,
    shared_ptr<deque<std::deque<wchar_t>>> okPrefix, int minWordLength,
    int maxWordCount, int maxTokenLength) 
{
  shared_ptr<CapitalizationFilter> filter = make_shared<CapitalizationFilter>(
      tokenizer, onlyFirstWord, keep, forceFirstLetter, okPrefix, minWordLength,
      maxWordCount, maxTokenLength);
  assertTokenStreamContents(filter, expected);
}

void TestCapitalizationFilter::assertCapitalizesTo(
    const wstring &input, std::deque<wstring> &expected, bool onlyFirstWord,
    shared_ptr<CharArraySet> keep, bool forceFirstLetter,
    shared_ptr<deque<std::deque<wchar_t>>> okPrefix, int minWordLength,
    int maxWordCount, int maxTokenLength) 
{
  shared_ptr<MockTokenizer> *const tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(make_shared<StringReader>(input));
  assertCapitalizesTo(tokenizer, expected, onlyFirstWord, keep,
                      forceFirstLetter, okPrefix, minWordLength, maxWordCount,
                      maxTokenLength);
}

void TestCapitalizationFilter::assertCapitalizesToKeyword(
    const wstring &input, const wstring &expected, bool onlyFirstWord,
    shared_ptr<CharArraySet> keep, bool forceFirstLetter,
    shared_ptr<deque<std::deque<wchar_t>>> okPrefix, int minWordLength,
    int maxWordCount, int maxTokenLength) 
{
  shared_ptr<MockTokenizer> *const tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  tokenizer->setReader(make_shared<StringReader>(input));
  assertCapitalizesTo(tokenizer, std::deque<wstring>{expected}, onlyFirstWord,
                      keep, forceFirstLetter, okPrefix, minWordLength,
                      maxWordCount, maxTokenLength);
}

void TestCapitalizationFilter::testRandomString() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

TestCapitalizationFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestCapitalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCapitalizationFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CapitalizationFilter>(tokenizer));
}

void TestCapitalizationFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestCapitalizationFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestCapitalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCapitalizationFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CapitalizationFilter>(tokenizer));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testIllegalArguments() throws Exception
void TestCapitalizationFilter::testIllegalArguments() 
{
  make_shared<CapitalizationFilter>(
      whitespaceMockTokenizer(L"accept only valid arguments"), true, nullptr,
      true, nullptr, -1, CapitalizationFilter::DEFAULT_MAX_WORD_COUNT,
      MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testIllegalArguments1() throws Exception
void TestCapitalizationFilter::testIllegalArguments1() 
{
  make_shared<CapitalizationFilter>(
      whitespaceMockTokenizer(L"accept only valid arguments"), true, nullptr,
      true, nullptr, 0, -10, MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testIllegalArguments2() throws Exception
void TestCapitalizationFilter::testIllegalArguments2() 
{
  make_shared<CapitalizationFilter>(
      whitespaceMockTokenizer(L"accept only valid arguments"), true, nullptr,
      true, nullptr, 0, CapitalizationFilter::DEFAULT_MAX_WORD_COUNT, -50);
}
} // namespace org::apache::lucene::analysis::miscellaneous