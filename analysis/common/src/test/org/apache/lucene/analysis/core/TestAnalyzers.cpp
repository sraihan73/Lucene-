using namespace std;

#include "TestAnalyzers.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/SimpleAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/StopAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/UnicodeWhitespaceAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/UnicodeWhitespaceTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/UpperCaseFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestAnalyzers::testSimple() 
{
  shared_ptr<Analyzer> a = make_shared<SimpleAnalyzer>();
  assertAnalyzesTo(a, L"foo bar FOO BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"});
  assertAnalyzesTo(a, L"foo      bar .  FOO <> BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"});
  assertAnalyzesTo(a, L"foo.bar.FOO.BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"});
  assertAnalyzesTo(a, L"U.S.A.", std::deque<wstring>{L"u", L"s", L"a"});
  assertAnalyzesTo(a, L"C++", std::deque<wstring>{L"c"});
  assertAnalyzesTo(a, L"B2B", std::deque<wstring>{L"b", L"b"});
  assertAnalyzesTo(a, L"2B", std::deque<wstring>{L"b"});
  assertAnalyzesTo(a, L"\"QUOTED\" word",
                   std::deque<wstring>{L"quoted", L"word"});
  assertEquals(make_shared<BytesRef>(L"\"\\à3[]()! cz@"),
               a->normalize(L"dummy", L"\"\\À3[]()! Cz@"));
  delete a;
}

void TestAnalyzers::testNull() 
{
  shared_ptr<Analyzer> a = make_shared<WhitespaceAnalyzer>();
  assertAnalyzesTo(a, L"foo bar FOO BAR",
                   std::deque<wstring>{L"foo", L"bar", L"FOO", L"BAR"});
  assertAnalyzesTo(
      a, L"foo      bar .  FOO <> BAR",
      std::deque<wstring>{L"foo", L"bar", L".", L"FOO", L"<>", L"BAR"});
  assertAnalyzesTo(a, L"foo.bar.FOO.BAR",
                   std::deque<wstring>{L"foo.bar.FOO.BAR"});
  assertAnalyzesTo(a, L"U.S.A.", std::deque<wstring>{L"U.S.A."});
  assertAnalyzesTo(a, L"C++", std::deque<wstring>{L"C++"});
  assertAnalyzesTo(a, L"B2B", std::deque<wstring>{L"B2B"});
  assertAnalyzesTo(a, L"2B", std::deque<wstring>{L"2B"});
  assertAnalyzesTo(a, L"\"QUOTED\" word",
                   std::deque<wstring>{L"\"QUOTED\"", L"word"});
  assertEquals(make_shared<BytesRef>(L"\"\\À3[]()! Cz@"),
               a->normalize(L"dummy", L"\"\\À3[]()! Cz@"));
  delete a;
}

void TestAnalyzers::testStop() 
{
  shared_ptr<Analyzer> a = make_shared<StopAnalyzer>();
  assertAnalyzesTo(a, L"foo bar FOO BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"});
  assertAnalyzesTo(a, L"foo a bar such FOO THESE BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"});
  assertEquals(make_shared<BytesRef>(L"\"\\à3[]()! cz@"),
               a->normalize(L"dummy", L"\"\\À3[]()! Cz@"));
  assertEquals(make_shared<BytesRef>(L"the"), a->normalize(L"dummy", L"the"));
  delete a;
}

void TestAnalyzers::verifyPayload(shared_ptr<TokenStream> ts) 
{
  shared_ptr<PayloadAttribute> payloadAtt =
      ts->getAttribute(PayloadAttribute::typeid);
  ts->reset();
  for (char b = 1;; b++) {
    bool hasNext = ts->incrementToken();
    if (!hasNext) {
      break;
    }
    // System.out.println("id="+System.identityHashCode(nextToken) + " " + t);
    // System.out.println("payload=" +
    // (int)nextToken.getPayload().toByteArray()[0]);
    assertEquals(b, payloadAtt->getPayload()->bytes[0]);
  }
}

void TestAnalyzers::testPayloadCopy() 
{
  wstring s = L"how now brown cow";
  shared_ptr<TokenStream> ts;
  ts = make_shared<WhitespaceTokenizer>();
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(s));
  ts = make_shared<PayloadSetter>(ts);
  verifyPayload(ts);

  ts = make_shared<WhitespaceTokenizer>();
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(s));
  ts = make_shared<PayloadSetter>(ts);
  verifyPayload(ts);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") public void
// _testStandardConstants()
void TestAnalyzers::_testStandardConstants()
{
  int x = StandardTokenizer::ALPHANUM;
  x = StandardTokenizer::NUM;
  x = StandardTokenizer::SOUTHEAST_ASIAN;
  x = StandardTokenizer::IDEOGRAPHIC;
  x = StandardTokenizer::HIRAGANA;
  x = StandardTokenizer::KATAKANA;
  x = StandardTokenizer::HANGUL;
  std::deque<wstring> y = StandardTokenizer::TOKEN_TYPES;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestAnalyzers::LowerCaseWhitespaceAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      random()->nextBoolean() ? make_shared<WhitespaceTokenizer>()
                              : make_shared<UnicodeWhitespaceTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<LowerCaseFilter>(tokenizer));
}

shared_ptr<Analyzer::TokenStreamComponents>
TestAnalyzers::UpperCaseWhitespaceAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      random()->nextBoolean() ? make_shared<WhitespaceTokenizer>()
                              : make_shared<UnicodeWhitespaceTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<UpperCaseFilter>(tokenizer));
}

