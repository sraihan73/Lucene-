using namespace std;

#include "TestWordDelimiterGraphFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/WordDelimiterGraphFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/WordDelimiterIterator.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using namespace org::apache::lucene::analysis;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
using namespace org::apache::lucene::analysis::miscellaneous;
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterGraphFilter.*;
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterIterator.DEFAULT_WORD_DELIM_TABLE;

void TestWordDelimiterGraphFilter::testOffsets() 
{
  int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterGraphFilter::CATENATE_ALL |
              WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
              WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
  // test that subwords and catenated subwords have
  // the correct offsets.
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(
          make_shared<CannedTokenStream>(make_shared<Token>(L"foo-bar", 5, 12)),
          DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(
      wdf, std::deque<wstring>{L"foobar", L"foo", L"bar"},
      std::deque<int>{5, 5, 9}, std::deque<int>{12, 8, 12});

  // with illegal offsets:
  wdf = make_shared<WordDelimiterGraphFilter>(
      make_shared<CannedTokenStream>(make_shared<Token>(L"foo-bar", 5, 6)),
      DEFAULT_WORD_DELIM_TABLE, flags, nullptr);
  assertTokenStreamContents(
      wdf, std::deque<wstring>{L"foobar", L"foo", L"bar"},
      std::deque<int>{5, 5, 5}, std::deque<int>{6, 6, 6});
}

void TestWordDelimiterGraphFilter::testOffsetChange() 
{
  int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterGraphFilter::CATENATE_ALL |
              WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
              WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(
          make_shared<CannedTokenStream>(
              make_shared<Token>(L"übelkeit)", 7, 16)),
          DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(wdf, std::deque<wstring>{L"übelkeit"},
                            std::deque<int>{7}, std::deque<int>{15});
}

void TestWordDelimiterGraphFilter::testOffsetChange2() 
{
  int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterGraphFilter::CATENATE_ALL |
              WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
              WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(
          make_shared<CannedTokenStream>(
              make_shared<Token>(L"(übelkeit", 7, 17)),
          DEFAULT_WORD_DELIM_TABLE, flags, nullptr);
  // illegal offsets:
  assertTokenStreamContents(wdf, std::deque<wstring>{L"übelkeit"},
                            std::deque<int>{7}, std::deque<int>{17});
}

void TestWordDelimiterGraphFilter::testOffsetChange3() 
{
  int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterGraphFilter::CATENATE_ALL |
              WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
              WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(
          make_shared<CannedTokenStream>(
              make_shared<Token>(L"(übelkeit", 7, 16)),
          DEFAULT_WORD_DELIM_TABLE, flags, nullptr);
  assertTokenStreamContents(wdf, std::deque<wstring>{L"übelkeit"},
                            std::deque<int>{8}, std::deque<int>{16});
}

void TestWordDelimiterGraphFilter::testOffsetChange4() 
{
  int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterGraphFilter::CATENATE_ALL |
              WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
              WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(
          make_shared<CannedTokenStream>(
              make_shared<Token>(L"(foo,bar)", 7, 16)),
          DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(
      wdf, std::deque<wstring>{L"foobar", L"foo", L"bar"},
      std::deque<int>{8, 8, 12}, std::deque<int>{15, 11, 15});
}

void TestWordDelimiterGraphFilter::doSplit(
    const wstring &input, deque<wstring> &output) 
{
  int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
              WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(
          keywordMockTokenizer(input),
          WordDelimiterIterator::DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(wdf, output);
}

void TestWordDelimiterGraphFilter::testSplits() 
{
  doSplit(L"basic-split", {L"basic", L"split"});
  doSplit(L"camelCase", {L"camel", L"Case"});

  // non-space marking symbol shouldn't cause split
  // this is an example in Thai
  doSplit(L"\u0e1a\u0e49\u0e32\u0e19", {L"\u0e1a\u0e49\u0e32\u0e19"});
  // possessive followed by delimiter
  doSplit(L"test's'", {L"test"});

  // some russian upper and lowercase
  doSplit(L"Роберт", {L"Роберт"});
  // now cause a split (russian camelCase)
  doSplit(L"РобЕрт", {L"Роб", L"Ерт"});

  // a composed titlecase character, don't split
  doSplit(L"aǅungla", {L"aǅungla"});

  // a modifier letter, don't split
  doSplit(L"ســـــــــــــــــلام", {L"ســـــــــــــــــلام"});

  // enclosing mark, don't split
  doSplit(L"test⃝", {L"test⃝"});

  // combining spacing mark (the virama), don't split
  doSplit(L"हिन्दी", {L"हिन्दी"});

  // don't split non-ascii digits
  doSplit(L"١٢٣٤", {L"١٢٣٤"});

  // don't split supplementaries into unpaired surrogates
  doSplit(L"𠀀𠀀", {L"𠀀𠀀"});
}

void TestWordDelimiterGraphFilter::doSplitPossessive(
    int stemPossessive, const wstring &input,
    deque<wstring> &output) 
{
  int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterGraphFilter::SPLIT_ON_NUMERICS;
  flags |= (stemPossessive == 1)
               ? WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE
               : 0;
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(keywordMockTokenizer(input), flags,
                                            nullptr);

  assertTokenStreamContents(wdf, output);
}

void TestWordDelimiterGraphFilter::testPossessives() 
{
  doSplitPossessive(1, L"ra's", {L"ra"});
  doSplitPossessive(0, L"ra's", {L"ra", L"s"});
}

void TestWordDelimiterGraphFilter::testTokenType() 
{
  int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterGraphFilter::CATENATE_ALL |
              WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
              WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
  // test that subwords and catenated subwords have
  // the correct offsets.
  shared_ptr<Token> token = make_shared<Token>(L"foo-bar", 5, 12);
  token->setType(L"mytype");
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(
          make_shared<CannedTokenStream>(token), DEFAULT_WORD_DELIM_TABLE,
          flags, nullptr);

  assertTokenStreamContents(
      wdf, std::deque<wstring>{L"foobar", L"foo", L"bar"},
      std::deque<wstring>{L"mytype", L"mytype", L"mytype"});
}

TestWordDelimiterGraphFilter::LargePosIncTokenFilter::LargePosIncTokenFilter(
    shared_ptr<TokenStream> input)
    : TokenFilter(input)
{
}

bool TestWordDelimiterGraphFilter::LargePosIncTokenFilter::
    incrementToken() 
{
  if (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    if (termAtt->toString()->equals(L"largegap") ||
        termAtt->toString()->equals(L"/")) {
      posIncAtt->setPositionIncrement(10);
    }
    return true;
  } else {
    return false;
  }
}

void TestWordDelimiterGraphFilter::testPositionIncrements() 
{
  constexpr int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                        WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
                        WordDelimiterGraphFilter::CATENATE_ALL |
                        WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                        WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
                        WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<CharArraySet> *const protWords = make_shared<CharArraySet>(
      unordered_set<>(Arrays::asList(L"NUTCH")), false);

  /* analyzer that uses whitespace + wdf */
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), flags, protWords);

  /* in this case, works as expected. */
  assertAnalyzesTo(a, L"LUCENE / SOLR",
                   std::deque<wstring>{L"LUCENE", L"SOLR"},
                   std::deque<int>{0, 9}, std::deque<int>{6, 13}, nullptr,
                   std::deque<int>{1, 2}, nullptr, false);

  /* only in this case, posInc of 2 ?! */
  assertAnalyzesTo(a, L"LUCENE / solR",
                   std::deque<wstring>{L"LUCENE", L"solR", L"sol", L"R"},
                   std::deque<int>{0, 9, 9, 12},
                   std::deque<int>{6, 13, 12, 13}, nullptr,
                   std::deque<int>{1, 2, 0, 1}, nullptr, false);

  assertAnalyzesTo(a, L"LUCENE / NUTCH SOLR",
                   std::deque<wstring>{L"LUCENE", L"NUTCH", L"SOLR"},
                   std::deque<int>{0, 9, 15}, std::deque<int>{6, 14, 19},
                   nullptr, std::deque<int>{1, 2, 1}, nullptr, false);

  /* analyzer that will consume tokens with large position increments */
  shared_ptr<Analyzer> a2 = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), flags, protWords);

  /* increment of "largegap" is preserved */
  assertAnalyzesTo(a2, L"LUCENE largegap SOLR",
                   std::deque<wstring>{L"LUCENE", L"largegap", L"SOLR"},
                   std::deque<int>{0, 7, 16}, std::deque<int>{6, 15, 20},
                   nullptr, std::deque<int>{1, 10, 1}, nullptr, false);

  /* the "/" had a position increment of 10, where did it go?!?!! */
  assertAnalyzesTo(a2, L"LUCENE / SOLR",
                   std::deque<wstring>{L"LUCENE", L"SOLR"},
                   std::deque<int>{0, 9}, std::deque<int>{6, 13}, nullptr,
                   std::deque<int>{1, 11}, nullptr, false);

  /* in this case, the increment of 10 from the "/" is carried over */
  assertAnalyzesTo(a2, L"LUCENE / solR",
                   std::deque<wstring>{L"LUCENE", L"solR", L"sol", L"R"},
                   std::deque<int>{0, 9, 9, 12},
                   std::deque<int>{6, 13, 12, 13}, nullptr,
                   std::deque<int>{1, 11, 0, 1}, nullptr, false);

  assertAnalyzesTo(a2, L"LUCENE / NUTCH SOLR",
                   std::deque<wstring>{L"LUCENE", L"NUTCH", L"SOLR"},
                   std::deque<int>{0, 9, 15}, std::deque<int>{6, 14, 19},
                   nullptr, std::deque<int>{1, 11, 1}, nullptr, false);

  shared_ptr<Analyzer> a3 = make_shared<AnalyzerAnonymousInnerClass3>(
      shared_from_this(), flags, protWords);

  assertAnalyzesTo(a3, L"lucene.solr",
                   std::deque<wstring>{L"lucenesolr", L"lucene", L"solr"},
                   std::deque<int>{0, 0, 7}, std::deque<int>{11, 6, 11},
                   nullptr, std::deque<int>{1, 0, 1}, nullptr, false);

  /* the stopword should add a gap here */
  assertAnalyzesTo(a3, L"the lucene.solr",
                   std::deque<wstring>{L"lucenesolr", L"lucene", L"solr"},
                   std::deque<int>{4, 4, 11}, std::deque<int>{15, 10, 15},
                   nullptr, std::deque<int>{2, 0, 1}, nullptr, false);

  IOUtils::close({a, a2, a3});
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protWords = protWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterGraphFilter>(tokenizer, flags, protWords));
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protWords = protWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterGraphFilter>(
          make_shared<LargePosIncTokenFilter>(tokenizer), flags, protWords));
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protWords = protWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<StopFilter> filter =
      make_shared<StopFilter>(tokenizer, StandardAnalyzer::STOP_WORDS_SET);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterGraphFilter>(filter, flags, protWords));
}

