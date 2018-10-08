using namespace std;

#include "TestLithuanianStemming.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/snowball/SnowballFilter.h"
#include "../../../../../../java/org/tartarus/snowball/ext/LithuanianStemmer.h"

namespace org::apache::lucene::analysis::lt
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using SnowballFilter = org::apache::lucene::analysis::snowball::SnowballFilter;
using LithuanianStemmer = org::tartarus::snowball::ext::LithuanianStemmer;

void TestLithuanianStemming::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestLithuanianStemming::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestLithuanianStemming> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestLithuanianStemming::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<SnowballFilter>(tokenizer, make_shared<LithuanianStemmer>()));
}

void TestLithuanianStemming::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestLithuanianStemming::testNounsI() 
{
  // n. decl. I (-as)
  checkOneTerm(a, L"vaikas", L"vaik");   // nom. sing.
  checkOneTerm(a, L"vaikai", L"vaik");   // nom. pl.
  checkOneTerm(a, L"vaiko", L"vaik");    // gen. sg.
  checkOneTerm(a, L"vaikų", L"vaik");    // gen. pl.
  checkOneTerm(a, L"vaikui", L"vaik");   // dat. sg.
  checkOneTerm(a, L"vaikams", L"vaik");  // dat. pl.
  checkOneTerm(a, L"vaiką", L"vaik");    // acc. sg.
  checkOneTerm(a, L"vaikus", L"vaik");   // acc. pl.
  checkOneTerm(a, L"vaiku", L"vaik");    // ins. sg.
  checkOneTerm(a, L"vaikais", L"vaik");  // ins. pl.
  checkOneTerm(a, L"vaike", L"vaik");    // loc. sg.
  checkOneTerm(a, L"vaikuose", L"vaik"); // loc. pl.
  checkOneTerm(a, L"vaike", L"vaik");    // voc. sg.
  checkOneTerm(a, L"vaikai", L"vaik");   // voc. pl.

  // n. decl. I (-is)
  checkOneTerm(a, L"brolis", L"brol");    // nom. sing.
  checkOneTerm(a, L"broliai", L"brol");   // nom. pl.
  checkOneTerm(a, L"brolio", L"brol");    // gen. sg.
  checkOneTerm(a, L"brolių", L"brol");    // gen. pl.
  checkOneTerm(a, L"broliui", L"brol");   // dat. sg.
  checkOneTerm(a, L"broliams", L"brol");  // dat. pl.
  checkOneTerm(a, L"brolį", L"brol");     // acc. sg.
  checkOneTerm(a, L"brolius", L"brol");   // acc. pl.
  checkOneTerm(a, L"broliu", L"brol");    // ins. sg.
  checkOneTerm(a, L"broliais", L"brol");  // ins. pl.
  checkOneTerm(a, L"brolyje", L"brol");   // loc. sg.
  checkOneTerm(a, L"broliuose", L"brol"); // loc. pl.
  checkOneTerm(a, L"broli", L"brol");     // voc. sg.
  checkOneTerm(a, L"broliai", L"brol");   // voc. pl.

  // n. decl. I (-ys)
  // note: some forms don't conflate
  checkOneTerm(a, L"arklys", L"arkl");        // nom. sing.
  checkOneTerm(a, L"arkliai", L"arkliai");    // nom. pl.
  checkOneTerm(a, L"arklio", L"arkl");        // gen. sg.
  checkOneTerm(a, L"arklių", L"arkl");        // gen. pl.
  checkOneTerm(a, L"arkliui", L"arkliui");    // dat. sg.
  checkOneTerm(a, L"arkliams", L"arkliam");   // dat. pl.
  checkOneTerm(a, L"arklį", L"arkl");         // acc. sg.
  checkOneTerm(a, L"arklius", L"arklius");    // acc. pl.
  checkOneTerm(a, L"arkliu", L"arkl");        // ins. sg.
  checkOneTerm(a, L"arkliais", L"arkliais");  // ins. pl.
  checkOneTerm(a, L"arklyje", L"arklyj");     // loc. sg.
  checkOneTerm(a, L"arkliuose", L"arkliuos"); // loc. pl.
  checkOneTerm(a, L"arkly", L"arkl");         // voc. sg.
  checkOneTerm(a, L"arkliai", L"arkliai");    // voc. pl.
}

