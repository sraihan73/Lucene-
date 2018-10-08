using namespace std;

#include "TestWordDelimiterFilter.h"
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
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/WordDelimiterFilter.h"
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
using namespace org::apache::lucene::analysis::miscellaneous;
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterFilter.*; import
//    static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterIterator.DEFAULT_WORD_DELIM_TABLE;

void TestWordDelimiterFilter::testOffsets() 
{
  int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
              WordDelimiterFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterFilter::CATENATE_ALL |
              WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterFilter::SPLIT_ON_NUMERICS |
              WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE;
  // test that subwords and catenated subwords have
  // the correct offsets.
  shared_ptr<WordDelimiterFilter> wdf = make_shared<WordDelimiterFilter>(
      make_shared<CannedTokenStream>(make_shared<Token>(L"foo-bar", 5, 12)),
      DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(
      wdf, std::deque<wstring>{L"foo", L"foobar", L"bar"},
      std::deque<int>{5, 5, 9}, std::deque<int>{8, 12, 12});

  wdf = make_shared<WordDelimiterFilter>(
      make_shared<CannedTokenStream>(make_shared<Token>(L"foo-bar", 5, 6)),
      DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(
      wdf, std::deque<wstring>{L"foo", L"bar", L"foobar"},
      std::deque<int>{5, 5, 5}, std::deque<int>{6, 6, 6});
}

void TestWordDelimiterFilter::testOffsetChange() 
{
  int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
              WordDelimiterFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterFilter::CATENATE_ALL |
              WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterFilter::SPLIT_ON_NUMERICS |
              WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterFilter> wdf = make_shared<WordDelimiterFilter>(
      make_shared<CannedTokenStream>(make_shared<Token>(L"übelkeit)", 7, 16)),
      DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(wdf, std::deque<wstring>{L"übelkeit"},
                            std::deque<int>{7}, std::deque<int>{15});
}

void TestWordDelimiterFilter::testOffsetChange2() 
{
  int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
              WordDelimiterFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterFilter::CATENATE_ALL |
              WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterFilter::SPLIT_ON_NUMERICS |
              WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterFilter> wdf = make_shared<WordDelimiterFilter>(
      make_shared<CannedTokenStream>(make_shared<Token>(L"(übelkeit", 7, 17)),
      DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(wdf, std::deque<wstring>{L"übelkeit"},
                            std::deque<int>{8}, std::deque<int>{17});
}

void TestWordDelimiterFilter::testOffsetChange3() 
{
  int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
              WordDelimiterFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterFilter::CATENATE_ALL |
              WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterFilter::SPLIT_ON_NUMERICS |
              WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterFilter> wdf = make_shared<WordDelimiterFilter>(
      make_shared<CannedTokenStream>(make_shared<Token>(L"(übelkeit", 7, 16)),
      DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(wdf, std::deque<wstring>{L"übelkeit"},
                            std::deque<int>{8}, std::deque<int>{16});
}

void TestWordDelimiterFilter::testOffsetChange4() 
{
  int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
              WordDelimiterFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterFilter::CATENATE_ALL |
              WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterFilter::SPLIT_ON_NUMERICS |
              WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterFilter> wdf = make_shared<WordDelimiterFilter>(
      make_shared<CannedTokenStream>(make_shared<Token>(L"(foo,bar)", 7, 16)),
      DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(
      wdf, std::deque<wstring>{L"foo", L"foobar", L"bar"},
      std::deque<int>{8, 8, 12}, std::deque<int>{11, 15, 15});
}

void TestWordDelimiterFilter::doSplit(
    const wstring &input, deque<wstring> &output) 
{
  int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
              WordDelimiterFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterFilter::SPLIT_ON_NUMERICS |
              WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<WordDelimiterFilter> wdf = make_shared<WordDelimiterFilter>(
      keywordMockTokenizer(input),
      WordDelimiterIterator::DEFAULT_WORD_DELIM_TABLE, flags, nullptr);

  assertTokenStreamContents(wdf, output);
}

void TestWordDelimiterFilter::testSplits() 
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

void TestWordDelimiterFilter::doSplitPossessive(
    int stemPossessive, const wstring &input,
    deque<wstring> &output) 
{
  int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
              WordDelimiterFilter::GENERATE_NUMBER_PARTS |
              WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterFilter::SPLIT_ON_NUMERICS;
  flags |=
      (stemPossessive == 1) ? WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE : 0;
  shared_ptr<WordDelimiterFilter> wdf = make_shared<WordDelimiterFilter>(
      keywordMockTokenizer(input), flags, nullptr);

  assertTokenStreamContents(wdf, output);
}

void TestWordDelimiterFilter::testPossessives() 
{
  doSplitPossessive(1, L"ra's", {L"ra"});
  doSplitPossessive(0, L"ra's", {L"ra", L"s"});
}

TestWordDelimiterFilter::LargePosIncTokenFilter::LargePosIncTokenFilter(
    shared_ptr<TokenStream> input)
    : TokenFilter(input)
{
}

bool TestWordDelimiterFilter::LargePosIncTokenFilter::incrementToken() throw(
    IOException)
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

void TestWordDelimiterFilter::testPositionIncrements() 
{
  constexpr int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
                        WordDelimiterFilter::GENERATE_NUMBER_PARTS |
                        WordDelimiterFilter::CATENATE_ALL |
                        WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
                        WordDelimiterFilter::SPLIT_ON_NUMERICS |
                        WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE;
  shared_ptr<CharArraySet> *const protWords = make_shared<CharArraySet>(
      unordered_set<>(Arrays::asList(L"NUTCH")), false);

  /* analyzer that uses whitespace + wdf */
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), flags, protWords);

  /* in this case, works as expected. */
  assertAnalyzesTo(a, L"LUCENE / SOLR",
                   std::deque<wstring>{L"LUCENE", L"SOLR"},
                   std::deque<int>{0, 9}, std::deque<int>{6, 13}, nullptr,
                   std::deque<int>{1, 1}, nullptr, false);

  /* only in this case, posInc of 2 ?! */
  assertAnalyzesTo(a, L"LUCENE / solR",
                   std::deque<wstring>{L"LUCENE", L"sol", L"solR", L"R"},
                   std::deque<int>{0, 9, 9, 12},
                   std::deque<int>{6, 12, 13, 13}, nullptr,
                   std::deque<int>{1, 1, 0, 1}, nullptr, false);

  assertAnalyzesTo(a, L"LUCENE / NUTCH SOLR",
                   std::deque<wstring>{L"LUCENE", L"NUTCH", L"SOLR"},
                   std::deque<int>{0, 9, 15}, std::deque<int>{6, 14, 19},
                   nullptr, std::deque<int>{1, 1, 1}, nullptr, false);

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
                   std::deque<wstring>{L"LUCENE", L"sol", L"solR", L"R"},
                   std::deque<int>{0, 9, 9, 12},
                   std::deque<int>{6, 12, 13, 13}, nullptr,
                   std::deque<int>{1, 11, 0, 1}, nullptr, false);

  assertAnalyzesTo(a2, L"LUCENE / NUTCH SOLR",
                   std::deque<wstring>{L"LUCENE", L"NUTCH", L"SOLR"},
                   std::deque<int>{0, 9, 15}, std::deque<int>{6, 14, 19},
                   nullptr, std::deque<int>{1, 11, 1}, nullptr, false);

  shared_ptr<Analyzer> a3 = make_shared<AnalyzerAnonymousInnerClass3>(
      shared_from_this(), flags, protWords);

  assertAnalyzesTo(a3, L"lucene.solr",
                   std::deque<wstring>{L"lucene", L"lucenesolr", L"solr"},
                   std::deque<int>{0, 0, 7}, std::deque<int>{6, 11, 11},
                   nullptr, std::deque<int>{1, 0, 1}, nullptr, false);

  /* the stopword should add a gap here */
  assertAnalyzesTo(a3, L"the lucene.solr",
                   std::deque<wstring>{L"lucene", L"lucenesolr", L"solr"},
                   std::deque<int>{4, 4, 11}, std::deque<int>{10, 15, 15},
                   nullptr, std::deque<int>{2, 0, 1}, nullptr, false);

  IOUtils::close({a, a2, a3});
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protWords = protWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<WordDelimiterFilter>(tokenizer, flags, protWords));
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protWords = protWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterFilter>(
          make_shared<LargePosIncTokenFilter>(tokenizer), flags, protWords));
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protWords = protWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<StopFilter> filter =
      make_shared<StopFilter>(tokenizer, StandardAnalyzer::STOP_WORDS_SET);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<WordDelimiterFilter>(filter, flags, protWords));
}

