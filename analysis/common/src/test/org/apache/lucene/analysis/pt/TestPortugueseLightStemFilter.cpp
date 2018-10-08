using namespace std;

#include "TestPortugueseLightStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/VocabularyAssert.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/pt/PortugueseLightStemFilter.h"

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
//    import static org.apache.lucene.analysis.VocabularyAssert.*;

void TestPortugueseLightStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestPortugueseLightStemFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestPortugueseLightStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPortugueseLightStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<PortugueseLightStemFilter>(source));
}

void TestPortugueseLightStemFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestPortugueseLightStemFilter::testExamples() 
{
  assertAnalyzesTo(
      analyzer,
      wstring(L"O debate político, pelo menos o que vem a público, parece, de "
              L"modo nada ") +
          L"surpreendente, restrito a temas menores. Mas há, evidentemente, " +
          L"grandes questões em jogo nas eleições que se aproximam.",
      std::deque<wstring>{L"o",    L"debat",   L"politic",      L"pelo",
                           L"meno", L"o",       L"que",          L"vem",
                           L"a",    L"public",  L"parec",        L"de",
                           L"modo", L"nada",    L"surpreendent", L"restrit",
                           L"a",    L"tema",    L"menor",        L"mas",
                           L"há",   L"evident", L"grand",        L"questa",
                           L"em",   L"jogo",    L"nas",          L"eleica",
                           L"que",  L"se",      L"aproximam"});
}

void TestPortugueseLightStemFilter::testMoreExamples() 
{
  checkOneTerm(analyzer, L"doutores", L"doutor");
  checkOneTerm(analyzer, L"doutor", L"doutor");

  checkOneTerm(analyzer, L"homens", L"homem");
  checkOneTerm(analyzer, L"homem", L"homem");

  checkOneTerm(analyzer, L"papéis", L"papel");
  checkOneTerm(analyzer, L"papel", L"papel");

  checkOneTerm(analyzer, L"normais", L"normal");
  checkOneTerm(analyzer, L"normal", L"normal");

  checkOneTerm(analyzer, L"lencóis", L"lencol");
  checkOneTerm(analyzer, L"lencol", L"lencol");

  checkOneTerm(analyzer, L"barris", L"barril");
  checkOneTerm(analyzer, L"barril", L"barril");

  checkOneTerm(analyzer, L"botões", L"bota");
  checkOneTerm(analyzer, L"botão", L"bota");
}

void TestPortugueseLightStemFilter::testVocabulary() 
{
  VocabularyAssert::assertVocabulary(
      analyzer, getDataPath(L"ptlighttestdata.zip"), L"ptlight.txt");
}

void TestPortugueseLightStemFilter::testKeyword() 
{
  shared_ptr<CharArraySet> *const exclusionSet =
      make_shared<CharArraySet>(asSet({L"quilométricas"}), false);
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), exclusionSet);
  checkOneTerm(a, L"quilométricas", L"quilométricas");
  delete a;
}

TestPortugueseLightStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestPortugueseLightStemFilter> outerInstance,
        shared_ptr<CharArraySet> exclusionSet)
{
  this->outerInstance = outerInstance;
  this->exclusionSet = exclusionSet;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPortugueseLightStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> sink =
      make_shared<SetKeywordMarkerFilter>(source, exclusionSet);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<PortugueseLightStemFilter>(sink));
}

void TestPortugueseLightStemFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestPortugueseLightStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestPortugueseLightStemFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestPortugueseLightStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPortugueseLightStemFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<PortugueseLightStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::pt