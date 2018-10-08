using namespace std;

#include "TestFrenchLightStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/VocabularyAssert.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/fr/FrenchLightStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::fr
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

void TestFrenchLightStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestFrenchLightStemFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestFrenchLightStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestFrenchLightStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<FrenchLightStemFilter>(source));
}

void TestFrenchLightStemFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestFrenchLightStemFilter::testExamples() 
{
  checkOneTerm(analyzer, L"chevaux", L"cheval");
  checkOneTerm(analyzer, L"cheval", L"cheval");

  checkOneTerm(analyzer, L"hiboux", L"hibou");
  checkOneTerm(analyzer, L"hibou", L"hibou");

  checkOneTerm(analyzer, L"chantés", L"chant");
  checkOneTerm(analyzer, L"chanter", L"chant");
  checkOneTerm(analyzer, L"chante", L"chant");
  checkOneTerm(analyzer, L"chant", L"chant");

  checkOneTerm(analyzer, L"baronnes", L"baron");
  checkOneTerm(analyzer, L"barons", L"baron");
  checkOneTerm(analyzer, L"baron", L"baron");

  checkOneTerm(analyzer, L"peaux", L"peau");
  checkOneTerm(analyzer, L"peau", L"peau");

  checkOneTerm(analyzer, L"anneaux", L"aneau");
  checkOneTerm(analyzer, L"anneau", L"aneau");

  checkOneTerm(analyzer, L"neveux", L"neveu");
  checkOneTerm(analyzer, L"neveu", L"neveu");

  checkOneTerm(analyzer, L"affreux", L"afreu");
  checkOneTerm(analyzer, L"affreuse", L"afreu");

  checkOneTerm(analyzer, L"investissement", L"investi");
  checkOneTerm(analyzer, L"investir", L"investi");

  checkOneTerm(analyzer, L"assourdissant", L"asourdi");
  checkOneTerm(analyzer, L"assourdir", L"asourdi");

  checkOneTerm(analyzer, L"pratiquement", L"pratiqu");
  checkOneTerm(analyzer, L"pratique", L"pratiqu");

  checkOneTerm(analyzer, L"administrativement", L"administratif");
  checkOneTerm(analyzer, L"administratif", L"administratif");

  checkOneTerm(analyzer, L"justificatrice", L"justifi");
  checkOneTerm(analyzer, L"justificateur", L"justifi");
  checkOneTerm(analyzer, L"justifier", L"justifi");

  checkOneTerm(analyzer, L"educatrice", L"eduqu");
  checkOneTerm(analyzer, L"eduquer", L"eduqu");

  checkOneTerm(analyzer, L"communicateur", L"comuniqu");
  checkOneTerm(analyzer, L"communiquer", L"comuniqu");

  checkOneTerm(analyzer, L"accompagnatrice", L"acompagn");
  checkOneTerm(analyzer, L"accompagnateur", L"acompagn");

  checkOneTerm(analyzer, L"administrateur", L"administr");
  checkOneTerm(analyzer, L"administrer", L"administr");

  checkOneTerm(analyzer, L"productrice", L"product");
  checkOneTerm(analyzer, L"producteur", L"product");

  checkOneTerm(analyzer, L"acheteuse", L"achet");
  checkOneTerm(analyzer, L"acheteur", L"achet");

  checkOneTerm(analyzer, L"planteur", L"plant");
  checkOneTerm(analyzer, L"plante", L"plant");

  checkOneTerm(analyzer, L"poreuse", L"poreu");
  checkOneTerm(analyzer, L"poreux", L"poreu");

  checkOneTerm(analyzer, L"plieuse", L"plieu");

  checkOneTerm(analyzer, L"bijoutière", L"bijouti");
  checkOneTerm(analyzer, L"bijoutier", L"bijouti");

  checkOneTerm(analyzer, L"caissière", L"caisi");
  checkOneTerm(analyzer, L"caissier", L"caisi");

  checkOneTerm(analyzer, L"abrasive", L"abrasif");
  checkOneTerm(analyzer, L"abrasif", L"abrasif");

  checkOneTerm(analyzer, L"folle", L"fou");
  checkOneTerm(analyzer, L"fou", L"fou");

  checkOneTerm(analyzer, L"personnelle", L"person");
  checkOneTerm(analyzer, L"personne", L"person");

  // algo bug: too short length
  // checkOneTerm(analyzer, "personnel", "person");

  checkOneTerm(analyzer, L"complète", L"complet");
  checkOneTerm(analyzer, L"complet", L"complet");

  checkOneTerm(analyzer, L"aromatique", L"aromat");

  checkOneTerm(analyzer, L"faiblesse", L"faibl");
  checkOneTerm(analyzer, L"faible", L"faibl");

  checkOneTerm(analyzer, L"patinage", L"patin");
  checkOneTerm(analyzer, L"patin", L"patin");

  checkOneTerm(analyzer, L"sonorisation", L"sono");

  checkOneTerm(analyzer, L"ritualisation", L"rituel");
  checkOneTerm(analyzer, L"rituel", L"rituel");

  // algo bug: masked by rules above
  // checkOneTerm(analyzer, "colonisateur", "colon");

  checkOneTerm(analyzer, L"nomination", L"nomin");

  checkOneTerm(analyzer, L"disposition", L"dispos");
  checkOneTerm(analyzer, L"dispose", L"dispos");

  // SOLR-3463 : abusive compression of repeated characters in numbers
  // Trailing repeated char elision :
  checkOneTerm(analyzer, L"1234555", L"1234555");
  // Repeated char within numbers with more than 4 characters :
  checkOneTerm(analyzer, L"12333345", L"12333345");
  // Short numbers weren't affected already:
  checkOneTerm(analyzer, L"1234", L"1234");
  // Ensure behaviour is preserved for words!
  // Trailing repeated char elision :
  checkOneTerm(analyzer, L"abcdeff", L"abcdef");
  // Repeated char within words with more than 4 characters :
  checkOneTerm(analyzer, L"abcccddeef", L"abcdef");
  checkOneTerm(analyzer, L"créées", L"cre");
  // Combined letter and digit repetition
  checkOneTerm(analyzer, L"22hh00", L"22h00"); // 10:00pm
}

void TestFrenchLightStemFilter::testVocabulary() 
{
  VocabularyAssert::assertVocabulary(
      analyzer, getDataPath(L"frlighttestdata.zip"), L"frlight.txt");
}

void TestFrenchLightStemFilter::testKeyword() 
{
  shared_ptr<CharArraySet> *const exclusionSet =
      make_shared<CharArraySet>(asSet({L"chevaux"}), false);
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), exclusionSet);
  checkOneTerm(a, L"chevaux", L"chevaux");
  delete a;
}

TestFrenchLightStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestFrenchLightStemFilter> outerInstance,
        shared_ptr<CharArraySet> exclusionSet)
{
  this->outerInstance = outerInstance;
  this->exclusionSet = exclusionSet;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestFrenchLightStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> sink =
      make_shared<SetKeywordMarkerFilter>(source, exclusionSet);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<FrenchLightStemFilter>(sink));
}

void TestFrenchLightStemFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestFrenchLightStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestFrenchLightStemFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestFrenchLightStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestFrenchLightStemFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<FrenchLightStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::fr