using namespace std;

#include "TestCompoundWordTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include "../../../../../../java/org/apache/lucene/analysis/compound/CompoundWordTokenFilterBase.h"
#include "../../../../../../java/org/apache/lucene/analysis/compound/DictionaryCompoundWordTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/compound/HyphenationCompoundWordTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/compound/hyphenation/HyphenationTree.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"

namespace org::apache::lucene::analysis::compound
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using MappingCharFilter =
    org::apache::lucene::analysis::charfilter::MappingCharFilter;
using NormalizeCharMap =
    org::apache::lucene::analysis::charfilter::NormalizeCharMap;
using HyphenationTree =
    org::apache::lucene::analysis::compound::hyphenation::HyphenationTree;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using org::xml::sax::InputSource;

shared_ptr<CharArraySet>
TestCompoundWordTokenFilter::makeDictionary(deque<wstring> &dictionary)
{
  return make_shared<CharArraySet>(Arrays::asList(dictionary), true);
}

void TestCompoundWordTokenFilter::testHyphenationCompoundWordsDA() throw(
    runtime_error)
{
  shared_ptr<CharArraySet> dict = makeDictionary({L"læse", L"hest"});

  shared_ptr<InputSource> is = make_shared<InputSource>(
      getClass().getResource(L"da_UTF8.xml").toExternalForm());
  shared_ptr<HyphenationTree> hyphenator =
      HyphenationCompoundWordTokenFilter::getHyphenationTree(is);

  shared_ptr<HyphenationCompoundWordTokenFilter> tf =
      make_shared<HyphenationCompoundWordTokenFilter>(
          whitespaceMockTokenizer(L"min veninde som er lidt af en læsehest"),
          hyphenator, dict, CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MIN_SUBWORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MAX_SUBWORD_SIZE, false);
  assertTokenStreamContents(tf,
                            std::deque<wstring>{L"min", L"veninde", L"som",
                                                 L"er", L"lidt", L"af", L"en",
                                                 L"læsehest", L"læse", L"hest"},
                            std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 0, 0});
}

void TestCompoundWordTokenFilter::
    testHyphenationCompoundWordsDELongestMatch() 
{
  shared_ptr<CharArraySet> dict =
      makeDictionary({L"basketball", L"basket", L"ball", L"kurv"});

  shared_ptr<InputSource> is = make_shared<InputSource>(
      getClass().getResource(L"da_UTF8.xml").toExternalForm());
  shared_ptr<HyphenationTree> hyphenator =
      HyphenationCompoundWordTokenFilter::getHyphenationTree(is);

  // the word basket will not be added due to the longest match option
  shared_ptr<HyphenationCompoundWordTokenFilter> tf =
      make_shared<HyphenationCompoundWordTokenFilter>(
          whitespaceMockTokenizer(L"basketballkurv"), hyphenator, dict,
          CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MIN_SUBWORD_SIZE, 40, true);
  assertTokenStreamContents(
      tf,
      std::deque<wstring>{L"basketballkurv", L"basketball", L"ball", L"kurv"},
      std::deque<int>{1, 0, 0, 0});
}

void TestCompoundWordTokenFilter::testHyphenationOnly() 
{
  shared_ptr<InputSource> is = make_shared<InputSource>(
      getClass().getResource(L"da_UTF8.xml").toExternalForm());
  shared_ptr<HyphenationTree> hyphenator =
      HyphenationCompoundWordTokenFilter::getHyphenationTree(is);

  shared_ptr<HyphenationCompoundWordTokenFilter> tf =
      make_shared<HyphenationCompoundWordTokenFilter>(
          whitespaceMockTokenizer(L"basketballkurv"), hyphenator,
          CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE, 2, 4);

  // min=2, max=4
  assertTokenStreamContents(tf, std::deque<wstring>{L"basketballkurv", L"ba",
                                                     L"sket", L"bal", L"ball",
                                                     L"kurv"});

  tf = make_shared<HyphenationCompoundWordTokenFilter>(
      whitespaceMockTokenizer(L"basketballkurv"), hyphenator,
      CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE, 4, 6);

  // min=4, max=6
  assertTokenStreamContents(
      tf, std::deque<wstring>{L"basketballkurv", L"basket", L"sket", L"ball",
                               L"lkurv", L"kurv"});

  tf = make_shared<HyphenationCompoundWordTokenFilter>(
      whitespaceMockTokenizer(L"basketballkurv"), hyphenator,
      CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE, 4, 10);

  // min=4, max=10
  assertTokenStreamContents(
      tf, std::deque<wstring>{L"basketballkurv", L"basket", L"basketbal",
                               L"basketball", L"sket", L"sketbal", L"sketball",
                               L"ball", L"ballkurv", L"lkurv", L"kurv"});
}

