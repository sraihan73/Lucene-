using namespace std;

#include "TestSnowball.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/snowball/SnowballFilter.h"

namespace org::apache::lucene::analysis::snowball
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

void TestSnowball::testEnglish() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  assertAnalyzesTo(a, L"he abhorred accents",
                   std::deque<wstring>{L"he", L"abhor", L"accent"});
  delete a;
}

TestSnowball::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestSnowball> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSnowball::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SnowballFilter>(tokenizer, L"English"));
}

void TestSnowball::testFilterTokens() 
{
  shared_ptr<SnowballFilter> filter =
      make_shared<SnowballFilter>(make_shared<TestTokenStream>(), L"English");
  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  shared_ptr<OffsetAttribute> offsetAtt =
      filter->getAttribute(OffsetAttribute::typeid);
  shared_ptr<TypeAttribute> typeAtt =
      filter->getAttribute(TypeAttribute::typeid);
  shared_ptr<PayloadAttribute> payloadAtt =
      filter->getAttribute(PayloadAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> posIncAtt =
      filter->getAttribute(PositionIncrementAttribute::typeid);
  shared_ptr<FlagsAttribute> flagsAtt =
      filter->getAttribute(FlagsAttribute::typeid);

  filter->incrementToken();

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"accent", termAtt->toString());
  assertEquals(2, offsetAtt->startOffset());
  assertEquals(7, offsetAtt->endOffset());
  assertEquals(L"wrd", typeAtt->type());
  assertEquals(3, posIncAtt->getPositionIncrement());
  assertEquals(77, flagsAtt->getFlags());
  assertEquals(make_shared<BytesRef>(std::deque<char>{0, 1, 2, 3}),
               payloadAtt->getPayload());
}

TestSnowball::TestTokenStream::TestTokenStream()
    : org::apache::lucene::analysis::TokenStream()
{
}

bool TestSnowball::TestTokenStream::incrementToken()
{
  clearAttributes();
  termAtt->setEmpty()->append(L"accents");
  offsetAtt->setOffset(2, 7);
  typeAtt->setType(L"wrd");
  posIncAtt->setPositionIncrement(3);
  payloadAtt->setPayload(make_shared<BytesRef>(std::deque<char>{0, 1, 2, 3}));
  flagsAtt->setFlags(77);
  return true;
}

std::deque<wstring> TestSnowball::SNOWBALL_LANGS = {
    L"Armenian",  L"Basque",  L"Catalan",    L"Danish",   L"Dutch",
    L"English",   L"Finnish", L"French",     L"German2",  L"German",
    L"Hungarian", L"Irish",   L"Italian",    L"Kp",       L"Lovins",
    L"Norwegian", L"Porter",  L"Portuguese", L"Romanian", L"Russian",
    L"Spanish",   L"Swedish", L"Turkish"};

void TestSnowball::testEmptyTerm() 
{
  for (auto lang : SNOWBALL_LANGS) {
    shared_ptr<Analyzer> a =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), lang);
    checkOneTerm(a, L"", L"");
    delete a;
  }
}

TestSnowball::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestSnowball> outerInstance, const wstring &lang)
{
  this->outerInstance = outerInstance;
  this->lang = lang;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSnowball::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SnowballFilter>(tokenizer, lang));
}

void TestSnowball::testRandomStrings() 
{
  for (auto lang : SNOWBALL_LANGS) {
    checkRandomStrings(lang);
  }
}

void TestSnowball::checkRandomStrings(const wstring &snowballLanguage) throw(
    IOException)
{
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), snowballLanguage);
  checkRandomData(random(), a, 100 * RANDOM_MULTIPLIER);
  delete a;
}

TestSnowball::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestSnowball> outerInstance, const wstring &snowballLanguage)
{
  this->outerInstance = outerInstance;
  this->snowballLanguage = snowballLanguage;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSnowball::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t = make_shared<MockTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      t, make_shared<SnowballFilter>(t, snowballLanguage));
}
} // namespace org::apache::lucene::analysis::snowball