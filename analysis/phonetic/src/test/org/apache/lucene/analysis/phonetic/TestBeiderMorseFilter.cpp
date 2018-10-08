using namespace std;

#include "TestBeiderMorseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/miscellaneous/PatternKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/BeiderMorseFilter.h"

namespace org::apache::lucene::analysis::phonetic
{
using org::apache::commons::codec::language::bm::NameType;
using org::apache::commons::codec::language::bm::PhoneticEngine;
using org::apache::commons::codec::language::bm::RuleType;
using org::apache::commons::codec::language::bm::Languages::LanguageSet;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using PatternKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::PatternKeywordMarkerFilter;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

void TestBeiderMorseFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestBeiderMorseFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestBeiderMorseFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBeiderMorseFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<BeiderMorseFilter>(
                     tokenizer, make_shared<PhoneticEngine>(
                                    NameType::GENERIC, RuleType::EXACT, true)));
}

void TestBeiderMorseFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestBeiderMorseFilter::testBasicUsage() 
{
  assertAnalyzesTo(analyzer, L"Angelo",
                   std::deque<wstring>{L"anZelo", L"andZelo", L"angelo",
                                        L"anhelo", L"anjelo", L"anxelo"},
                   std::deque<int>{0, 0, 0, 0, 0, 0},
                   std::deque<int>{6, 6, 6, 6, 6, 6},
                   std::deque<int>{1, 0, 0, 0, 0, 0});

  assertAnalyzesTo(analyzer, L"D'Angelo",
                   std::deque<wstring>{L"anZelo", L"andZelo", L"angelo",
                                        L"anhelo", L"anjelo", L"anxelo",
                                        L"danZelo", L"dandZelo", L"dangelo",
                                        L"danhelo", L"danjelo", L"danxelo"},
                   std::deque<int>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                   std::deque<int>{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
                   std::deque<int>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
}

void TestBeiderMorseFilter::testLanguageSet() 
{
  shared_ptr<LanguageSet> *const languages = LanguageSet::from(
      make_shared<HashSetAnonymousInnerClass>(shared_from_this()));
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this(), languages);
  assertAnalyzesTo(analyzer, L"Angelo",
                   std::deque<wstring>{L"andZelo", L"angelo", L"anxelo"},
                   std::deque<int>{0, 0, 0}, std::deque<int>{6, 6, 6},
                   std::deque<int>{1, 0, 0});
  delete analyzer;
}

TestBeiderMorseFilter::HashSetAnonymousInnerClass::HashSetAnonymousInnerClass(
    shared_ptr<TestBeiderMorseFilter> outerInstance)
{
  this->outerInstance = outerInstance;

  this->add(L"italian");
  this->add(L"greek");
  this->add(L"spanish");
}

TestBeiderMorseFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestBeiderMorseFilter> outerInstance,
        shared_ptr<LanguageSet> languages)
{
  this->outerInstance = outerInstance;
  this->languages = languages;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBeiderMorseFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<BeiderMorseFilter>(
          tokenizer,
          make_shared<PhoneticEngine>(NameType::GENERIC, RuleType::EXACT, true),
          languages));
}

void TestBeiderMorseFilter::testNumbers() 
{
  assertAnalyzesTo(analyzer, L"1234", std::deque<wstring>{L"1234"},
                   std::deque<int>{0}, std::deque<int>{4},
                   std::deque<int>{1});
}

void TestBeiderMorseFilter::testRandom() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestBeiderMorseFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestBeiderMorseFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestBeiderMorseFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBeiderMorseFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<BeiderMorseFilter>(
                     tokenizer, make_shared<PhoneticEngine>(
                                    NameType::GENERIC, RuleType::EXACT, true)));
}

void TestBeiderMorseFilter::testCustomAttribute() 
{
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  (std::static_pointer_cast<Tokenizer>(stream))
      ->setReader(make_shared<StringReader>(L"D'Angelo"));
  stream =
      make_shared<PatternKeywordMarkerFilter>(stream, Pattern::compile(L".*"));
  stream = make_shared<BeiderMorseFilter>(
      stream,
      make_shared<PhoneticEngine>(NameType::GENERIC, RuleType::EXACT, true));
  shared_ptr<KeywordAttribute> keyAtt =
      stream->addAttribute(KeywordAttribute::typeid);
  stream->reset();
  int i = 0;
  while (stream->incrementToken()) {
    assertTrue(keyAtt->isKeyword());
    i++;
  }
  assertEquals(12, i);
  stream->end();
  delete stream;
}
} // namespace org::apache::lucene::analysis::phonetic