void TestLithuanianStemming::testNounsII() 
{
  // n. decl II (-a)
  checkOneTerm(a, L"motina", L"motin");    // nom. sing.
  checkOneTerm(a, L"motinos", L"motin");   // nom. pl.
  checkOneTerm(a, L"motinos", L"motin");   // gen. sg.
  checkOneTerm(a, L"motinų", L"motin");    // gen. pl.
  checkOneTerm(a, L"motinai", L"motin");   // dat. sg.
  checkOneTerm(a, L"motinoms", L"motin");  // dat. pl.
  checkOneTerm(a, L"motiną", L"motin");    // acc. sg.
  checkOneTerm(a, L"motinas", L"motin");   // acc. pl.
  checkOneTerm(a, L"motina", L"motin");    // ins. sg.
  checkOneTerm(a, L"motinomis", L"motin"); // ins. pl.
  checkOneTerm(a, L"motinoje", L"motin");  // loc. sg.
  checkOneTerm(a, L"motinose", L"motin");  // loc. pl.
  checkOneTerm(a, L"motina", L"motin");    // voc. sg.
  checkOneTerm(a, L"motinos", L"motin");   // voc. pl.

  // n. decl II (-ė)
  checkOneTerm(a, L"katė", L"kat");    // nom. sing.
  checkOneTerm(a, L"katės", L"kat");   // nom. pl.
  checkOneTerm(a, L"katės", L"kat");   // gen. sg.
  checkOneTerm(a, L"kačių", L"kat");   // gen. pl.
  checkOneTerm(a, L"katei", L"kat");   // dat. sg.
  checkOneTerm(a, L"katėms", L"kat");  // dat. pl.
  checkOneTerm(a, L"katę", L"kat");    // acc. sg.
  checkOneTerm(a, L"kates", L"kat");   // acc. pl.
  checkOneTerm(a, L"kate", L"kat");    // ins. sg.
  checkOneTerm(a, L"katėmis", L"kat"); // ins. pl.
  checkOneTerm(a, L"katėje", L"kat");  // loc. sg.
  checkOneTerm(a, L"katėse", L"kat");  // loc. pl.
  checkOneTerm(a, L"kate", L"kat");    // voc. sg.
  checkOneTerm(a, L"katės", L"kat");   // voc. pl.

  // n. decl II (-ti)
  checkOneTerm(a, L"pati", L"pat");     // nom. sing.
  checkOneTerm(a, L"pačios", L"pat");   // nom. pl.
  checkOneTerm(a, L"pačios", L"pat");   // gen. sg.
  checkOneTerm(a, L"pačių", L"pat");    // gen. pl.
  checkOneTerm(a, L"pačiai", L"pat");   // dat. sg.
  checkOneTerm(a, L"pačioms", L"pat");  // dat. pl.
  checkOneTerm(a, L"pačią", L"pat");    // acc. sg.
  checkOneTerm(a, L"pačias", L"pat");   // acc. pl.
  checkOneTerm(a, L"pačia", L"pat");    // ins. sg.
  checkOneTerm(a, L"pačiomis", L"pat"); // ins. pl.
  checkOneTerm(a, L"pačioje", L"pat");  // loc. sg.
  checkOneTerm(a, L"pačiose", L"pat");  // loc. pl.
  checkOneTerm(a, L"pati", L"pat");     // voc. sg.
  checkOneTerm(a, L"pačios", L"pat");   // voc. pl.
}