void TestAnalyzers::testLowerCaseFilter() 
{
  shared_ptr<Analyzer> a = make_shared<LowerCaseWhitespaceAnalyzer>();
  // BMP
  assertAnalyzesTo(a, L"AbaCaDabA", std::deque<wstring>{L"abacadaba"});
  // supplementary
  assertAnalyzesTo(a, L"\ud801\udc16\ud801\udc16\ud801\udc16\ud801\udc16",
                   std::deque<wstring>{
                       L"\ud801\udc3e\ud801\udc3e\ud801\udc3e\ud801\udc3e"});
  assertAnalyzesTo(a, L"AbaCa\ud801\udc16DabA",
                   std::deque<wstring>{L"abaca\ud801\udc3edaba"});
  // unpaired lead surrogate
  assertAnalyzesTo(a, L"AbaC\uD801AdaBa",
                   std::deque<wstring>{L"abac\uD801adaba"});
  // unpaired trail surrogate
  assertAnalyzesTo(a, L"AbaC\uDC16AdaBa",
                   std::deque<wstring>{L"abac\uDC16adaba"});
  delete a;
}

void TestAnalyzers::testUpperCaseFilter() 
{
  shared_ptr<Analyzer> a = make_shared<UpperCaseWhitespaceAnalyzer>();
  // BMP
  assertAnalyzesTo(a, L"AbaCaDabA", std::deque<wstring>{L"ABACADABA"});
  // supplementary
  assertAnalyzesTo(a, L"\ud801\udc3e\ud801\udc3e\ud801\udc3e\ud801\udc3e",
                   std::deque<wstring>{
                       L"\ud801\udc16\ud801\udc16\ud801\udc16\ud801\udc16"});
  assertAnalyzesTo(a, L"AbaCa\ud801\udc3eDabA",
                   std::deque<wstring>{L"ABACA\ud801\udc16DABA"});
  // unpaired lead surrogate
  assertAnalyzesTo(a, L"AbaC\uD801AdaBa",
                   std::deque<wstring>{L"ABAC\uD801ADABA"});
  // unpaired trail surrogate
  assertAnalyzesTo(a, L"AbaC\uDC16AdaBa",
                   std::deque<wstring>{L"ABAC\uDC16ADABA"});
  delete a;
}

void TestAnalyzers::testLowerCaseFilterLowSurrogateLeftover() 
{
  // test if the limit of the termbuffer is correctly used with supplementary
  // chars
  shared_ptr<WhitespaceTokenizer> tokenizer =
      make_shared<WhitespaceTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(L"BogustermBogusterm\udc16"));
  shared_ptr<LowerCaseFilter> filter = make_shared<LowerCaseFilter>(tokenizer);
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"bogustermbogusterm\udc16"});
  filter->reset();
  wstring highSurEndingUpper = L"BogustermBoguster\ud801";
  wstring highSurEndingLower = L"bogustermboguster\ud801";
  tokenizer->setReader(make_shared<StringReader>(highSurEndingUpper));
  assertTokenStreamContents(filter, std::deque<wstring>{highSurEndingLower});
  assertTrue(filter->hasAttribute(CharTermAttribute::typeid));
  std::deque<wchar_t> termBuffer =
      filter->getAttribute(CharTermAttribute::typeid)->buffer();
  int length = highSurEndingLower.length();
  assertEquals(L'\ud801', termBuffer[length - 1]);
}

void TestAnalyzers::testLowerCaseTokenizer() 
{
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"Tokenizer \ud801\udc1ctest");
  shared_ptr<LowerCaseTokenizer> tokenizer = make_shared<LowerCaseTokenizer>();
  tokenizer->setReader(reader);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"tokenizer", L"\ud801\udc44test"});
}

void TestAnalyzers::testWhitespaceTokenizer() 
{
  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"Tokenizer \ud801\udc1ctest");
  shared_ptr<WhitespaceTokenizer> tokenizer =
      make_shared<WhitespaceTokenizer>();
  tokenizer->setReader(reader);
  assertTokenStreamContents(
      tokenizer, std::deque<wstring>{L"Tokenizer", L"\ud801\udc1ctest"});
}

void TestAnalyzers::testRandomStrings() 
{
  std::deque<std::shared_ptr<Analyzer>> analyzers = {
      make_shared<WhitespaceAnalyzer>(), make_shared<SimpleAnalyzer>(),
      make_shared<StopAnalyzer>(), make_shared<UnicodeWhitespaceAnalyzer>()};
  for (auto analyzer : analyzers) {
    checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  }
  IOUtils::close(analyzers);
}

void TestAnalyzers::testRandomHugeStrings() 
{
  std::deque<std::shared_ptr<Analyzer>> analyzers = {
      make_shared<WhitespaceAnalyzer>(), make_shared<SimpleAnalyzer>(),
      make_shared<StopAnalyzer>(), make_shared<UnicodeWhitespaceAnalyzer>()};
  for (auto analyzer : analyzers) {
    checkRandomData(random(), analyzer, 100 * RANDOM_MULTIPLIER, 8192);
  }
  IOUtils::close(analyzers);
}

PayloadSetter::PayloadSetter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
  payloadAtt = addAttribute(PayloadAttribute::typeid);
}

bool PayloadSetter::incrementToken() 
{
  bool hasNext = input->incrementToken();
  if (!hasNext) {
    return false;
  }
  payloadAtt->setPayload(p); // reuse the payload / byte[]
  data[0]++;
  return true;
}
} // namespace org::apache::lucene::analysis::core