void TestWordDelimiterFilter::testKeywordFilter() 
{
  assertAnalyzesTo(
      keywordTestAnalyzer(WordDelimiterFilter::GENERATE_WORD_PARTS),
      L"abc-def klm-nop kpop",
      std::deque<wstring>{L"abc", L"def", L"klm", L"nop", L"kpop"});
  assertAnalyzesTo(
      keywordTestAnalyzer(WordDelimiterFilter::GENERATE_WORD_PARTS |
                          WordDelimiterFilter::IGNORE_KEYWORDS),
      L"abc-def klm-nop kpop",
      std::deque<wstring>{L"abc", L"def", L"klm-nop", L"kpop"},
      std::deque<int>{0, 4, 8, 16}, std::deque<int>{3, 7, 15, 20}, nullptr,
      std::deque<int>{1, 1, 1, 1}, nullptr, false);
}

shared_ptr<Analyzer>
TestWordDelimiterFilter::keywordTestAnalyzer(int flags) 
{
  return make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this(), flags);
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<KeywordMarkerFilter> kFilter =
      make_shared<KeywordMarkerFilterAnonymousInnerClass>(shared_from_this(),
                                                          tokenizer);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<WordDelimiterFilter>(kFilter, flags, nullptr));
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass4::
    KeywordMarkerFilterAnonymousInnerClass::
        KeywordMarkerFilterAnonymousInnerClass(
            shared_ptr<AnalyzerAnonymousInnerClass4> outerInstance,
            shared_ptr<org::apache::lucene::analysis::Tokenizer> tokenizer)
    : KeywordMarkerFilter(tokenizer)
{
  this->outerInstance = outerInstance;
  term = addAttribute(CharTermAttribute::typeid);
}