void TestLithuanianStemming::testNounsIII() 
{
  // n. decl III-m
  checkOneTerm(a, L"vagis", L"vag");   // nom. sing.
  checkOneTerm(a, L"vagys", L"vag");   // nom. pl.
  checkOneTerm(a, L"vagies", L"vag");  // gen. sg.
  checkOneTerm(a, L"vagių", L"vag");   // gen. pl.
  checkOneTerm(a, L"vagiui", L"vag");  // dat. sg.
  checkOneTerm(a, L"vagims", L"vag");  // dat. pl.
  checkOneTerm(a, L"vagį", L"vag");    // acc. sg.
  checkOneTerm(a, L"vagis", L"vag");   // acc. pl.
  checkOneTerm(a, L"vagimi", L"vag");  // ins. sg.
  checkOneTerm(a, L"vagimis", L"vag"); // ins. pl.
  checkOneTerm(a, L"vagyje", L"vag");  // loc. sg.
  checkOneTerm(a, L"vagyse", L"vag");  // loc. pl.
  checkOneTerm(a, L"vagie", L"vag");   // voc. sg.
  checkOneTerm(a, L"vagys", L"vag");   // voc. pl.

  // n. decl III-f
  checkOneTerm(a, L"akis", L"ak");   // nom. sing.
  checkOneTerm(a, L"akys", L"ak");   // nom. pl.
  checkOneTerm(a, L"akies", L"ak");  // gen. sg.
  checkOneTerm(a, L"akių", L"ak");   // gen. pl.
  checkOneTerm(a, L"akiai", L"ak");  // dat. sg.
  checkOneTerm(a, L"akims", L"ak");  // dat. pl.
  checkOneTerm(a, L"akį", L"ak");    // acc. sg.
  checkOneTerm(a, L"akis", L"ak");   // acc. pl.
  checkOneTerm(a, L"akimi", L"ak");  // ins. sg.
  checkOneTerm(a, L"akimis", L"ak"); // ins. pl.
  checkOneTerm(a, L"akyje", L"ak");  // loc. sg.
  checkOneTerm(a, L"akyse", L"ak");  // loc. pl.
  checkOneTerm(a, L"akie", L"ak");   // voc. sg.
  checkOneTerm(a, L"akys", L"ak");   // voc. pl.
}

void TestLithuanianStemming::testNounsIV() 
{
  // n. decl IV (-us)
  checkOneTerm(a, L"sūnus", L"sūn");   // nom. sing.
  checkOneTerm(a, L"sūnūs", L"sūn");   // nom. pl.
  checkOneTerm(a, L"sūnaus", L"sūn");  // gen. sg.
  checkOneTerm(a, L"sūnų", L"sūn");    // gen. pl.
  checkOneTerm(a, L"sūnui", L"sūn");   // dat. sg.
  checkOneTerm(a, L"sūnums", L"sūn");  // dat. pl.
  checkOneTerm(a, L"sūnų", L"sūn");    // acc. sg.
  checkOneTerm(a, L"sūnus", L"sūn");   // acc. pl.
  checkOneTerm(a, L"sūnumi", L"sūn");  // ins. sg.
  checkOneTerm(a, L"sūnumis", L"sūn"); // ins. pl.
  checkOneTerm(a, L"sūnuje", L"sūn");  // loc. sg.
  checkOneTerm(a, L"sūnuose", L"sūn"); // loc. pl.
  checkOneTerm(a, L"sūnau", L"sūn");   // voc. sg.
  checkOneTerm(a, L"sūnūs", L"sūn");   // voc. pl.

  // n. decl IV (-ius)
  checkOneTerm(a, L"profesorius", L"profesor");   // nom. sing.
  checkOneTerm(a, L"profesoriai", L"profesor");   // nom. pl.
  checkOneTerm(a, L"profesoriaus", L"profesor");  // gen. sg.
  checkOneTerm(a, L"profesorių", L"profesor");    // gen. pl.
  checkOneTerm(a, L"profesoriui", L"profesor");   // dat. sg.
  checkOneTerm(a, L"profesoriams", L"profesor");  // dat. pl.
  checkOneTerm(a, L"profesorių", L"profesor");    // acc. sg.
  checkOneTerm(a, L"profesorius", L"profesor");   // acc. pl.
  checkOneTerm(a, L"profesoriumi", L"profesor");  // ins. sg.
  checkOneTerm(a, L"profesoriais", L"profesor");  // ins. pl.
  checkOneTerm(a, L"profesoriuje", L"profesor");  // loc. sg.
  checkOneTerm(a, L"profesoriuose", L"profesor"); // loc. pl.
  checkOneTerm(a, L"profesoriau", L"profesor");   // voc. sg.
  checkOneTerm(a, L"profesoriai", L"profesor");   // voc. pl.
}