void TestCompoundWordTokenFilter::testDumbCompoundWordsSE() 
{
  shared_ptr<CharArraySet> dict = makeDictionary(
      {L"Bil",    L"Dörr", L"Motor", L"Tak",    L"Borr",    L"Slag", L"Hammar",
       L"Pelar",  L"Glas", L"Ögon",  L"Fodral", L"Bas",     L"Fiol", L"Makare",
       L"Gesäll", L"Sko",  L"Vind",  L"Rute",   L"Torkare", L"Blad"});

  shared_ptr<DictionaryCompoundWordTokenFilter> tf =
      make_shared<DictionaryCompoundWordTokenFilter>(
          whitespaceMockTokenizer(
              L"Bildörr Bilmotor Biltak Slagborr Hammarborr Pelarborr "
              L"Glasögonfodral Basfiolsfodral Basfiolsfodralmakaregesäll "
              L"Skomakare Vindrutetorkare Vindrutetorkarblad abba"),
          dict);

  assertTokenStreamContents(
      tf,
      std::deque<wstring>{L"Bildörr",
                           L"Bil",
                           L"dörr",
                           L"Bilmotor",
                           L"Bil",
                           L"motor",
                           L"Biltak",
                           L"Bil",
                           L"tak",
                           L"Slagborr",
                           L"Slag",
                           L"borr",
                           L"Hammarborr",
                           L"Hammar",
                           L"borr",
                           L"Pelarborr",
                           L"Pelar",
                           L"borr",
                           L"Glasögonfodral",
                           L"Glas",
                           L"ögon",
                           L"fodral",
                           L"Basfiolsfodral",
                           L"Bas",
                           L"fiol",
                           L"fodral",
                           L"Basfiolsfodralmakaregesäll",
                           L"Bas",
                           L"fiol",
                           L"fodral",
                           L"makare",
                           L"gesäll",
                           L"Skomakare",
                           L"Sko",
                           L"makare",
                           L"Vindrutetorkare",
                           L"Vind",
                           L"rute",
                           L"torkare",
                           L"Vindrutetorkarblad",
                           L"Vind",
                           L"rute",
                           L"blad",
                           L"abba"},
      std::deque<int>{0,   0,   0,   8,   8,   8,   17,  17,  17,  24,  24,
                       24,  33,  33,  33,  44,  44,  44,  54,  54,  54,  54,
                       69,  69,  69,  69,  84,  84,  84,  84,  84,  84,  111,
                       111, 111, 121, 121, 121, 121, 137, 137, 137, 137, 156},
      std::deque<int>{7,   7,   7,   16,  16,  16,  23,  23,  23,  32,  32,
                       32,  43,  43,  43,  53,  53,  53,  68,  68,  68,  68,
                       83,  83,  83,  83,  110, 110, 110, 110, 110, 110, 120,
                       120, 120, 136, 136, 136, 136, 155, 155, 155, 155, 160},
      std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
                       1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
                       0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1});
}

void TestCompoundWordTokenFilter::testDumbCompoundWordsSELongestMatch() throw(
    runtime_error)
{
  shared_ptr<CharArraySet> dict = makeDictionary(
      {L"Bil",     L"Dörr",   L"Motor",      L"Tak",  L"Borr",   L"Slag",
       L"Hammar",  L"Pelar",  L"Glas",       L"Ögon", L"Fodral", L"Bas",
       L"Fiols",   L"Makare", L"Gesäll",     L"Sko",  L"Vind",   L"Rute",
       L"Torkare", L"Blad",   L"Fiolsfodral"});

  shared_ptr<DictionaryCompoundWordTokenFilter> tf =
      make_shared<DictionaryCompoundWordTokenFilter>(
          whitespaceMockTokenizer(L"Basfiolsfodralmakaregesäll"), dict,
          CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MIN_SUBWORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MAX_SUBWORD_SIZE, true);

  assertTokenStreamContents(
      tf,
      std::deque<wstring>{L"Basfiolsfodralmakaregesäll", L"Bas",
                           L"fiolsfodral", L"fodral", L"makare", L"gesäll"},
      std::deque<int>{0, 0, 0, 0, 0, 0},
      std::deque<int>{26, 26, 26, 26, 26, 26},
      std::deque<int>{1, 0, 0, 0, 0, 0});
}