void TestWordDelimiterGraphFilter::testKeywordFilter() 
{
  assertAnalyzesTo(
      keywordTestAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS),
      L"abc-def klm-nop kpop",
      std::deque<wstring>{L"abc", L"def", L"klm", L"nop", L"kpop"});
  assertAnalyzesTo(
      keywordTestAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                          WordDelimiterGraphFilter::IGNORE_KEYWORDS),
      L"abc-def klm-nop kpop",
      std::deque<wstring>{L"abc", L"def", L"klm-nop", L"kpop"},
      std::deque<int>{0, 4, 8, 16}, std::deque<int>{3, 7, 15, 20}, nullptr,
      std::deque<int>{1, 1, 1, 1}, nullptr, false);
}

shared_ptr<Analyzer> TestWordDelimiterGraphFilter::keywordTestAnalyzer(
    int flags) 
{
  return make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this(), flags);
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<KeywordMarkerFilter> kFilter =
      make_shared<KeywordMarkerFilterAnonymousInnerClass>(shared_from_this(),
                                                          tokenizer);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterGraphFilter>(kFilter, flags, nullptr));
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass4::
    KeywordMarkerFilterAnonymousInnerClass::
        KeywordMarkerFilterAnonymousInnerClass(
            shared_ptr<AnalyzerAnonymousInnerClass4> outerInstance,
            shared_ptr<org::apache::lucene::analysis::Tokenizer> tokenizer)
    : KeywordMarkerFilter(tokenizer)
{
  this->outerInstance = outerInstance;
  term = addAttribute(CharTermAttribute::typeid);
}