void TestLithuanianStemming::testNounsV() 
{
  // n. decl V
  // note: gen.pl. doesn't conflate
  checkOneTerm(a, L"vanduo", L"vand");     // nom. sing.
  checkOneTerm(a, L"vandenys", L"vand");   // nom. pl.
  checkOneTerm(a, L"vandens", L"vand");    // gen. sg.
  checkOneTerm(a, L"vandenų", L"vanden");  // gen. pl.
  checkOneTerm(a, L"vandeniui", L"vand");  // dat. sg.
  checkOneTerm(a, L"vandenims", L"vand");  // dat. pl.
  checkOneTerm(a, L"vandenį", L"vand");    // acc. sg.
  checkOneTerm(a, L"vandenis", L"vand");   // acc. pl.
  checkOneTerm(a, L"vandeniu", L"vand");   // ins. sg.
  checkOneTerm(a, L"vandenimis", L"vand"); // ins. pl.
  checkOneTerm(a, L"vandenyje", L"vand");  // loc. sg.
  checkOneTerm(a, L"vandenyse", L"vand");  // loc. pl.
  checkOneTerm(a, L"vandenie", L"vand");   // voc. sg.
  checkOneTerm(a, L"vandenys", L"vand");   // voc. pl.
}

void TestLithuanianStemming::testAdjI() 
{
  // adj. decl I
  checkOneTerm(a, L"geras", L"ger");   // m. nom. sing.
  checkOneTerm(a, L"geri", L"ger");    // m. nom. pl.
  checkOneTerm(a, L"gero", L"ger");    // m. gen. sg.
  checkOneTerm(a, L"gerų", L"ger");    // m. gen. pl.
  checkOneTerm(a, L"geram", L"ger");   // m. dat. sg.
  checkOneTerm(a, L"geriems", L"ger"); // m. dat. pl.
  checkOneTerm(a, L"gerą", L"ger");    // m. acc. sg.
  checkOneTerm(a, L"gerus", L"ger");   // m. acc. pl.
  checkOneTerm(a, L"geru", L"ger");    // m. ins. sg.
  checkOneTerm(a, L"gerais", L"ger");  // m. ins. pl.
  checkOneTerm(a, L"gerame", L"ger");  // m. loc. sg.
  checkOneTerm(a, L"geruose", L"ger"); // m. loc. pl.

  checkOneTerm(a, L"gera", L"ger");    // f. nom. sing.
  checkOneTerm(a, L"geros", L"ger");   // f. nom. pl.
  checkOneTerm(a, L"geros", L"ger");   // f. gen. sg.
  checkOneTerm(a, L"gerų", L"ger");    // f. gen. pl.
  checkOneTerm(a, L"gerai", L"ger");   // f. dat. sg.
  checkOneTerm(a, L"geroms", L"ger");  // f. dat. pl.
  checkOneTerm(a, L"gerą", L"ger");    // f. acc. sg.
  checkOneTerm(a, L"geras", L"ger");   // f. acc. pl.
  checkOneTerm(a, L"gera", L"ger");    // f. ins. sg.
  checkOneTerm(a, L"geromis", L"ger"); // f. ins. pl.
  checkOneTerm(a, L"geroje", L"ger");  // f. loc. sg.
  checkOneTerm(a, L"gerose", L"ger");  // f. loc. pl.
}

