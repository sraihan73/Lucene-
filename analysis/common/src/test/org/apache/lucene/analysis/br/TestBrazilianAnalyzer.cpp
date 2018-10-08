using namespace std;

#include "TestBrazilianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/br/BrazilianAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/br/BrazilianStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::br
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using LowerCaseTokenizer =
    org::apache::lucene::analysis::core::LowerCaseTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;

void TestBrazilianAnalyzer::testWithSnowballExamples() 
{
  check(L"boa", L"boa");
  check(L"boainain", L"boainain");
  check(L"boas", L"boas");
  check(L"bôas",
        L"boas"); // removes diacritic: different from snowball portugese
  check(L"boassu", L"boassu");
  check(L"boataria", L"boat");
  check(L"boate", L"boat");
  check(L"boates", L"boat");
  check(L"boatos", L"boat");
  check(L"bob", L"bob");
  check(L"boba", L"bob");
  check(L"bobagem", L"bobag");
  check(L"bobagens", L"bobagens");
  check(L"bobalhões",
        L"bobalho"); // removes diacritic: different from snowball portugese
  check(L"bobear", L"bob");
  check(L"bobeira", L"bobeir");
  check(L"bobinho", L"bobinh");
  check(L"bobinhos", L"bobinh");
  check(L"bobo", L"bob");
  check(L"bobs", L"bobs");
  check(L"boca", L"boc");
  check(L"bocadas", L"boc");
  check(L"bocadinho", L"bocadinh");
  check(L"bocado", L"boc");
  check(L"bocaiúva",
        L"bocaiuv"); // removes diacritic: different from snowball portuguese
  check(L"boçal",
        L"bocal"); // removes diacritic: different from snowball portuguese
  check(L"bocarra", L"bocarr");
  check(L"bocas", L"boc");
  check(L"bode", L"bod");
  check(L"bodoque", L"bodoqu");
  check(L"body", L"body");
  check(L"boeing", L"boeing");
  check(L"boem", L"boem");
  check(L"boemia", L"boem");
  check(L"boêmio",
        L"boemi"); // removes diacritic: different from snowball portuguese
  check(L"bogotá", L"bogot");
  check(L"boi", L"boi");
  check(L"bóia",
        L"boi"); // removes diacritic: different from snowball portuguese
  check(L"boiando", L"boi");
  check(L"quiabo", L"quiab");
  check(L"quicaram", L"quic");
  check(L"quickly", L"quickly");
  check(L"quieto", L"quiet");
  check(L"quietos", L"quiet");
  check(L"quilate", L"quilat");
  check(L"quilates", L"quilat");
  check(L"quilinhos", L"quilinh");
  check(L"quilo", L"quil");
  check(L"quilombo", L"quilomb");
  check(L"quilométricas",
        L"quilometr"); // removes diacritic: different from snowball portuguese
  check(L"quilométricos",
        L"quilometr"); // removes diacritic: different from snowball portuguese
  check(L"quilômetro",
        L"quilometr"); // removes diacritic: different from snowball portoguese
  check(L"quilômetros",
        L"quilometr"); // removes diacritic: different from snowball portoguese
  check(L"quilos", L"quil");
  check(L"quimica", L"quimic");
  check(L"quilos", L"quil");
  check(L"quimica", L"quimic");
  check(L"quimicas", L"quimic");
  check(L"quimico", L"quimic");
  check(L"quimicos", L"quimic");
  check(L"quimioterapia", L"quimioterap");
  check(
      L"quimioterápicos",
      L"quimioterap"); // removes diacritic: different from snowball portoguese
  check(L"quimono", L"quimon");
  check(L"quincas", L"quinc");
  check(L"quinhão",
        L"quinha"); // removes diacritic: different from snowball portoguese
  check(L"quinhentos", L"quinhent");
  check(L"quinn", L"quinn");
  check(L"quino", L"quin");
  check(L"quinta", L"quint");
  check(L"quintal", L"quintal");
  check(L"quintana", L"quintan");
  check(L"quintanilha", L"quintanilh");
  check(L"quintão",
        L"quinta"); // removes diacritic: different from snowball portoguese
  check(L"quintessência",
        L"quintessente"); // versus snowball portuguese 'quintessent'
  check(L"quintino", L"quintin");
  check(L"quinto", L"quint");
  check(L"quintos", L"quint");
  check(L"quintuplicou", L"quintuplic");
  check(L"quinze", L"quinz");
  check(L"quinzena", L"quinzen");
  check(L"quiosque", L"quiosqu");
}

void TestBrazilianAnalyzer::testNormalization() 
{
  check(L"Brasil", L"brasil");   // lowercase by default
  check(L"Brasília", L"brasil"); // remove diacritics
  check(L"quimio5terápicos",
        L"quimio5terapicos"); // contains non-letter, diacritic will still be
                              // removed
  check(L"áá", L"áá");        // token is too short: diacritics are not removed
  check(L"ááá", L"aaa");      // normally, diacritics are removed
}

void TestBrazilianAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<BrazilianAnalyzer>();
  checkReuse(a, L"boa", L"boa");
  checkReuse(a, L"boainain", L"boainain");
  checkReuse(a, L"boas", L"boas");
  checkReuse(a, L"bôas",
             L"boas"); // removes diacritic: different from snowball portugese
  delete a;
}

void TestBrazilianAnalyzer::testStemExclusionTable() 
{
  shared_ptr<BrazilianAnalyzer> a = make_shared<BrazilianAnalyzer>(
      CharArraySet::EMPTY_SET,
      make_shared<CharArraySet>(asSet({L"quintessência"}), false));
  checkReuse(a, L"quintessência",
             L"quintessência"); // excluded words will be completely unchanged.
  delete a;
}

void TestBrazilianAnalyzer::testWithKeywordAttribute() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"Brasília");
  shared_ptr<Tokenizer> tokenizer = make_shared<LowerCaseTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(L"Brasília Brasilia"));
  shared_ptr<BrazilianStemFilter> filter = make_shared<BrazilianStemFilter>(
      make_shared<SetKeywordMarkerFilter>(tokenizer, set));

  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"brasília", L"brasil"});
}

void TestBrazilianAnalyzer::check(const wstring &input,
                                  const wstring &expected) 
{
  shared_ptr<BrazilianAnalyzer> a = make_shared<BrazilianAnalyzer>();
  checkOneTerm(a, input, expected);
  delete a;
}

void TestBrazilianAnalyzer::checkReuse(
    shared_ptr<Analyzer> a, const wstring &input,
    const wstring &expected) 
{
  checkOneTerm(a, input, expected);
}

void TestBrazilianAnalyzer::testRandomStrings() 
{
  shared_ptr<BrazilianAnalyzer> a = make_shared<BrazilianAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

void TestBrazilianAnalyzer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestBrazilianAnalyzer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestBrazilianAnalyzer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBrazilianAnalyzer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<BrazilianStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::br