bool TestWordDelimiterFilter::AnalyzerAnonymousInnerClass4::
    KeywordMarkerFilterAnonymousInnerClass::isKeyword()
{
  // Marks terms starting with the letter 'k' as keywords
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return term->toString()->charAt(0) == L'k';
}

void TestWordDelimiterFilter::testLotsOfConcatenating() 
{
  constexpr int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
                        WordDelimiterFilter::GENERATE_NUMBER_PARTS |
                        WordDelimiterFilter::CATENATE_WORDS |
                        WordDelimiterFilter::CATENATE_NUMBERS |
                        WordDelimiterFilter::CATENATE_ALL |
                        WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
                        WordDelimiterFilter::SPLIT_ON_NUMERICS |
                        WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE;

  /* analyzer that uses whitespace + wdf */
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this(), flags);

  assertAnalyzesTo(a, L"abc-def-123-456",
                   std::deque<wstring>{L"abc", L"abcdef", L"abcdef123456",
                                        L"def", L"123", L"123456", L"456"},
                   std::deque<int>{0, 0, 0, 4, 8, 8, 12},
                   std::deque<int>{3, 7, 15, 7, 11, 15, 15}, nullptr,
                   std::deque<int>{1, 0, 0, 1, 1, 0, 1}, nullptr, false);
  delete a;
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass5::
    AnalyzerAnonymousInnerClass5(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<WordDelimiterFilter>(tokenizer, flags, nullptr));
}