bool TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass4::
    KeywordMarkerFilterAnonymousInnerClass::isKeyword()
{
  // Marks terms starting with the letter 'k' as keywords
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return term->toString()->charAt(0) == L'k';
}

void TestWordDelimiterGraphFilter::testLotsOfConcatenating() throw(
    runtime_error)
{
  constexpr int flags = WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                        WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
                        WordDelimiterGraphFilter::CATENATE_WORDS |
                        WordDelimiterGraphFilter::CATENATE_NUMBERS |
                        WordDelimiterGraphFilter::CATENATE_ALL |
                        WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                        WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
                        WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;

  /* analyzer that uses whitespace + wdf */
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this(), flags);

  assertAnalyzesTo(a, L"abc-def-123-456",
                   std::deque<wstring>{L"abcdef123456", L"abcdef", L"abc",
                                        L"def", L"123456", L"123", L"456"},
                   std::deque<int>{0, 0, 0, 4, 8, 8, 12},
                   std::deque<int>{15, 7, 3, 7, 15, 11, 15}, nullptr,
                   std::deque<int>{1, 0, 0, 1, 1, 0, 1}, nullptr, false);
  delete a;
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass5::
    AnalyzerAnonymousInnerClass5(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterGraphFilter>(tokenizer, flags, nullptr));
}