void TestLithuanianStemming::testAdjII() 
{
  // adj. decl II
  checkOneTerm(a, L"gražus", L"graž");    // m. nom. sing.
  checkOneTerm(a, L"gražūs", L"graž");    // m. nom. pl.
  checkOneTerm(a, L"gražaus", L"graž");   // m. gen. sg.
  checkOneTerm(a, L"gražių", L"graž");    // m. gen. pl.
  checkOneTerm(a, L"gražiam", L"graž");   // m. dat. sg.
  checkOneTerm(a, L"gražiems", L"graž");  // m. dat. pl.
  checkOneTerm(a, L"gražų", L"graž");     // m. acc. sg.
  checkOneTerm(a, L"gražius", L"graž");   // m. acc. pl.
  checkOneTerm(a, L"gražiu", L"graž");    // m. ins. sg.
  checkOneTerm(a, L"gražiais", L"graž");  // m. ins. pl.
  checkOneTerm(a, L"gražiame", L"graž");  // m. loc. sg.
  checkOneTerm(a, L"gražiuose", L"graž"); // m. loc. pl.

  checkOneTerm(a, L"graži", L"graž");     // f. nom. sing.
  checkOneTerm(a, L"gražios", L"graž");   // f. nom. pl.
  checkOneTerm(a, L"gražios", L"graž");   // f. gen. sg.
  checkOneTerm(a, L"gražių", L"graž");    // f. gen. pl.
  checkOneTerm(a, L"gražiai", L"graž");   // f. dat. sg.
  checkOneTerm(a, L"gražioms", L"graž");  // f. dat. pl.
  checkOneTerm(a, L"gražią", L"graž");    // f. acc. sg.
  checkOneTerm(a, L"gražias", L"graž");   // f. acc. pl.
  checkOneTerm(a, L"gražia", L"graž");    // f. ins. sg.
  checkOneTerm(a, L"gražiomis", L"graž"); // f. ins. pl.
  checkOneTerm(a, L"gražioje", L"graž");  // f. loc. sg.
  checkOneTerm(a, L"gražiose", L"graž");  // f. loc. pl.
}

void TestLithuanianStemming::testAdjIII() 
{
  // adj. decl III
  checkOneTerm(a, L"vidutinis", L"vidutin");    // m. nom. sing.
  checkOneTerm(a, L"vidutiniai", L"vidutin");   // m. nom. pl.
  checkOneTerm(a, L"vidutinio", L"vidutin");    // m. gen. sg.
  checkOneTerm(a, L"vidutinių", L"vidutin");    // m. gen. pl.
  checkOneTerm(a, L"vidutiniam", L"vidutin");   // m. dat. sg.
  checkOneTerm(a, L"vidutiniams", L"vidutin");  // m. dat. pl.
  checkOneTerm(a, L"vidutinį", L"vidutin");     // m. acc. sg.
  checkOneTerm(a, L"vidutinius", L"vidutin");   // m. acc. pl.
  checkOneTerm(a, L"vidutiniu", L"vidutin");    // m. ins. sg.
  checkOneTerm(a, L"vidutiniais", L"vidutin");  // m. ins. pl.
  checkOneTerm(a, L"vidutiniame", L"vidutin");  // m. loc. sg.
  checkOneTerm(a, L"vidutiniuose", L"vidutin"); // m. loc. pl.

  checkOneTerm(a, L"vidutinė", L"vidutin");    // f. nom. sing.
  checkOneTerm(a, L"vidutinės", L"vidutin");   // f. nom. pl.
  checkOneTerm(a, L"vidutinės", L"vidutin");   // f. gen. sg.
  checkOneTerm(a, L"vidutinių", L"vidutin");   // f. gen. pl.
  checkOneTerm(a, L"vidutinei", L"vidutin");   // f. dat. sg.
  checkOneTerm(a, L"vidutinėms", L"vidutin");  // f. dat. pl.
  checkOneTerm(a, L"vidutinę", L"vidutin");    // f. acc. sg.
  checkOneTerm(a, L"vidutines", L"vidutin");   // f. acc. pl.
  checkOneTerm(a, L"vidutine", L"vidutin");    // f. ins. sg.
  checkOneTerm(a, L"vidutinėmis", L"vidutin"); // f. ins. pl.
  checkOneTerm(a, L"vidutinėje", L"vidutin");  // f. loc. sg.
  checkOneTerm(a, L"vidutinėse", L"vidutin");  // f. loc. pl.
}

