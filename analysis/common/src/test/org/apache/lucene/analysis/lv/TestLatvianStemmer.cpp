using namespace std;

#include "TestLatvianStemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/lv/LatvianStemFilter.h"

namespace org::apache::lucene::analysis::lv
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestLatvianStemmer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestLatvianStemmer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestLatvianStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestLatvianStemmer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<LatvianStemFilter>(tokenizer));
}

void TestLatvianStemmer::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestLatvianStemmer::testNouns1() 
{
  // decl. I
  checkOneTerm(a, L"tēvs", L"tēv");   // nom. sing.
  checkOneTerm(a, L"tēvi", L"tēv");   // nom. pl.
  checkOneTerm(a, L"tēva", L"tēv");   // gen. sing.
  checkOneTerm(a, L"tēvu", L"tēv");   // gen. pl.
  checkOneTerm(a, L"tēvam", L"tēv");  // dat. sing.
  checkOneTerm(a, L"tēviem", L"tēv"); // dat. pl.
  checkOneTerm(a, L"tēvu", L"tēv");   // acc. sing.
  checkOneTerm(a, L"tēvus", L"tēv");  // acc. pl.
  checkOneTerm(a, L"tēvā", L"tēv");   // loc. sing.
  checkOneTerm(a, L"tēvos", L"tēv");  // loc. pl.
  checkOneTerm(a, L"tēvs", L"tēv");   // voc. sing.
  checkOneTerm(a, L"tēvi", L"tēv");   // voc. pl.
}

void TestLatvianStemmer::testNouns2() 
{
  // decl. II

  // c -> č palatalization
  checkOneTerm(a, L"lācis", L"lāc");  // nom. sing.
  checkOneTerm(a, L"lāči", L"lāc");   // nom. pl.
  checkOneTerm(a, L"lāča", L"lāc");   // gen. sing.
  checkOneTerm(a, L"lāču", L"lāc");   // gen. pl.
  checkOneTerm(a, L"lācim", L"lāc");  // dat. sing.
  checkOneTerm(a, L"lāčiem", L"lāc"); // dat. pl.
  checkOneTerm(a, L"lāci", L"lāc");   // acc. sing.
  checkOneTerm(a, L"lāčus", L"lāc");  // acc. pl.
  checkOneTerm(a, L"lācī", L"lāc");   // loc. sing.
  checkOneTerm(a, L"lāčos", L"lāc");  // loc. pl.
  checkOneTerm(a, L"lāci", L"lāc");   // voc. sing.
  checkOneTerm(a, L"lāči", L"lāc");   // voc. pl.

  // n -> ņ palatalization
  checkOneTerm(a, L"akmens", L"akmen");   // nom. sing.
  checkOneTerm(a, L"akmeņi", L"akmen");   // nom. pl.
  checkOneTerm(a, L"akmens", L"akmen");   // gen. sing.
  checkOneTerm(a, L"akmeņu", L"akmen");   // gen. pl.
  checkOneTerm(a, L"akmenim", L"akmen");  // dat. sing.
  checkOneTerm(a, L"akmeņiem", L"akmen"); // dat. pl.
  checkOneTerm(a, L"akmeni", L"akmen");   // acc. sing.
  checkOneTerm(a, L"akmeņus", L"akmen");  // acc. pl.
  checkOneTerm(a, L"akmenī", L"akmen");   // loc. sing.
  checkOneTerm(a, L"akmeņos", L"akmen");  // loc. pl.
  checkOneTerm(a, L"akmens", L"akmen");   // voc. sing.
  checkOneTerm(a, L"akmeņi", L"akmen");   // voc. pl.

  // no palatalization
  checkOneTerm(a, L"kurmis", L"kurm");   // nom. sing.
  checkOneTerm(a, L"kurmji", L"kurm");   // nom. pl.
  checkOneTerm(a, L"kurmja", L"kurm");   // gen. sing.
  checkOneTerm(a, L"kurmju", L"kurm");   // gen. pl.
  checkOneTerm(a, L"kurmim", L"kurm");   // dat. sing.
  checkOneTerm(a, L"kurmjiem", L"kurm"); // dat. pl.
  checkOneTerm(a, L"kurmi", L"kurm");    // acc. sing.
  checkOneTerm(a, L"kurmjus", L"kurm");  // acc. pl.
  checkOneTerm(a, L"kurmī", L"kurm");    // loc. sing.
  checkOneTerm(a, L"kurmjos", L"kurm");  // loc. pl.
  checkOneTerm(a, L"kurmi", L"kurm");    // voc. sing.
  checkOneTerm(a, L"kurmji", L"kurm");   // voc. pl.
}