void TestWordDelimiterGraphFilter::testLotsOfConcatenating2() throw(
    runtime_error)
{
  constexpr int flags = WordDelimiterGraphFilter::PRESERVE_ORIGINAL |
                        WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                        WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
                        WordDelimiterGraphFilter::CATENATE_WORDS |
                        WordDelimiterGraphFilter::CATENATE_NUMBERS |
                        WordDelimiterGraphFilter::CATENATE_ALL |
                        WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                        WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
                        WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;

  /* analyzer that uses whitespace + wdf */
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass6>(shared_from_this(), flags);

  assertAnalyzesTo(a, L"abc-def-123-456",
                   std::deque<wstring>{L"abcdef123456", L"abc-def-123-456",
                                        L"abcdef", L"abc", L"def", L"123456",
                                        L"123", L"456"},
                   std::deque<int>{0, 0, 0, 0, 4, 8, 8, 12},
                   std::deque<int>{15, 15, 7, 3, 7, 15, 11, 15}, nullptr,
                   std::deque<int>{1, 0, 0, 0, 1, 1, 0, 1}, nullptr, false);
  delete a;
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass6::
    AnalyzerAnonymousInnerClass6(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass6::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterGraphFilter>(tokenizer, flags, nullptr));
}

void TestWordDelimiterGraphFilter::testRandomStrings() 
{
  int numIterations = atLeast(5);
  for (int i = 0; i < numIterations; i++) {
    constexpr int flags = random()->nextInt(512);
    shared_ptr<CharArraySet> *const protectedWords;
    if (random()->nextBoolean()) {
      protectedWords = make_shared<CharArraySet>(
          unordered_set<>(Arrays::asList(L"a", L"b", L"cd")), false);
    } else {
      protectedWords.reset();
    }

    shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass7>(
        shared_from_this(), flags, protectedWords);
    // TODO: properly support positionLengthAttribute
    checkRandomData(random(), a, 200 * RANDOM_MULTIPLIER, 20, false, false);
    delete a;
  }
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass7::
    AnalyzerAnonymousInnerClass7(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protectedWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protectedWords = protectedWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass7::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterGraphFilter>(tokenizer, flags, protectedWords));
}

void TestWordDelimiterGraphFilter::testRandomHugeStrings() 
{
  int numIterations = atLeast(5);
  for (int i = 0; i < numIterations; i++) {
    constexpr int flags = random()->nextInt(512);
    shared_ptr<CharArraySet> *const protectedWords;
    if (random()->nextBoolean()) {
      protectedWords = make_shared<CharArraySet>(
          unordered_set<>(Arrays::asList(L"a", L"b", L"cd")), false);
    } else {
      protectedWords.reset();
    }

    shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass8>(
        shared_from_this(), flags, protectedWords);
    // TODO: properly support positionLengthAttribute
    checkRandomData(random(), a, 20 * RANDOM_MULTIPLIER, 8192, false, false);
    delete a;
  }
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass8::
    AnalyzerAnonymousInnerClass8(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protectedWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protectedWords = protectedWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass8::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> wdgf =
      make_shared<WordDelimiterGraphFilter>(tokenizer, flags, protectedWords);
  return make_shared<TokenStreamComponents>(tokenizer, wdgf);
}

void TestWordDelimiterGraphFilter::testEmptyTerm() 
{
  shared_ptr<Random> random = TestWordDelimiterGraphFilter::random();
  for (int i = 0; i < 512; i++) {
    constexpr int flags = i;
    shared_ptr<CharArraySet> *const protectedWords;
    if (random->nextBoolean()) {
      protectedWords = make_shared<CharArraySet>(
          unordered_set<>(Arrays::asList(L"a", L"b", L"cd")), false);
    } else {
      protectedWords.reset();
    }

    shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass9>(
        shared_from_this(), flags, protectedWords);
    // depending upon options, this thing may or may not preserve the empty term
    checkAnalysisConsistency(random, a, random->nextBoolean(), L"");
    delete a;
  }
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass9::
    AnalyzerAnonymousInnerClass9(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protectedWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protectedWords = protectedWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass9::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterGraphFilter>(tokenizer, flags, protectedWords));
}

shared_ptr<Analyzer> TestWordDelimiterGraphFilter::getAnalyzer(int flags)
{
  return getAnalyzer(flags, nullptr);
}

shared_ptr<Analyzer> TestWordDelimiterGraphFilter::getAnalyzer(
    int flags, shared_ptr<CharArraySet> protectedWords)
{
  return make_shared<AnalyzerAnonymousInnerClass10>(shared_from_this(), flags,
                                                    protectedWords);
}

TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass10::
    AnalyzerAnonymousInnerClass10(
        shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protectedWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protectedWords = protectedWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterGraphFilter::AnalyzerAnonymousInnerClass10::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterGraphFilter>(tokenizer, flags, protectedWords));
}