void TestCompoundWordTokenFilter::
    testTokenEndingWithWordComponentOfMinimumLength() 
{
  shared_ptr<CharArraySet> dict = makeDictionary({L"ab", L"cd", L"ef"});

  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(make_shared<StringReader>(L"abcdef"));
  shared_ptr<DictionaryCompoundWordTokenFilter> tf =
      make_shared<DictionaryCompoundWordTokenFilter>(
          tokenizer, dict, CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MIN_SUBWORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MAX_SUBWORD_SIZE, false);

  assertTokenStreamContents(
      tf, std::deque<wstring>{L"abcdef", L"ab", L"cd", L"ef"},
      std::deque<int>{0, 0, 0, 0}, std::deque<int>{6, 6, 6, 6},
      std::deque<int>{1, 0, 0, 0});
}

void TestCompoundWordTokenFilter::
    testWordComponentWithLessThanMinimumLength() 
{
  shared_ptr<CharArraySet> dict = makeDictionary({L"abc", L"d", L"efg"});

  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(make_shared<StringReader>(L"abcdefg"));
  shared_ptr<DictionaryCompoundWordTokenFilter> tf =
      make_shared<DictionaryCompoundWordTokenFilter>(
          tokenizer, dict, CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MIN_SUBWORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MAX_SUBWORD_SIZE, false);

  // since "d" is shorter than the minimum subword size, it should not be added
  // to the token stream
  assertTokenStreamContents(
      tf, std::deque<wstring>{L"abcdefg", L"abc", L"efg"},
      std::deque<int>{0, 0, 0}, std::deque<int>{7, 7, 7},
      std::deque<int>{1, 0, 0});
}

void TestCompoundWordTokenFilter::testReset() 
{
  shared_ptr<CharArraySet> dict =
      makeDictionary({L"Rind", L"Fleisch", L"Draht", L"Schere", L"Gesetz",
                      L"Aufgabe", L"Überwachung"});

  shared_ptr<MockTokenizer> wsTokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  wsTokenizer->setEnableChecks(false); // we will reset in a strange place
  wsTokenizer->setReader(
      make_shared<StringReader>(L"Rindfleischüberwachungsgesetz"));
  shared_ptr<DictionaryCompoundWordTokenFilter> tf =
      make_shared<DictionaryCompoundWordTokenFilter>(
          wsTokenizer, dict, CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MIN_SUBWORD_SIZE,
          CompoundWordTokenFilterBase::DEFAULT_MAX_SUBWORD_SIZE, false);

  shared_ptr<CharTermAttribute> termAtt =
      tf->getAttribute(CharTermAttribute::typeid);
  tf->reset();
  assertTrue(tf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Rindfleischüberwachungsgesetz", termAtt->toString());
  assertTrue(tf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Rind", termAtt->toString());
  tf->end();
  delete tf;
  wsTokenizer->setReader(
      make_shared<StringReader>(L"Rindfleischüberwachungsgesetz"));
  tf->reset();
  assertTrue(tf->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"Rindfleischüberwachungsgesetz", termAtt->toString());
}

void TestCompoundWordTokenFilter::testRetainMockAttribute() 
{
  shared_ptr<CharArraySet> dict = makeDictionary({L"abc", L"d", L"efg"});
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(make_shared<StringReader>(L"abcdefg"));
  shared_ptr<TokenStream> stream =
      make_shared<MockRetainAttributeFilter>(tokenizer);
  stream = make_shared<DictionaryCompoundWordTokenFilter>(
      stream, dict, CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE,
      CompoundWordTokenFilterBase::DEFAULT_MIN_SUBWORD_SIZE,
      CompoundWordTokenFilterBase::DEFAULT_MAX_SUBWORD_SIZE, false);
  shared_ptr<MockRetainAttribute> retAtt =
      stream->addAttribute(MockRetainAttribute::typeid);
  stream->reset();
  while (stream->incrementToken()) {
    assertTrue(L"Custom attribute value was lost", retAtt->getRetain());
  }
}

void TestCompoundWordTokenFilter::testLucene8124() 
{
  shared_ptr<InputSource> is = make_shared<InputSource>(
      getClass().getResource(L"hyphenation-LUCENE-8124.xml").toExternalForm());
  shared_ptr<HyphenationTree> hyphenator =
      HyphenationCompoundWordTokenFilter::getHyphenationTree(is);

  shared_ptr<HyphenationCompoundWordTokenFilter> tf =
      make_shared<HyphenationCompoundWordTokenFilter>(
          whitespaceMockTokenizer(L"Rindfleisch"), hyphenator);

  // TODO Rindfleisch returned twice is another issue of the
  // HyphenationCompoundTokenFilter
  assertTokenStreamContents(tf,
                            std::deque<wstring>{L"Rindfleisch", L"Rind",
                                                 L"Rindfleisch", L"fleisch"});
}

void TestCompoundWordTokenFilter::MockRetainAttributeImpl::clear()
{
  retain = false;
}

bool TestCompoundWordTokenFilter::MockRetainAttributeImpl::getRetain()
{
  return retain;
}

void TestCompoundWordTokenFilter::MockRetainAttributeImpl::setRetain(
    bool retain)
{
  this->retain = retain;
}

void TestCompoundWordTokenFilter::MockRetainAttributeImpl::copyTo(
    shared_ptr<AttributeImpl> target)
{
  shared_ptr<MockRetainAttribute> t =
      std::static_pointer_cast<MockRetainAttribute>(target);
  t->setRetain(retain);
}

void TestCompoundWordTokenFilter::MockRetainAttributeImpl::reflectWith(
    AttributeReflector reflector)
{
  reflector(MockRetainAttribute::typeid, L"retain", retain);
}

TestCompoundWordTokenFilter::MockRetainAttributeFilter::
    MockRetainAttributeFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TestCompoundWordTokenFilter::MockRetainAttributeFilter::
    incrementToken() 
{
  if (input->incrementToken()) {
    retainAtt->setRetain(true);
    return true;
  } else {
    return false;
  }
}

void TestCompoundWordTokenFilter::testInvalidOffsets() 
{
  shared_ptr<CharArraySet> *const dict = makeDictionary({L"fall"});
  shared_ptr<NormalizeCharMap::Builder> *const builder =
      make_shared<NormalizeCharMap::Builder>();
  builder->add(L"ü", L"ue");
  shared_ptr<NormalizeCharMap> *const normMap = builder->build();

  shared_ptr<Analyzer> analyzer = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), dict, normMap);

  assertAnalyzesTo(analyzer, L"banküberfall",
                   std::deque<wstring>{L"bankueberfall", L"fall"},
                   std::deque<int>{0, 0}, std::deque<int>{12, 12});
  delete analyzer;
}

TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        shared_ptr<CharArraySet> dict, shared_ptr<NormalizeCharMap> normMap)
{
  this->outerInstance = outerInstance;
  this->dict = dict;
  this->normMap = normMap;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenFilter> filter =
      make_shared<DictionaryCompoundWordTokenFilter>(tokenizer, dict);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

shared_ptr<Reader>
TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<MappingCharFilter>(normMap, reader);
}

void TestCompoundWordTokenFilter::testRandomStrings() 
{
  shared_ptr<CharArraySet> *const dict =
      makeDictionary({L"a", L"e", L"i", L"o", L"u", L"y", L"bc", L"def"});
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), dict);
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;

  shared_ptr<InputSource> is = make_shared<InputSource>(
      getClass().getResource(L"da_UTF8.xml").toExternalForm());
  shared_ptr<HyphenationTree> *const hyphenator =
      HyphenationCompoundWordTokenFilter::getHyphenationTree(is);
  shared_ptr<Analyzer> b =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this(), hyphenator);
  checkRandomData(random(), b, 1000 * RANDOM_MULTIPLIER);
  delete b;
}

TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        shared_ptr<CharArraySet> dict)
{
  this->outerInstance = outerInstance;
  this->dict = dict;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<DictionaryCompoundWordTokenFilter>(tokenizer, dict));
}

TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        shared_ptr<HyphenationTree> hyphenator)
{
  this->outerInstance = outerInstance;
  this->hyphenator = hyphenator;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenFilter> filter =
      make_shared<HyphenationCompoundWordTokenFilter>(tokenizer, hyphenator);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

void TestCompoundWordTokenFilter::testEmptyTerm() 
{
  shared_ptr<CharArraySet> *const dict =
      makeDictionary({L"a", L"e", L"i", L"o", L"u", L"y", L"bc", L"def"});
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this(), dict);
  checkOneTerm(a, L"", L"");
  delete a;

  shared_ptr<InputSource> is = make_shared<InputSource>(
      getClass().getResource(L"da_UTF8.xml").toExternalForm());
  shared_ptr<HyphenationTree> *const hyphenator =
      HyphenationCompoundWordTokenFilter::getHyphenationTree(is);
  shared_ptr<Analyzer> b =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this(), hyphenator);
  checkOneTerm(b, L"", L"");
  delete b;
}

TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        shared_ptr<CharArraySet> dict)
{
  this->outerInstance = outerInstance;
  this->dict = dict;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<DictionaryCompoundWordTokenFilter>(tokenizer, dict));
}

TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass5::
    AnalyzerAnonymousInnerClass5(
        shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        shared_ptr<HyphenationTree> hyphenator)
{
  this->outerInstance = outerInstance;
  this->hyphenator = hyphenator;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCompoundWordTokenFilter::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  shared_ptr<TokenFilter> filter =
      make_shared<HyphenationCompoundWordTokenFilter>(tokenizer, hyphenator);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}
} // namespace org::apache::lucene::analysis::compound