void TestLatvianStemmer::testNouns3() 
{
  // decl III
  checkOneTerm(a, L"lietus", L"liet");  // nom. sing.
  checkOneTerm(a, L"lieti", L"liet");   // nom. pl.
  checkOneTerm(a, L"lietus", L"liet");  // gen. sing.
  checkOneTerm(a, L"lietu", L"liet");   // gen. pl.
  checkOneTerm(a, L"lietum", L"liet");  // dat. sing.
  checkOneTerm(a, L"lietiem", L"liet"); // dat. pl.
  checkOneTerm(a, L"lietu", L"liet");   // acc. sing.
  checkOneTerm(a, L"lietus", L"liet");  // acc. pl.
  checkOneTerm(a, L"lietū", L"liet");   // loc. sing.
  checkOneTerm(a, L"lietos", L"liet");  // loc. pl.
  checkOneTerm(a, L"lietus", L"liet");  // voc. sing.
  checkOneTerm(a, L"lieti", L"liet");   // voc. pl.
}

void TestLatvianStemmer::testNouns4() 
{
  // decl IV
  checkOneTerm(a, L"lapa", L"lap");  // nom. sing.
  checkOneTerm(a, L"lapas", L"lap"); // nom. pl.
  checkOneTerm(a, L"lapas", L"lap"); // gen. sing.
  checkOneTerm(a, L"lapu", L"lap");  // gen. pl.
  checkOneTerm(a, L"lapai", L"lap"); // dat. sing.
  checkOneTerm(a, L"lapām", L"lap"); // dat. pl.
  checkOneTerm(a, L"lapu", L"lap");  // acc. sing.
  checkOneTerm(a, L"lapas", L"lap"); // acc. pl.
  checkOneTerm(a, L"lapā", L"lap");  // loc. sing.
  checkOneTerm(a, L"lapās", L"lap"); // loc. pl.
  checkOneTerm(a, L"lapa", L"lap");  // voc. sing.
  checkOneTerm(a, L"lapas", L"lap"); // voc. pl.

  checkOneTerm(a, L"puika", L"puik");  // nom. sing.
  checkOneTerm(a, L"puikas", L"puik"); // nom. pl.
  checkOneTerm(a, L"puikas", L"puik"); // gen. sing.
  checkOneTerm(a, L"puiku", L"puik");  // gen. pl.
  checkOneTerm(a, L"puikam", L"puik"); // dat. sing.
  checkOneTerm(a, L"puikām", L"puik"); // dat. pl.
  checkOneTerm(a, L"puiku", L"puik");  // acc. sing.
  checkOneTerm(a, L"puikas", L"puik"); // acc. pl.
  checkOneTerm(a, L"puikā", L"puik");  // loc. sing.
  checkOneTerm(a, L"puikās", L"puik"); // loc. pl.
  checkOneTerm(a, L"puika", L"puik");  // voc. sing.
  checkOneTerm(a, L"puikas", L"puik"); // voc. pl.
}