bool TestWordDelimiterGraphFilter::has(int flags, int flag)
{
  return (flags & flag) != 0;
}

bool TestWordDelimiterGraphFilter::isEnglishPossessive(const wstring &text,
                                                       int pos)
{
  if (pos > 2) {
    if ((text[pos - 1] == L's' || text[pos - 1] == L'S') &&
        (pos == text.length() || text[pos] != L'-')) {
      text = text.substr(0, text.length() - 2);
    }
  }
  return true;
}

TestWordDelimiterGraphFilter::WordPart::WordPart(const wstring &text,
                                                 int startOffset, int endOffset)
    : part(text.substr(startOffset, endOffset - startOffset)),
      startOffset(startOffset), endOffset(endOffset), type(toType(part[0]))
{
}

wstring TestWordDelimiterGraphFilter::WordPart::toString()
{
  return L"WordPart(" + part + L" " + to_wstring(startOffset) + L"-" +
         to_wstring(endOffset) + L")";
}

int TestWordDelimiterGraphFilter::toType(wchar_t ch)
{
  if (isdigit(ch)) {
    // numbers
    return NUMBER;
  } else if (isalpha(ch)) {
    // letters
    return LETTER;
  } else {
    // delimiter
    return DELIM;
  }
}

shared_ptr<Set<wstring>>
TestWordDelimiterGraphFilter::slowWDF(const wstring &text, int flags)
{

  // first make word parts:
  deque<std::shared_ptr<WordPart>> wordParts =
      deque<std::shared_ptr<WordPart>>();
  int lastCH = -1;
  int wordPartStart = 0;
  bool inToken = false;

  for (int i = 0; i < text.length(); i++) {
    wchar_t ch = text[i];
    if (toType(ch) == DELIM) {
      // delimiter
      if (inToken) {
        // end current token
        wordParts.push_back(make_shared<WordPart>(text, wordPartStart, i));
        inToken = false;
      }

      // strip english possessive at the end of this token?:
      if (has(flags, WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE) &&
          ch == L'\'' && i > 0 && i < text.length() - 1 &&
          (text[i + 1] == L's' || text[i + 1] == L'S') &&
          toType(text[i - 1]) == LETTER &&
          (i + 2 == text.length() || toType(text[i + 2]) == DELIM)) {
        i += 2;
      }

    } else if (inToken == false) {
      // start new token
      inToken = true;
      wordPartStart = i;
    } else {
      bool newToken = false;
      if (isalpha(lastCH)) {
        if (isalpha(ch)) {
          if (has(flags, WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE) &&
              islower(lastCH) && islower(ch) == false) {
            // start new token on lower -> UPPER case change (but not vice
            // versa!)
            newToken = true;
          }
        } else if (has(flags, WordDelimiterGraphFilter::SPLIT_ON_NUMERICS) &&
                   isdigit(ch)) {
          // start new token on letter -> number change
          newToken = true;
        }
      } else {
        assert(isdigit(lastCH));
        if (isalpha(ch) &&
            has(flags, WordDelimiterGraphFilter::SPLIT_ON_NUMERICS)) {
          // start new token on number -> letter change
          newToken = true;
        }
      }
      if (newToken) {
        wordParts.push_back(make_shared<WordPart>(text, wordPartStart, i));
        wordPartStart = i;
      }
    }
    lastCH = ch;
  }

  if (inToken) {
    // add last token
    wordParts.push_back(
        make_shared<WordPart>(text, wordPartStart, text.length()));
  }

  shared_ptr<Set<wstring>> paths = unordered_set<wstring>();
  if (wordParts.empty() == false) {
    enumerate(flags, 0, text, wordParts, paths, make_shared<StringBuilder>());
  }

  if (has(flags, WordDelimiterGraphFilter::PRESERVE_ORIGINAL)) {
    paths->add(text);
  }

  if (has(flags, WordDelimiterGraphFilter::CATENATE_ALL) &&
      wordParts.empty() == false) {
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    for (auto wordPart : wordParts) {
      b->append(wordPart->part);
    }
    paths->add(b->toString());
  }

  return paths;
}

