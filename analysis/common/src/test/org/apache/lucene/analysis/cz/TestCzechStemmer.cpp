using namespace std;

#include "TestCzechStemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/cz/CzechAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/cz/CzechStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::cz
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;

void TestCzechStemmer::testMasculineNouns() 
{
  shared_ptr<CzechAnalyzer> cz = make_shared<CzechAnalyzer>();

  /* animate ending with a hard consonant */
  assertAnalyzesTo(cz, L"pán", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"páni", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"pánové", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"pána", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"pánů", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"pánovi", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"pánům", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"pány", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"páne", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"pánech", std::deque<wstring>{L"pán"});
  assertAnalyzesTo(cz, L"pánem", std::deque<wstring>{L"pán"});

  /* inanimate ending with hard consonant */
  assertAnalyzesTo(cz, L"hrad", std::deque<wstring>{L"hrad"});
  assertAnalyzesTo(cz, L"hradu", std::deque<wstring>{L"hrad"});
  assertAnalyzesTo(cz, L"hrade", std::deque<wstring>{L"hrad"});
  assertAnalyzesTo(cz, L"hradem", std::deque<wstring>{L"hrad"});
  assertAnalyzesTo(cz, L"hrady", std::deque<wstring>{L"hrad"});
  assertAnalyzesTo(cz, L"hradech", std::deque<wstring>{L"hrad"});
  assertAnalyzesTo(cz, L"hradům", std::deque<wstring>{L"hrad"});
  assertAnalyzesTo(cz, L"hradů", std::deque<wstring>{L"hrad"});

  /* animate ending with a soft consonant */
  assertAnalyzesTo(cz, L"muž", std::deque<wstring>{L"muh"});
  assertAnalyzesTo(cz, L"muži", std::deque<wstring>{L"muh"});
  assertAnalyzesTo(cz, L"muže", std::deque<wstring>{L"muh"});
  assertAnalyzesTo(cz, L"mužů", std::deque<wstring>{L"muh"});
  assertAnalyzesTo(cz, L"mužům", std::deque<wstring>{L"muh"});
  assertAnalyzesTo(cz, L"mužích", std::deque<wstring>{L"muh"});
  assertAnalyzesTo(cz, L"mužem", std::deque<wstring>{L"muh"});

  /* inanimate ending with a soft consonant */
  assertAnalyzesTo(cz, L"stroj", std::deque<wstring>{L"stroj"});
  assertAnalyzesTo(cz, L"stroje", std::deque<wstring>{L"stroj"});
  assertAnalyzesTo(cz, L"strojů", std::deque<wstring>{L"stroj"});
  assertAnalyzesTo(cz, L"stroji", std::deque<wstring>{L"stroj"});
  assertAnalyzesTo(cz, L"strojům", std::deque<wstring>{L"stroj"});
  assertAnalyzesTo(cz, L"strojích", std::deque<wstring>{L"stroj"});
  assertAnalyzesTo(cz, L"strojem", std::deque<wstring>{L"stroj"});

  /* ending with a */
  assertAnalyzesTo(cz, L"předseda", std::deque<wstring>{L"předsd"});
  assertAnalyzesTo(cz, L"předsedové", std::deque<wstring>{L"předsd"});
  assertAnalyzesTo(cz, L"předsedy", std::deque<wstring>{L"předsd"});
  assertAnalyzesTo(cz, L"předsedů", std::deque<wstring>{L"předsd"});
  assertAnalyzesTo(cz, L"předsedovi", std::deque<wstring>{L"předsd"});
  assertAnalyzesTo(cz, L"předsedům", std::deque<wstring>{L"předsd"});
  assertAnalyzesTo(cz, L"předsedu", std::deque<wstring>{L"předsd"});
  assertAnalyzesTo(cz, L"předsedo", std::deque<wstring>{L"předsd"});
  assertAnalyzesTo(cz, L"předsedech", std::deque<wstring>{L"předsd"});
  assertAnalyzesTo(cz, L"předsedou", std::deque<wstring>{L"předsd"});

  /* ending with e */
  assertAnalyzesTo(cz, L"soudce", std::deque<wstring>{L"soudk"});
  assertAnalyzesTo(cz, L"soudci", std::deque<wstring>{L"soudk"});
  assertAnalyzesTo(cz, L"soudců", std::deque<wstring>{L"soudk"});
  assertAnalyzesTo(cz, L"soudcům", std::deque<wstring>{L"soudk"});
  assertAnalyzesTo(cz, L"soudcích", std::deque<wstring>{L"soudk"});
  assertAnalyzesTo(cz, L"soudcem", std::deque<wstring>{L"soudk"});

  delete cz;
}