void TestLatvianStemmer::testNouns5() 
{
  // decl V
  // l -> ļ palatalization
  checkOneTerm(a, L"egle", L"egl");  // nom. sing.
  checkOneTerm(a, L"egles", L"egl"); // nom. pl.
  checkOneTerm(a, L"egles", L"egl"); // gen. sing.
  checkOneTerm(a, L"egļu", L"egl");  // gen. pl.
  checkOneTerm(a, L"eglei", L"egl"); // dat. sing.
  checkOneTerm(a, L"eglēm", L"egl"); // dat. pl.
  checkOneTerm(a, L"egli", L"egl");  // acc. sing.
  checkOneTerm(a, L"egles", L"egl"); // acc. pl.
  checkOneTerm(a, L"eglē", L"egl");  // loc. sing.
  checkOneTerm(a, L"eglēs", L"egl"); // loc. pl.
  checkOneTerm(a, L"egle", L"egl");  // voc. sing.
  checkOneTerm(a, L"egles", L"egl"); // voc. pl.
}

void TestLatvianStemmer::testNouns6() 
{
  // decl VI

  // no palatalization
  checkOneTerm(a, L"govs", L"gov");  // nom. sing.
  checkOneTerm(a, L"govis", L"gov"); // nom. pl.
  checkOneTerm(a, L"govs", L"gov");  // gen. sing.
  checkOneTerm(a, L"govju", L"gov"); // gen. pl.
  checkOneTerm(a, L"govij", L"gov"); // dat. sing.
  checkOneTerm(a, L"govīm", L"gov"); // dat. pl.
  checkOneTerm(a, L"govi ", L"gov"); // acc. sing.
  checkOneTerm(a, L"govis", L"gov"); // acc. pl.
  checkOneTerm(a, L"govi ", L"gov"); // inst. sing.
  checkOneTerm(a, L"govīm", L"gov"); // inst. pl.
  checkOneTerm(a, L"govī", L"gov");  // loc. sing.
  checkOneTerm(a, L"govīs", L"gov"); // loc. pl.
  checkOneTerm(a, L"govs", L"gov");  // voc. sing.
  checkOneTerm(a, L"govis", L"gov"); // voc. pl.
}