void TestWordDelimiterGraphFilter::add(shared_ptr<StringBuilder> path,
                                       const wstring &part)
{
  if (path->length() != 0) {
    path->append(L' ');
  }
  path->append(part);
}

void TestWordDelimiterGraphFilter::add(
    shared_ptr<StringBuilder> path,
    deque<std::shared_ptr<WordPart>> &wordParts, int from, int to)
{
  if (path->length() != 0) {
    path->append(L' ');
  }
  // no spaces:
  for (int i = from; i < to; i++) {
    path->append(wordParts[i]->part);
  }
}

void TestWordDelimiterGraphFilter::addWithSpaces(
    shared_ptr<StringBuilder> path,
    deque<std::shared_ptr<WordPart>> &wordParts, int from, int to)
{
  for (int i = from; i < to; i++) {
    add(path, wordParts[i]->part);
  }
}

int TestWordDelimiterGraphFilter::endOfRun(
    deque<std::shared_ptr<WordPart>> &wordParts, int start)
{
  int upto = start + 1;
  while (upto < wordParts.size() &&
         wordParts[upto]->type == wordParts[start]->type) {
    upto++;
  }
  return upto;
}

void TestWordDelimiterGraphFilter::enumerate(
    int flags, int upto, const wstring &text,
    deque<std::shared_ptr<WordPart>> &wordParts,
    shared_ptr<Set<wstring>> paths, shared_ptr<StringBuilder> path)
{
  if (upto == wordParts.size()) {
    if (path->length() > 0) {
      paths->add(path->toString());
    }
  } else {
    int savLength = path->length();
    int end = endOfRun(wordParts, upto);

    if (wordParts[upto]->type == NUMBER) {
      // always output single word, optionally surrounded by delims:
      if (has(flags, WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS) ||
          wordParts.size() == 1) {
        addWithSpaces(path, wordParts, upto, end);
        if (has(flags, WordDelimiterGraphFilter::CATENATE_NUMBERS)) {
          // recurse first with the parts
          enumerate(flags, end, text, wordParts, paths, path);
          path->setLength(savLength);
          // .. and second with the concat
          add(path, wordParts, upto, end);
        }
      } else if (has(flags, WordDelimiterGraphFilter::CATENATE_NUMBERS)) {
        add(path, wordParts, upto, end);
      }
      enumerate(flags, end, text, wordParts, paths, path);
      path->setLength(savLength);
    } else {
      assert(wordParts[upto]->type == LETTER);
      // always output single word, optionally surrounded by delims:
      if (has(flags, WordDelimiterGraphFilter::GENERATE_WORD_PARTS) ||
          wordParts.size() == 1) {
        addWithSpaces(path, wordParts, upto, end);
        if (has(flags, WordDelimiterGraphFilter::CATENATE_WORDS)) {
          // recurse first with the parts
          enumerate(flags, end, text, wordParts, paths, path);
          path->setLength(savLength);
          // .. and second with the concat
          add(path, wordParts, upto, end);
        }
      } else if (has(flags, WordDelimiterGraphFilter::CATENATE_WORDS)) {
        add(path, wordParts, upto, end);
      }
      enumerate(flags, end, text, wordParts, paths, path);
      path->setLength(savLength);
    }
  }
}