void TestCzechStemmer::testFeminineNouns() 
{
  shared_ptr<CzechAnalyzer> cz = make_shared<CzechAnalyzer>();

  /* ending with hard consonant */
  assertAnalyzesTo(cz, L"kost", std::deque<wstring>{L"kost"});
  assertAnalyzesTo(cz, L"kosti", std::deque<wstring>{L"kost"});
  assertAnalyzesTo(cz, L"kostí", std::deque<wstring>{L"kost"});
  assertAnalyzesTo(cz, L"kostem", std::deque<wstring>{L"kost"});
  assertAnalyzesTo(cz, L"kostech", std::deque<wstring>{L"kost"});
  assertAnalyzesTo(cz, L"kostmi", std::deque<wstring>{L"kost"});

  /* ending with a soft consonant */
  // note: in this example sing nom. and sing acc. don't conflate w/ the rest
  assertAnalyzesTo(cz, L"píseň", std::deque<wstring>{L"písň"});
  assertAnalyzesTo(cz, L"písně", std::deque<wstring>{L"písn"});
  assertAnalyzesTo(cz, L"písni", std::deque<wstring>{L"písn"});
  assertAnalyzesTo(cz, L"písněmi", std::deque<wstring>{L"písn"});
  assertAnalyzesTo(cz, L"písních", std::deque<wstring>{L"písn"});
  assertAnalyzesTo(cz, L"písním", std::deque<wstring>{L"písn"});

  /* ending with e */
  assertAnalyzesTo(cz, L"růže", std::deque<wstring>{L"růh"});
  assertAnalyzesTo(cz, L"růží", std::deque<wstring>{L"růh"});
  assertAnalyzesTo(cz, L"růžím", std::deque<wstring>{L"růh"});
  assertAnalyzesTo(cz, L"růžích", std::deque<wstring>{L"růh"});
  assertAnalyzesTo(cz, L"růžemi", std::deque<wstring>{L"růh"});
  assertAnalyzesTo(cz, L"růži", std::deque<wstring>{L"růh"});

  /* ending with a */
  assertAnalyzesTo(cz, L"žena", std::deque<wstring>{L"žn"});
  assertAnalyzesTo(cz, L"ženy", std::deque<wstring>{L"žn"});
  assertAnalyzesTo(cz, L"žen", std::deque<wstring>{L"žn"});
  assertAnalyzesTo(cz, L"ženě", std::deque<wstring>{L"žn"});
  assertAnalyzesTo(cz, L"ženám", std::deque<wstring>{L"žn"});
  assertAnalyzesTo(cz, L"ženu", std::deque<wstring>{L"žn"});
  assertAnalyzesTo(cz, L"ženo", std::deque<wstring>{L"žn"});
  assertAnalyzesTo(cz, L"ženách", std::deque<wstring>{L"žn"});
  assertAnalyzesTo(cz, L"ženou", std::deque<wstring>{L"žn"});
  assertAnalyzesTo(cz, L"ženami", std::deque<wstring>{L"žn"});

  delete cz;
}

void TestCzechStemmer::testNeuterNouns() 
{
  shared_ptr<CzechAnalyzer> cz = make_shared<CzechAnalyzer>();

  /* ending with o */
  assertAnalyzesTo(cz, L"město", std::deque<wstring>{L"měst"});
  assertAnalyzesTo(cz, L"města", std::deque<wstring>{L"měst"});
  assertAnalyzesTo(cz, L"měst", std::deque<wstring>{L"měst"});
  assertAnalyzesTo(cz, L"městu", std::deque<wstring>{L"měst"});
  assertAnalyzesTo(cz, L"městům", std::deque<wstring>{L"měst"});
  assertAnalyzesTo(cz, L"městě", std::deque<wstring>{L"měst"});
  assertAnalyzesTo(cz, L"městech", std::deque<wstring>{L"měst"});
  assertAnalyzesTo(cz, L"městem", std::deque<wstring>{L"měst"});
  assertAnalyzesTo(cz, L"městy", std::deque<wstring>{L"měst"});

  /* ending with e */
  assertAnalyzesTo(cz, L"moře", std::deque<wstring>{L"moř"});
  assertAnalyzesTo(cz, L"moří", std::deque<wstring>{L"moř"});
  assertAnalyzesTo(cz, L"mořím", std::deque<wstring>{L"moř"});
  assertAnalyzesTo(cz, L"moři", std::deque<wstring>{L"moř"});
  assertAnalyzesTo(cz, L"mořích", std::deque<wstring>{L"moř"});
  assertAnalyzesTo(cz, L"mořem", std::deque<wstring>{L"moř"});

  /* ending with ě */
  assertAnalyzesTo(cz, L"kuře", std::deque<wstring>{L"kuř"});
  assertAnalyzesTo(cz, L"kuřata", std::deque<wstring>{L"kuř"});
  assertAnalyzesTo(cz, L"kuřete", std::deque<wstring>{L"kuř"});
  assertAnalyzesTo(cz, L"kuřat", std::deque<wstring>{L"kuř"});
  assertAnalyzesTo(cz, L"kuřeti", std::deque<wstring>{L"kuř"});
  assertAnalyzesTo(cz, L"kuřatům", std::deque<wstring>{L"kuř"});
  assertAnalyzesTo(cz, L"kuřatech", std::deque<wstring>{L"kuř"});
  assertAnalyzesTo(cz, L"kuřetem", std::deque<wstring>{L"kuř"});
  assertAnalyzesTo(cz, L"kuřaty", std::deque<wstring>{L"kuř"});

  /* ending with í */
  assertAnalyzesTo(cz, L"stavení", std::deque<wstring>{L"stavn"});
  assertAnalyzesTo(cz, L"stavením", std::deque<wstring>{L"stavn"});
  assertAnalyzesTo(cz, L"staveních", std::deque<wstring>{L"stavn"});
  assertAnalyzesTo(cz, L"staveními", std::deque<wstring>{L"stavn"});

  delete cz;
}