void TestLatvianStemmer::testAdjectives() 
{
  checkOneTerm(a, L"zils", L"zil");     // indef. nom. masc. sing.
  checkOneTerm(a, L"zilais", L"zil");   // def. nom. masc. sing.
  checkOneTerm(a, L"zili", L"zil");     // indef. nom. masc. pl.
  checkOneTerm(a, L"zilie", L"zil");    // def. nom. masc. pl.
  checkOneTerm(a, L"zila", L"zil");     // indef. nom. fem. sing.
  checkOneTerm(a, L"zilā", L"zil");     // def. nom. fem. sing.
  checkOneTerm(a, L"zilas", L"zil");    // indef. nom. fem. pl.
  checkOneTerm(a, L"zilās", L"zil");    // def. nom. fem. pl.
  checkOneTerm(a, L"zila", L"zil");     // indef. gen. masc. sing.
  checkOneTerm(a, L"zilā", L"zil");     // def. gen. masc. sing.
  checkOneTerm(a, L"zilu", L"zil");     // indef. gen. masc. pl.
  checkOneTerm(a, L"zilo", L"zil");     // def. gen. masc. pl.
  checkOneTerm(a, L"zilas", L"zil");    // indef. gen. fem. sing.
  checkOneTerm(a, L"zilās", L"zil");    // def. gen. fem. sing.
  checkOneTerm(a, L"zilu", L"zil");     // indef. gen. fem. pl.
  checkOneTerm(a, L"zilo", L"zil");     // def. gen. fem. pl.
  checkOneTerm(a, L"zilam", L"zil");    // indef. dat. masc. sing.
  checkOneTerm(a, L"zilajam", L"zil");  // def. dat. masc. sing.
  checkOneTerm(a, L"ziliem", L"zil");   // indef. dat. masc. pl.
  checkOneTerm(a, L"zilajiem", L"zil"); // def. dat. masc. pl.
  checkOneTerm(a, L"zilai", L"zil");    // indef. dat. fem. sing.
  checkOneTerm(a, L"zilajai", L"zil");  // def. dat. fem. sing.
  checkOneTerm(a, L"zilām", L"zil");    // indef. dat. fem. pl.
  checkOneTerm(a, L"zilajām", L"zil");  // def. dat. fem. pl.
  checkOneTerm(a, L"zilu", L"zil");     // indef. acc. masc. sing.
  checkOneTerm(a, L"zilo", L"zil");     // def. acc. masc. sing.
  checkOneTerm(a, L"zilus", L"zil");    // indef. acc. masc. pl.
  checkOneTerm(a, L"zilos", L"zil");    // def. acc. masc. pl.
  checkOneTerm(a, L"zilu", L"zil");     // indef. acc. fem. sing.
  checkOneTerm(a, L"zilo", L"zil");     // def. acc. fem. sing.
  checkOneTerm(a, L"zilās", L"zil");    // indef. acc. fem. pl.
  checkOneTerm(a, L"zilās", L"zil");    // def. acc. fem. pl.
  checkOneTerm(a, L"zilā", L"zil");     // indef. loc. masc. sing.
  checkOneTerm(a, L"zilajā", L"zil");   // def. loc. masc. sing.
  checkOneTerm(a, L"zilos", L"zil");    // indef. loc. masc. pl.
  checkOneTerm(a, L"zilajos", L"zil");  // def. loc. masc. pl.
  checkOneTerm(a, L"zilā", L"zil");     // indef. loc. fem. sing.
  checkOneTerm(a, L"zilajā", L"zil");   // def. loc. fem. sing.
  checkOneTerm(a, L"zilās", L"zil");    // indef. loc. fem. pl.
  checkOneTerm(a, L"zilajās", L"zil");  // def. loc. fem. pl.
  checkOneTerm(a, L"zilais", L"zil");   // voc. masc. sing.
  checkOneTerm(a, L"zilie", L"zil");    // voc. masc. pl.
  checkOneTerm(a, L"zilā", L"zil");     // voc. fem. sing.
  checkOneTerm(a, L"zilās", L"zil");    // voc. fem. pl.
}

void TestLatvianStemmer::testPalatalization() 
{
  checkOneTerm(a, L"krāsns", L"krāsn");     // nom. sing.
  checkOneTerm(a, L"krāšņu", L"krāsn");     // gen. pl.
  checkOneTerm(a, L"zvaigzne", L"zvaigzn"); // nom. sing.
  checkOneTerm(a, L"zvaigžņu", L"zvaigzn"); // gen. pl.
  checkOneTerm(a, L"kāpslis", L"kāpsl");    // nom. sing.
  checkOneTerm(a, L"kāpšļu", L"kāpsl");     // gen. pl.
  checkOneTerm(a, L"zizlis", L"zizl");      // nom. sing.
  checkOneTerm(a, L"zižļu", L"zizl");       // gen. pl.
  checkOneTerm(a, L"vilnis", L"viln");      // nom. sing.
  checkOneTerm(a, L"viļņu", L"viln");       // gen. pl.
  checkOneTerm(a, L"lelle", L"lell");       // nom. sing.
  checkOneTerm(a, L"leļļu", L"lell");       // gen. pl.
  checkOneTerm(a, L"pinne", L"pinn");       // nom. sing.
  checkOneTerm(a, L"piņņu", L"pinn");       // gen. pl.
  checkOneTerm(a, L"rīkste", L"rīkst");     // nom. sing.
  checkOneTerm(a, L"rīkšu", L"rīkst");      // gen. pl.
}

void TestLatvianStemmer::testLength() 
{
  checkOneTerm(a, L"usa", L"usa");   // length
  checkOneTerm(a, L"60ms", L"60ms"); // vowel count
}

void TestLatvianStemmer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestLatvianStemmer::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestLatvianStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestLatvianStemmer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<LatvianStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::lv