void TestWordDelimiterGraphFilter::testBasicGraphSplits() 
{
  assertGraphStrings(getAnalyzer(0), {L"PowerShotPlus", {L"PowerShotPlus"}});
  assertGraphStrings(getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS),
                     {L"PowerShotPlus", {L"PowerShotPlus"}});
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE),
      {L"PowerShotPlus", {L"Power Shot Plus"}});
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                  WordDelimiterGraphFilter::PRESERVE_ORIGINAL),
      {L"PowerShotPlus", {L"PowerShotPlus", L"Power Shot Plus"}});

  assertGraphStrings(getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS),
                     {L"Power-Shot-Plus", {L"Power Shot Plus"}});
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE),
      {L"Power-Shot-Plus", {L"Power Shot Plus"}});
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                  WordDelimiterGraphFilter::PRESERVE_ORIGINAL),
      {L"Power-Shot-Plus", {L"Power-Shot-Plus", L"Power Shot Plus"}});

  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE),
      {L"PowerShotPlus", {L"Power Shot Plus"}});
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE),
      {L"PowerShot1000Plus", {L"Power Shot1000Plus"}});
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE),
      {L"Power-Shot-Plus", {L"Power Shot Plus"}});

  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                  WordDelimiterGraphFilter::CATENATE_WORDS),
      {L"PowerShotPlus", {L"Power Shot Plus", L"PowerShotPlus"}});
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                  WordDelimiterGraphFilter::CATENATE_WORDS),
      {L"PowerShot1000Plus", {L"Power Shot1000Plus", L"PowerShot1000Plus"}});
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                  WordDelimiterGraphFilter::CATENATE_WORDS |
                  WordDelimiterGraphFilter::CATENATE_NUMBERS),
      {L"Power-Shot-1000-17-Plus",
       {L"Power Shot 1000 17 Plus", L"Power Shot 100017 Plus",
        L"PowerShot 1000 17 Plus", L"PowerShot 100017 Plus"}});
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                  WordDelimiterGraphFilter::CATENATE_WORDS |
                  WordDelimiterGraphFilter::CATENATE_NUMBERS |
                  WordDelimiterGraphFilter::PRESERVE_ORIGINAL),
      {L"Power-Shot-1000-17-Plus",
       {L"Power-Shot-1000-17-Plus", L"Power Shot 1000 17 Plus",
        L"Power Shot 100017 Plus", L"PowerShot 1000 17 Plus",
        L"PowerShot 100017 Plus"}});
}

wstring TestWordDelimiterGraphFilter::randomWDFText()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  int length = TestUtil::nextInt(random(), 1, 50);
  for (int i = 0; i < length; i++) {
    int surpriseMe = random()->nextInt(37);
    int lower = -1;
    int upper = -1;
    if (surpriseMe < 10) {
      // lowercase letter
      lower = L'a';
      upper = L'z';
    } else if (surpriseMe < 20) {
      // uppercase letter
      lower = L'A';
      upper = L'Z';
    } else if (surpriseMe < 30) {
      // digit
      lower = L'0';
      upper = L'9';
    } else if (surpriseMe < 35) {
      // punct
      lower = L'-';
      upper = L'-';
    } else {
      b->append(L"'s");
    }

    if (lower != -1) {
      b->append(
          static_cast<wchar_t>(TestUtil::nextInt(random(), lower, upper)));
    }
  }

  return b->toString();
}

void TestWordDelimiterGraphFilter::testInvalidFlag() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<WordDelimiterGraphFilter>(make_shared<CannedTokenStream>(),
                                          1 << 31, nullptr);
  });
}

void TestWordDelimiterGraphFilter::testRandomPaths() 
{
  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    wstring text = randomWDFText();
    if (VERBOSE) {
      wcout << L"\nTEST: text=" << text << L" len=" << text.length() << endl;
    }

    int flags = 0;
    if (random()->nextBoolean()) {
      flags |= WordDelimiterGraphFilter::GENERATE_WORD_PARTS;
    }
    if (random()->nextBoolean()) {
      flags |= WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS;
    }
    if (random()->nextBoolean()) {
      flags |= WordDelimiterGraphFilter::CATENATE_WORDS;
    }
    if (random()->nextBoolean()) {
      flags |= WordDelimiterGraphFilter::CATENATE_NUMBERS;
    }
    if (random()->nextBoolean()) {
      flags |= WordDelimiterGraphFilter::CATENATE_ALL;
    }
    if (random()->nextBoolean()) {
      flags |= WordDelimiterGraphFilter::PRESERVE_ORIGINAL;
    }
    if (random()->nextBoolean()) {
      flags |= WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE;
    }
    if (random()->nextBoolean()) {
      flags |= WordDelimiterGraphFilter::SPLIT_ON_NUMERICS;
    }
    if (random()->nextBoolean()) {
      flags |= WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
    }

    verify(text, flags);
  }
}