void TestCzechStemmer::testAdjectives() 
{
  shared_ptr<CzechAnalyzer> cz = make_shared<CzechAnalyzer>();

  /* ending with ý/á/é */
  assertAnalyzesTo(cz, L"mladý", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladí", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladého", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladých", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladému", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladým", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladé", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladém", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladými", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladá", std::deque<wstring>{L"mlad"});
  assertAnalyzesTo(cz, L"mladou", std::deque<wstring>{L"mlad"});

  /* ending with í */
  assertAnalyzesTo(cz, L"jarní", std::deque<wstring>{L"jarn"});
  assertAnalyzesTo(cz, L"jarního", std::deque<wstring>{L"jarn"});
  assertAnalyzesTo(cz, L"jarních", std::deque<wstring>{L"jarn"});
  assertAnalyzesTo(cz, L"jarnímu", std::deque<wstring>{L"jarn"});
  assertAnalyzesTo(cz, L"jarním", std::deque<wstring>{L"jarn"});
  assertAnalyzesTo(cz, L"jarními", std::deque<wstring>{L"jarn"});

  delete cz;
}

void TestCzechStemmer::testPossessive() 
{
  shared_ptr<CzechAnalyzer> cz = make_shared<CzechAnalyzer>();
  assertAnalyzesTo(cz, L"Karlův", std::deque<wstring>{L"karl"});
  assertAnalyzesTo(cz, L"jazykový", std::deque<wstring>{L"jazyk"});
  delete cz;
}

void TestCzechStemmer::testExceptions() 
{
  shared_ptr<CzechAnalyzer> cz = make_shared<CzechAnalyzer>();

  /* rewrite of št -> sk */
  assertAnalyzesTo(cz, L"český", std::deque<wstring>{L"česk"});
  assertAnalyzesTo(cz, L"čeští", std::deque<wstring>{L"česk"});

  /* rewrite of čt -> ck */
  assertAnalyzesTo(cz, L"anglický", std::deque<wstring>{L"anglick"});
  assertAnalyzesTo(cz, L"angličtí", std::deque<wstring>{L"anglick"});

  /* rewrite of z -> h */
  assertAnalyzesTo(cz, L"kniha", std::deque<wstring>{L"knih"});
  assertAnalyzesTo(cz, L"knize", std::deque<wstring>{L"knih"});

  /* rewrite of ž -> h */
  assertAnalyzesTo(cz, L"mazat", std::deque<wstring>{L"mah"});
  assertAnalyzesTo(cz, L"mažu", std::deque<wstring>{L"mah"});

  /* rewrite of c -> k */
  assertAnalyzesTo(cz, L"kluk", std::deque<wstring>{L"kluk"});
  assertAnalyzesTo(cz, L"kluci", std::deque<wstring>{L"kluk"});
  assertAnalyzesTo(cz, L"klucích", std::deque<wstring>{L"kluk"});

  /* rewrite of č -> k */
  assertAnalyzesTo(cz, L"hezký", std::deque<wstring>{L"hezk"});
  assertAnalyzesTo(cz, L"hezčí", std::deque<wstring>{L"hezk"});

  /* rewrite of *ů* -> *o* */
  assertAnalyzesTo(cz, L"hůl", std::deque<wstring>{L"hol"});
  assertAnalyzesTo(cz, L"hole", std::deque<wstring>{L"hol"});

  /* rewrite of e* -> * */
  assertAnalyzesTo(cz, L"deska", std::deque<wstring>{L"desk"});
  assertAnalyzesTo(cz, L"desek", std::deque<wstring>{L"desk"});

  delete cz;
}

void TestCzechStemmer::testDontStem() 
{
  shared_ptr<CzechAnalyzer> cz = make_shared<CzechAnalyzer>();
  assertAnalyzesTo(cz, L"e", std::deque<wstring>{L"e"});
  assertAnalyzesTo(cz, L"zi", std::deque<wstring>{L"zi"});
  delete cz;
}

void TestCzechStemmer::testWithKeywordAttribute() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"hole");
  shared_ptr<MockTokenizer> *const in_ =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  in_->setReader(make_shared<StringReader>(L"hole desek"));
  shared_ptr<CzechStemFilter> filter = make_shared<CzechStemFilter>(
      make_shared<SetKeywordMarkerFilter>(in_, set));
  assertTokenStreamContents(filter, std::deque<wstring>{L"hole", L"desk"});
}

void TestCzechStemmer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestCzechStemmer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestCzechStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCzechStemmer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CzechStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::cz