void TestLithuanianStemming::testHighFrequencyTerms() 
{
  checkOneTerm(a, L"ir", L"ir");
  checkOneTerm(a, L"kad", L"kad");
  checkOneTerm(a, L"į", L"į");
  checkOneTerm(a, L"tai", L"tai");
  checkOneTerm(a, L"su", L"su");
  checkOneTerm(a, L"o", L"o");
  checkOneTerm(a, L"iš", L"iš");
  checkOneTerm(a, L"kaip", L"kaip");
  checkOneTerm(a, L"bet", L"bet");
  checkOneTerm(a, L"yra", L"yr");
  checkOneTerm(a, L"buvo", L"buv");
  checkOneTerm(a, L"tik", L"tik");
  checkOneTerm(a, L"ne", L"ne");
  checkOneTerm(a, L"taip", L"taip");
  checkOneTerm(a, L"ar", L"ar");
  checkOneTerm(a, L"dar", L"dar");
  checkOneTerm(a, L"jau", L"jau");
  checkOneTerm(a, L"savo", L"sav");
  checkOneTerm(a, L"apie", L"ap");
  checkOneTerm(a, L"kai", L"kai");
  checkOneTerm(a, L"aš", L"aš");
  checkOneTerm(a, L"per", L"per");
  checkOneTerm(a, L"nuo", L"nuo");
  checkOneTerm(a, L"po", L"po");
  checkOneTerm(a, L"jis", L"jis");
  checkOneTerm(a, L"kas", L"kas");
  checkOneTerm(a, L"d", L"d");
  checkOneTerm(a, L"labai", L"lab");
  checkOneTerm(a, L"man", L"man");
  checkOneTerm(a, L"dėl", L"dėl");
  checkOneTerm(a, L"tačiau", L"tat");
  checkOneTerm(a, L"nes", L"nes");
  checkOneTerm(a, L"už", L"už");
  checkOneTerm(a, L"to", L"to");
  checkOneTerm(a, L"jo", L"jo");
  checkOneTerm(a, L"iki", L"ik");
  checkOneTerm(a, L"ką", L"ką");
  checkOneTerm(a, L"mano", L"man");
  checkOneTerm(a, L"metų", L"met");
  checkOneTerm(a, L"nors", L"nor");
  checkOneTerm(a, L"jei", L"jei");
  checkOneTerm(a, L"bus", L"bus");
  checkOneTerm(a, L"jų", L"jų");
  checkOneTerm(a, L"čia", L"čia");
  checkOneTerm(a, L"dabar", L"dabar");
  checkOneTerm(a, L"Lietuvos", L"Lietuv");
  checkOneTerm(a, L"net", L"net");
  checkOneTerm(a, L"nei", L"nei");
  checkOneTerm(a, L"gali", L"gal");
  checkOneTerm(a, L"daug", L"daug");
  checkOneTerm(a, L"prie", L"prie");
  checkOneTerm(a, L"ji", L"ji");
  checkOneTerm(a, L"jos", L"jos");
  checkOneTerm(a, L"pat", L"pat");
  checkOneTerm(a, L"jie", L"jie");
  checkOneTerm(a, L"kur", L"kur");
  checkOneTerm(a, L"gal", L"gal");
  checkOneTerm(a, L"ant", L"ant");
  checkOneTerm(a, L"tiek", L"tiek");
  checkOneTerm(a, L"be", L"be");
  checkOneTerm(a, L"būti", L"būt");
  checkOneTerm(a, L"bei", L"bei");
  checkOneTerm(a, L"daugiau", L"daug");
  checkOneTerm(a, L"turi", L"tur");
  checkOneTerm(a, L"prieš", L"prieš");
  checkOneTerm(a, L"vis", L"vis");
  checkOneTerm(a, L"būtų", L"būt");
  checkOneTerm(a, L"jog", L"jog");
  checkOneTerm(a, L"reikia", L"reik");
  checkOneTerm(a, L"mūsų", L"mūs");
  checkOneTerm(a, L"metu", L"met");
  checkOneTerm(a, L"galima", L"galim");
  checkOneTerm(a, L"nėra", L"nėr");
  checkOneTerm(a, L"arba", L"arb");
  checkOneTerm(a, L"mes", L"mes");
  checkOneTerm(a, L"kurie", L"kur");
  checkOneTerm(a, L"tikrai", L"tikr");
  checkOneTerm(a, L"todėl", L"tod");
  checkOneTerm(a, L"ten", L"ten");
  checkOneTerm(a, L"šiandien", L"šiandien");
  checkOneTerm(a, L"vienas", L"vien");
  checkOneTerm(a, L"visi", L"vis");
  checkOneTerm(a, L"kuris", L"kur");
  checkOneTerm(a, L"tada", L"tad");
  checkOneTerm(a, L"kiek", L"kiek");
  checkOneTerm(a, L"tuo", L"tuo");
  checkOneTerm(a, L"gerai", L"ger");
  checkOneTerm(a, L"nieko", L"niek");
  checkOneTerm(a, L"jį", L"jį");
  checkOneTerm(a, L"kol", L"kol");
  checkOneTerm(a, L"viskas", L"visk");
  checkOneTerm(a, L"mane", L"man");
  checkOneTerm(a, L"kartą", L"kart");
  checkOneTerm(a, L"m", L"m");
  checkOneTerm(a, L"tas", L"tas");
  checkOneTerm(a, L"sakė", L"sak");
  checkOneTerm(a, L"žmonių", L"žmon");
  checkOneTerm(a, L"tu", L"tu");
  checkOneTerm(a, L"dieną", L"dien");
  checkOneTerm(a, L"žmonės", L"žmon");
  checkOneTerm(a, L"metais", L"met");
  checkOneTerm(a, L"vieną", L"vien");
  checkOneTerm(a, L"vėl", L"vėl");
  checkOneTerm(a, L"na", L"na");
  checkOneTerm(a, L"tą", L"tą");
  checkOneTerm(a, L"tiesiog", L"tiesiog");
  checkOneTerm(a, L"toks", L"tok");
  checkOneTerm(a, L"pats", L"pat");
  checkOneTerm(a, L"ko", L"ko");
  checkOneTerm(a, L"Lietuvoje", L"Lietuv");
  checkOneTerm(a, L"pagal", L"pagal");
  checkOneTerm(a, L"jeigu", L"jeig");
  checkOneTerm(a, L"visai", L"vis");
  checkOneTerm(a, L"viena", L"vien");
  checkOneTerm(a, L"šį", L"šį");
  checkOneTerm(a, L"metus", L"met");
  checkOneTerm(a, L"jam", L"jam");
  checkOneTerm(a, L"kodėl", L"kod");
  checkOneTerm(a, L"litų", L"lit");
  checkOneTerm(a, L"ją", L"ją");
  checkOneTerm(a, L"kuri", L"kur");
  checkOneTerm(a, L"darbo", L"darb");
  checkOneTerm(a, L"tarp", L"tarp");
  checkOneTerm(a, L"juk", L"juk");
  checkOneTerm(a, L"laiko", L"laik");
  checkOneTerm(a, L"juos", L"juos");
  checkOneTerm(a, L"visą", L"vis");
  checkOneTerm(a, L"kurios", L"kur");
  checkOneTerm(a, L"tam", L"tam");
  checkOneTerm(a, L"pas", L"pas");
  checkOneTerm(a, L"viską", L"visk");
  checkOneTerm(a, L"Europos", L"Eur");
  checkOneTerm(a, L"atrodo", L"atrod");
  checkOneTerm(a, L"tad", L"tad");
  checkOneTerm(a, L"bent", L"bent");
  checkOneTerm(a, L"kitų", L"kit");
  checkOneTerm(a, L"šis", L"šis");
  checkOneTerm(a, L"Vilniaus", L"Viln");
  checkOneTerm(a, L"beveik", L"bevei");
  checkOneTerm(a, L"proc", L"proc");
  checkOneTerm(a, L"tokia", L"tok");
  checkOneTerm(a, L"šiuo", L"šiuo");
  checkOneTerm(a, L"du", L"du");
  checkOneTerm(a, L"kartu", L"kart");
  checkOneTerm(a, L"visada", L"visad");
  checkOneTerm(a, L"kuo", L"kuo");
}
} // namespace org::apache::lucene::analysis::lt