void TestWordDelimiterGraphFilter::verify(const wstring &text,
                                          int flags) 
{

  shared_ptr<Set<wstring>> expected = slowWDF(text, flags);
  if (VERBOSE) {
    for (auto path : expected) {
      wcout << L"  " << path << endl;
    }
  }

  shared_ptr<Set<wstring>> actual = getGraphStrings(getAnalyzer(flags), text);
  if (actual->equals(expected) == false) {
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    b->append(L"\n\nFAIL: text=");
    b->append(text);
    b->append(L" flags=");
    b->append(WordDelimiterGraphFilter::flagsToString(flags));
    b->append(L'\n');
    b->append(L"  expected paths:\n");
    for (auto s : expected) {
      b->append(L"    ");
      b->append(s);
      if (actual->contains(s) == false) {
        b->append(L" [missing!]");
      }
      b->append(L'\n');
    }

    b->append(L"  actual paths:\n");
    for (auto s : actual) {
      b->append(L"    ");
      b->append(s);
      if (expected->contains(s) == false) {
        b->append(L" [unexpected!]");
      }
      b->append(L'\n');
    }

    fail(b->toString());
  }
}

void TestWordDelimiterGraphFilter::testOnlyNumbers() 
{
  // no token should be produced
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                  WordDelimiterGraphFilter::SPLIT_ON_NUMERICS),
      {L"7-586"});
}

void TestWordDelimiterGraphFilter::testNoCatenate() 
{
  // no token should be produced
  assertGraphStrings(
      getAnalyzer(WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
                  WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
                  WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                  WordDelimiterGraphFilter::SPLIT_ON_NUMERICS),
      {L"a-b-c-9-d", {L"a b c 9 d"}});
}

void TestWordDelimiterGraphFilter::testCuriousCase1() 
{
  verify(L"u-0L-4836-ip4Gw--13--q7--L07E1",
         WordDelimiterGraphFilter::CATENATE_WORDS |
             WordDelimiterGraphFilter::CATENATE_ALL |
             WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
             WordDelimiterGraphFilter::SPLIT_ON_NUMERICS |
             WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE);
}

void TestWordDelimiterGraphFilter::testCuriousCase2() 
{
  verify(L"u-l-p", WordDelimiterGraphFilter::CATENATE_ALL);
}

void TestWordDelimiterGraphFilter::testOriginalPosLength() 
{
  verify(L"Foo-Bar-Baz", WordDelimiterGraphFilter::CATENATE_WORDS |
                             WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
                             WordDelimiterGraphFilter::PRESERVE_ORIGINAL);
}

void TestWordDelimiterGraphFilter::testCuriousCase3() 
{
  verify(L"cQzk4-GL0izl0mKM-J8--4m-'s",
         WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS |
             WordDelimiterGraphFilter::CATENATE_NUMBERS |
             WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE |
             WordDelimiterGraphFilter::SPLIT_ON_NUMERICS);
}

void TestWordDelimiterGraphFilter::testEmptyString() 
{
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(
          make_shared<CannedTokenStream>(make_shared<Token>(L"", 0, 0)),
          DEFAULT_WORD_DELIM_TABLE,
          WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::CATENATE_ALL |
              WordDelimiterGraphFilter::PRESERVE_ORIGINAL,
          nullptr);
  wdf->reset();
  assertTrue(wdf->incrementToken());
  assertFalse(wdf->incrementToken());
  wdf->end();
  delete wdf;
}

void TestWordDelimiterGraphFilter::testProtectedWords() 
{
  shared_ptr<TokenStream> tokens =
      make_shared<CannedTokenStream>(make_shared<Token>(L"foo17-bar", 0, 9),
                                     make_shared<Token>(L"foo-bar", 0, 7));

  shared_ptr<CharArraySet> protectedWords = make_shared<CharArraySet>(
      unordered_set<>(Arrays::asList(L"foo17-BAR")), true);
  shared_ptr<WordDelimiterGraphFilter> wdf =
      make_shared<WordDelimiterGraphFilter>(
          tokens, DEFAULT_WORD_DELIM_TABLE,
          WordDelimiterGraphFilter::GENERATE_WORD_PARTS |
              WordDelimiterGraphFilter::PRESERVE_ORIGINAL |
              WordDelimiterGraphFilter::CATENATE_ALL,
          protectedWords);
  assertGraphStrings(
      wdf, {L"foo17-bar foo bar", {L"foo17-bar foo-bar", L"foo17-bar foobar"}});
}
} // namespace org::apache::lucene::analysis::miscellaneous