using namespace std;

#include "TestHindiNormalizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/hi/HindiNormalizationFilter.h"

namespace org::apache::lucene::analysis::hi
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestHindiNormalizer::testBasics() 
{
  check(L"अँगरेज़ी", L"अंगरेजि");
  check(L"अँगरेजी", L"अंगरेजि");
  check(L"अँग्रेज़ी", L"अंगरेजि");
  check(L"अँग्रेजी", L"अंगरेजि");
  check(L"अंगरेज़ी", L"अंगरेजि");
  check(L"अंगरेजी", L"अंगरेजि");
  check(L"अंग्रेज़ी", L"अंगरेजि");
  check(L"अंग्रेजी", L"अंगरेजि");
}

void TestHindiNormalizer::testDecompositions() 
{
  // removing nukta dot
  check(L"क़िताब", L"किताब");
  check(L"फ़र्ज़", L"फरज");
  check(L"क़र्ज़", L"करज");
  // some other composed nukta forms
  check(L"ऱऴख़ग़ड़ढ़य़", L"रळखगडढय");
  // removal of format (ZWJ/ZWNJ)
  check(L"शार्‍मा", L"शारमा");
  check(L"शार्‌मा", L"शारमा");
  // removal of chandra
  check(L"ॅॆॉॊऍऎऑऒ\u0972", L"ेेोोएएओओअ");
  // vowel shortening
  check(L"आईऊॠॡऐऔीूॄॣैौ", L"अइउऋऌएओिुृॢेो");
}

void TestHindiNormalizer::check(const wstring &input,
                                const wstring &output) 
{
  shared_ptr<Tokenizer> tokenizer = whitespaceMockTokenizer(input);
  shared_ptr<TokenFilter> tf = make_shared<HindiNormalizationFilter>(tokenizer);
  assertTokenStreamContents(tf, std::deque<wstring>{output});
}

void TestHindiNormalizer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestHindiNormalizer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestHindiNormalizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestHindiNormalizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<HindiNormalizationFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::hi