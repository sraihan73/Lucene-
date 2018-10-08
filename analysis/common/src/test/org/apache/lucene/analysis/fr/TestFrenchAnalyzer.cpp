using namespace std;

#include "TestFrenchAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/fr/FrenchAnalyzer.h"

namespace org::apache::lucene::analysis::fr
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestFrenchAnalyzer::testAnalyzer() 
{
  shared_ptr<FrenchAnalyzer> fa = make_shared<FrenchAnalyzer>();

  assertAnalyzesTo(fa, L"", std::deque<wstring>());

  assertAnalyzesTo(fa, L"chien chat cheval",
                   std::deque<wstring>{L"chien", L"chat", L"cheval"});

  assertAnalyzesTo(fa, L"chien CHAT CHEVAL",
                   std::deque<wstring>{L"chien", L"chat", L"cheval"});

  assertAnalyzesTo(fa, L"  chien  ,? + = -  CHAT /: > CHEVAL",
                   std::deque<wstring>{L"chien", L"chat", L"cheval"});

  assertAnalyzesTo(fa, L"chien++", std::deque<wstring>{L"chien"});

  assertAnalyzesTo(fa, L"mot \"entreguillemet\"",
                   std::deque<wstring>{L"mot", L"entreguilemet"});

  // let's do some french specific tests now
  /* 1. couldn't resist
I would expect this to stay one term as in French the minus
sign is often used for composing words */
  assertAnalyzesTo(fa, L"Jean-François",
                   std::deque<wstring>{L"jean", L"francoi"});

  // 2. stopwords
  assertAnalyzesTo(fa, L"le la chien les aux chat du des à cheval",
                   std::deque<wstring>{L"chien", L"chat", L"cheval"});

  // some nouns and adjectives
  assertAnalyzesTo(fa, L"lances chismes habitable chiste éléments captifs",
                   std::deque<wstring>{L"lanc", L"chism", L"habitabl",
                                        L"chist", L"element", L"captif"});

  // some verbs
  assertAnalyzesTo(
      fa, L"finissions souffrirent rugissante",
      std::deque<wstring>{L"finision", L"soufrirent", L"rugisant"});

  // some everything else
  // aujourd'hui stays one term which is OK
  assertAnalyzesTo(
      fa, L"C3PO aujourd'hui oeuf ïâöûàä anticonstitutionnellement Java++ ",
      std::deque<wstring>{L"c3po", L"aujourd'hui", L"oeuf", L"ïaöuaä",
                           L"anticonstitutionel", L"java"});

  // some more everything else
  // here 1940-1945 stays as one term, 1940:1945 not ?
  assertAnalyzesTo(
      fa, L"33Bis 1940-1945 1940:1945 (---i+++)*",
      std::deque<wstring>{L"33bi", L"1940", L"1945", L"1940", L"1945", L"i"});
  delete fa;
}

void TestFrenchAnalyzer::testReusableTokenStream() 
{
  shared_ptr<FrenchAnalyzer> fa = make_shared<FrenchAnalyzer>();
  // stopwords
  assertAnalyzesTo(fa, L"le la chien les aux chat du des à cheval",
                   std::deque<wstring>{L"chien", L"chat", L"cheval"});

  // some nouns and adjectives
  assertAnalyzesTo(fa, L"lances chismes habitable chiste éléments captifs",
                   std::deque<wstring>{L"lanc", L"chism", L"habitabl",
                                        L"chist", L"element", L"captif"});
  delete fa;
}

void TestFrenchAnalyzer::testExclusionTableViaCtor() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"habitable");
  shared_ptr<FrenchAnalyzer> fa =
      make_shared<FrenchAnalyzer>(CharArraySet::EMPTY_SET, set);
  assertAnalyzesTo(fa, L"habitable chiste",
                   std::deque<wstring>{L"habitable", L"chist"});
  delete fa;

  fa = make_shared<FrenchAnalyzer>(CharArraySet::EMPTY_SET, set);
  assertAnalyzesTo(fa, L"habitable chiste",
                   std::deque<wstring>{L"habitable", L"chist"});
  delete fa;
}

void TestFrenchAnalyzer::testElision() 
{
  shared_ptr<FrenchAnalyzer> fa = make_shared<FrenchAnalyzer>();
  assertAnalyzesTo(fa, L"voir l'embrouille",
                   std::deque<wstring>{L"voir", L"embrouil"});
  delete fa;
}

void TestFrenchAnalyzer::testStopwordsCasing() 
{
  shared_ptr<FrenchAnalyzer> a = make_shared<FrenchAnalyzer>();
  assertAnalyzesTo(a, L"Votre", std::deque<wstring>());
  delete a;
}

void TestFrenchAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> a = make_shared<FrenchAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

void TestFrenchAnalyzer::testAccentInsensitive() 
{
  shared_ptr<Analyzer> a = make_shared<FrenchAnalyzer>();
  checkOneTerm(a, L"sécuritaires", L"securitair");
  checkOneTerm(a, L"securitaires", L"securitair");
  delete a;
}
} // namespace org::apache::lucene::analysis::fr