void TestWordDelimiterFilter::testLotsOfConcatenating2() 
{
  constexpr int flags = WordDelimiterFilter::PRESERVE_ORIGINAL |
                        WordDelimiterFilter::GENERATE_WORD_PARTS |
                        WordDelimiterFilter::GENERATE_NUMBER_PARTS |
                        WordDelimiterFilter::CATENATE_WORDS |
                        WordDelimiterFilter::CATENATE_NUMBERS |
                        WordDelimiterFilter::CATENATE_ALL |
                        WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
                        WordDelimiterFilter::SPLIT_ON_NUMERICS |
                        WordDelimiterFilter::STEM_ENGLISH_POSSESSIVE;

  /* analyzer that uses whitespace + wdf */
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass6>(shared_from_this(), flags);

  assertAnalyzesTo(a, L"abc-def-123-456",
                   std::deque<wstring>{L"abc-def-123-456", L"abc", L"abcdef",
                                        L"abcdef123456", L"def", L"123",
                                        L"123456", L"456"},
                   std::deque<int>{0, 0, 0, 0, 4, 8, 8, 12},
                   std::deque<int>{15, 3, 7, 15, 7, 11, 15, 15}, nullptr,
                   std::deque<int>{1, 0, 0, 0, 1, 1, 0, 1}, nullptr, false);
  delete a;
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass6::
    AnalyzerAnonymousInnerClass6(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass6::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<WordDelimiterFilter>(tokenizer, flags, nullptr));
}

void TestWordDelimiterFilter::testRandomStrings() 
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

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass7::
    AnalyzerAnonymousInnerClass7(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protectedWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protectedWords = protectedWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass7::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterFilter>(tokenizer, flags, protectedWords));
}

void TestWordDelimiterFilter::testRandomHugeStrings() 
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

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass8::
    AnalyzerAnonymousInnerClass8(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protectedWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protectedWords = protectedWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass8::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterFilter>(tokenizer, flags, protectedWords));
}

void TestWordDelimiterFilter::testEmptyTerm() 
{
  shared_ptr<Random> random = TestWordDelimiterFilter::random();
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

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass9::
    AnalyzerAnonymousInnerClass9(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        shared_ptr<CharArraySet> protectedWords)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
  this->protectedWords = protectedWords;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass9::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<TokenStreamComponents>(
      tokenizer,
      make_shared<WordDelimiterFilter>(tokenizer, flags, protectedWords));
}

void TestWordDelimiterFilter::testOnlyNumbers() 
{
  int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
              WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterFilter::SPLIT_ON_NUMERICS;
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass10>(shared_from_this(), flags);

  assertAnalyzesTo(a, L"7-586", std::deque<wstring>(), std::deque<int>(),
                   std::deque<int>(), nullptr, std::deque<int>(), nullptr,
                   false);
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass10::
    AnalyzerAnonymousInnerClass10(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass10::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<WordDelimiterFilter>(tokenizer, flags, nullptr));
}

void TestWordDelimiterFilter::testNumberPunct() 
{
  int flags = WordDelimiterFilter::GENERATE_WORD_PARTS |
              WordDelimiterFilter::SPLIT_ON_CASE_CHANGE |
              WordDelimiterFilter::SPLIT_ON_NUMERICS;
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass11>(shared_from_this(), flags);

  assertAnalyzesTo(a, L"6-", std::deque<wstring>{L"6"}, std::deque<int>{0},
                   std::deque<int>{1}, nullptr, std::deque<int>{1}, nullptr,
                   false);
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass11::
    AnalyzerAnonymousInnerClass11(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass11::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<WordDelimiterFilter>(tokenizer, flags, nullptr));
}

shared_ptr<Analyzer> TestWordDelimiterFilter::getAnalyzer(int const flags)
{
  return make_shared<AnalyzerAnonymousInnerClass12>(shared_from_this(), flags);
}

TestWordDelimiterFilter::AnalyzerAnonymousInnerClass12::
    AnalyzerAnonymousInnerClass12(
        shared_ptr<TestWordDelimiterFilter> outerInstance, int flags)
{
  this->outerInstance = outerInstance;
  this->flags = flags;
}

shared_ptr<TokenStreamComponents>
TestWordDelimiterFilter::AnalyzerAnonymousInnerClass12::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<WordDelimiterFilter>(tokenizer, flags, nullptr));
}
} // namespace org::apache::lucene::analysis::miscellaneous