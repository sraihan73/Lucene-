using namespace std;

#include "TestPortugueseMinimalStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/VocabularyAssert.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/pt/PortugueseMinimalStemFilter.h"

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

void TestPortugueseMinimalStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestPortugueseMinimalStemFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestPortugueseMinimalStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPortugueseMinimalStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<PortugueseMinimalStemFilter>(source));
}

void TestPortugueseMinimalStemFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestPortugueseMinimalStemFilter::testExamples() 
{
  assertAnalyzesTo(
      analyzer,
      wstring(L"O debate político, pelo menos o que vem a público, parece, de "
              L"modo nada ") +
          L"surpreendente, restrito a temas menores. Mas há, evidentemente, " +
          L"grandes questões em jogo nas eleições que se aproximam.",
      std::deque<wstring>{L"o",
                           L"debate",
                           L"político",
                           L"pelo",
                           L"menos",
                           L"o",
                           L"que",
                           L"vem",
                           L"a",
                           L"público",
                           L"parece",
                           L"de",
                           L"modo",
                           L"nada",
                           L"surpreendente",
                           L"restrito",
                           L"a",
                           L"tema",
                           L"menor",
                           L"mas",
                           L"há",
                           L"evidentemente",
                           L"grande",
                           L"questão",
                           L"em",
                           L"jogo",
                           L"na",
                           L"eleição",
                           L"que",
                           L"se",
                           L"aproximam"});
}

void TestPortugueseMinimalStemFilter::testVocabulary() 
{
  VocabularyAssert::assertVocabulary(
      analyzer, getDataPath(L"ptminimaltestdata.zip"), L"ptminimal.txt");
}

void TestPortugueseMinimalStemFilter::testKeyword() 
{
  shared_ptr<CharArraySet> *const exclusionSet =
      make_shared<CharArraySet>(asSet({L"quilométricas"}), false);
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), exclusionSet);
  checkOneTerm(a, L"quilométricas", L"quilométricas");
  delete a;
}

TestPortugueseMinimalStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestPortugueseMinimalStemFilter> outerInstance,
        shared_ptr<CharArraySet> exclusionSet)
{
  this->outerInstance = outerInstance;
  this->exclusionSet = exclusionSet;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPortugueseMinimalStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> sink =
      make_shared<SetKeywordMarkerFilter>(source, exclusionSet);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<PortugueseMinimalStemFilter>(sink));
}

void TestPortugueseMinimalStemFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestPortugueseMinimalStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestPortugueseMinimalStemFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestPortugueseMinimalStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPortugueseMinimalStemFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<PortugueseMinimalStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::pt