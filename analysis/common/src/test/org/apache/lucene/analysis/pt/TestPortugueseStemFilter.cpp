using namespace std;

#include "TestPortugueseStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/pt/PortugueseStemFilter.h"

namespace org::apache::lucene::analysis::pt
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
//    import static
//    org.apache.lucene.analysis.VocabularyAssert.assertVocabulary;

void TestPortugueseStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestPortugueseStemFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestPortugueseStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPortugueseStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<PortugueseStemFilter>(source));
}

void TestPortugueseStemFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestPortugueseStemFilter::testExamples() 
{
  assertAnalyzesTo(
      analyzer,
      wstring(L"O debate político, pelo menos o que vem a público, parece, de "
              L"modo nada ") +
          L"surpreendente, restrito a temas menores. Mas há, evidentemente, " +
          L"grandes questões em jogo nas eleições que se aproximam.",
      std::deque<wstring>{
          L"o",      L"debat", L"politic",   L"pel",     L"menos", L"o",
          L"que",    L"vem",   L"a",         L"public",  L"parec", L"de",
          L"mod",    L"nad",   L"surpreend", L"restrit", L"a",     L"tem",
          L"men",    L"mas",   L"ha",        L"evid",    L"grand", L"quest",
          L"em",     L"jog",   L"na",        L"eleic",   L"que",   L"se",
          L"aproxim"});
}

void TestPortugueseStemFilter::testVocabulary() 
{
  assertVocabulary(analyzer, getDataPath(L"ptrslptestdata.zip"), L"ptrslp.txt");
}

void TestPortugueseStemFilter::testKeyword() 
{
  shared_ptr<CharArraySet> *const exclusionSet =
      make_shared<CharArraySet>(asSet({L"quilométricas"}), false);
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), exclusionSet);
  checkOneTerm(a, L"quilométricas", L"quilométricas");
  delete a;
}

TestPortugueseStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestPortugueseStemFilter> outerInstance,
        shared_ptr<CharArraySet> exclusionSet)
{
  this->outerInstance = outerInstance;
  this->exclusionSet = exclusionSet;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPortugueseStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> sink =
      make_shared<SetKeywordMarkerFilter>(source, exclusionSet);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<PortugueseStemFilter>(sink));
}

void TestPortugueseStemFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestPortugueseStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestPortugueseStemFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestPortugueseStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPortugueseStemFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<PortugueseStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::pt