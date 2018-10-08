using namespace std;

#include "TestBulgarianStemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/bg/BulgarianAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/bg/BulgarianStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::bg
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

void TestBulgarianStemmer::testMasculineNouns() 
{
  shared_ptr<BulgarianAnalyzer> a = make_shared<BulgarianAnalyzer>();

  // -и pattern
  assertAnalyzesTo(a, L"град", std::deque<wstring>{L"град"});
  assertAnalyzesTo(a, L"града", std::deque<wstring>{L"град"});
  assertAnalyzesTo(a, L"градът", std::deque<wstring>{L"град"});
  assertAnalyzesTo(a, L"градове", std::deque<wstring>{L"град"});
  assertAnalyzesTo(a, L"градовете", std::deque<wstring>{L"град"});

  // -ове pattern
  assertAnalyzesTo(a, L"народ", std::deque<wstring>{L"народ"});
  assertAnalyzesTo(a, L"народа", std::deque<wstring>{L"народ"});
  assertAnalyzesTo(a, L"народът", std::deque<wstring>{L"народ"});
  assertAnalyzesTo(a, L"народи", std::deque<wstring>{L"народ"});
  assertAnalyzesTo(a, L"народите", std::deque<wstring>{L"народ"});
  assertAnalyzesTo(a, L"народе", std::deque<wstring>{L"народ"});

  // -ища pattern
  assertAnalyzesTo(a, L"път", std::deque<wstring>{L"път"});
  assertAnalyzesTo(a, L"пътя", std::deque<wstring>{L"път"});
  assertAnalyzesTo(a, L"пътят", std::deque<wstring>{L"път"});
  assertAnalyzesTo(a, L"пътища", std::deque<wstring>{L"път"});
  assertAnalyzesTo(a, L"пътищата", std::deque<wstring>{L"път"});

  // -чета pattern
  assertAnalyzesTo(a, L"градец", std::deque<wstring>{L"градец"});
  assertAnalyzesTo(a, L"градеца", std::deque<wstring>{L"градец"});
  assertAnalyzesTo(a, L"градецът", std::deque<wstring>{L"градец"});
  /* note the below forms conflate with each other, but not the rest */
  assertAnalyzesTo(a, L"градовце", std::deque<wstring>{L"градовц"});
  assertAnalyzesTo(a, L"градовцете", std::deque<wstring>{L"градовц"});

  // -овци pattern
  assertAnalyzesTo(a, L"дядо", std::deque<wstring>{L"дяд"});
  assertAnalyzesTo(a, L"дядото", std::deque<wstring>{L"дяд"});
  assertAnalyzesTo(a, L"дядовци", std::deque<wstring>{L"дяд"});
  assertAnalyzesTo(a, L"дядовците", std::deque<wstring>{L"дяд"});

  // -е pattern
  assertAnalyzesTo(a, L"мъж", std::deque<wstring>{L"мъж"});
  assertAnalyzesTo(a, L"мъжа", std::deque<wstring>{L"мъж"});
  assertAnalyzesTo(a, L"мъже", std::deque<wstring>{L"мъж"});
  assertAnalyzesTo(a, L"мъжете", std::deque<wstring>{L"мъж"});
  assertAnalyzesTo(a, L"мъжо", std::deque<wstring>{L"мъж"});
  /* word is too short, will not remove -ът */
  assertAnalyzesTo(a, L"мъжът", std::deque<wstring>{L"мъжът"});

  // -а pattern
  assertAnalyzesTo(a, L"крак", std::deque<wstring>{L"крак"});
  assertAnalyzesTo(a, L"крака", std::deque<wstring>{L"крак"});
  assertAnalyzesTo(a, L"кракът", std::deque<wstring>{L"крак"});
  assertAnalyzesTo(a, L"краката", std::deque<wstring>{L"крак"});

  // брат
  assertAnalyzesTo(a, L"брат", std::deque<wstring>{L"брат"});
  assertAnalyzesTo(a, L"брата", std::deque<wstring>{L"брат"});
  assertAnalyzesTo(a, L"братът", std::deque<wstring>{L"брат"});
  assertAnalyzesTo(a, L"братя", std::deque<wstring>{L"брат"});
  assertAnalyzesTo(a, L"братята", std::deque<wstring>{L"брат"});
  assertAnalyzesTo(a, L"брате", std::deque<wstring>{L"брат"});

  delete a;
}

void TestBulgarianStemmer::testFeminineNouns() 
{
  shared_ptr<BulgarianAnalyzer> a = make_shared<BulgarianAnalyzer>();

  assertAnalyzesTo(a, L"вест", std::deque<wstring>{L"вест"});
  assertAnalyzesTo(a, L"вестта", std::deque<wstring>{L"вест"});
  assertAnalyzesTo(a, L"вести", std::deque<wstring>{L"вест"});
  assertAnalyzesTo(a, L"вестите", std::deque<wstring>{L"вест"});

  delete a;
}

void TestBulgarianStemmer::testNeuterNouns() 
{
  shared_ptr<BulgarianAnalyzer> a = make_shared<BulgarianAnalyzer>();

  // -а pattern
  assertAnalyzesTo(a, L"дърво", std::deque<wstring>{L"дърв"});
  assertAnalyzesTo(a, L"дървото", std::deque<wstring>{L"дърв"});
  assertAnalyzesTo(a, L"дърва", std::deque<wstring>{L"дърв"});
  assertAnalyzesTo(a, L"дървета", std::deque<wstring>{L"дърв"});
  assertAnalyzesTo(a, L"дървата", std::deque<wstring>{L"дърв"});
  assertAnalyzesTo(a, L"дърветата", std::deque<wstring>{L"дърв"});

  // -та pattern
  assertAnalyzesTo(a, L"море", std::deque<wstring>{L"мор"});
  assertAnalyzesTo(a, L"морето", std::deque<wstring>{L"мор"});
  assertAnalyzesTo(a, L"морета", std::deque<wstring>{L"мор"});
  assertAnalyzesTo(a, L"моретата", std::deque<wstring>{L"мор"});

  // -я pattern
  assertAnalyzesTo(a, L"изключение", std::deque<wstring>{L"изключени"});
  assertAnalyzesTo(a, L"изключението", std::deque<wstring>{L"изключени"});
  assertAnalyzesTo(a, L"изключенията", std::deque<wstring>{L"изключени"});
  /* note the below form in this example does not conflate with the rest */
  assertAnalyzesTo(a, L"изключения", std::deque<wstring>{L"изключн"});

  delete a;
}

void TestBulgarianStemmer::testAdjectives() 
{
  shared_ptr<BulgarianAnalyzer> a = make_shared<BulgarianAnalyzer>();
  assertAnalyzesTo(a, L"красив", std::deque<wstring>{L"красив"});
  assertAnalyzesTo(a, L"красивия", std::deque<wstring>{L"красив"});
  assertAnalyzesTo(a, L"красивият", std::deque<wstring>{L"красив"});
  assertAnalyzesTo(a, L"красива", std::deque<wstring>{L"красив"});
  assertAnalyzesTo(a, L"красивата", std::deque<wstring>{L"красив"});
  assertAnalyzesTo(a, L"красиво", std::deque<wstring>{L"красив"});
  assertAnalyzesTo(a, L"красивото", std::deque<wstring>{L"красив"});
  assertAnalyzesTo(a, L"красиви", std::deque<wstring>{L"красив"});
  assertAnalyzesTo(a, L"красивите", std::deque<wstring>{L"красив"});
  delete a;
}

void TestBulgarianStemmer::testExceptions() 
{
  shared_ptr<BulgarianAnalyzer> a = make_shared<BulgarianAnalyzer>();

  // ци -> к
  assertAnalyzesTo(a, L"собственик", std::deque<wstring>{L"собственик"});
  assertAnalyzesTo(a, L"собственика", std::deque<wstring>{L"собственик"});
  assertAnalyzesTo(a, L"собственикът", std::deque<wstring>{L"собственик"});
  assertAnalyzesTo(a, L"собственици", std::deque<wstring>{L"собственик"});
  assertAnalyzesTo(a, L"собствениците", std::deque<wstring>{L"собственик"});

  // зи -> г
  assertAnalyzesTo(a, L"подлог", std::deque<wstring>{L"подлог"});
  assertAnalyzesTo(a, L"подлога", std::deque<wstring>{L"подлог"});
  assertAnalyzesTo(a, L"подлогът", std::deque<wstring>{L"подлог"});
  assertAnalyzesTo(a, L"подлози", std::deque<wstring>{L"подлог"});
  assertAnalyzesTo(a, L"подлозите", std::deque<wstring>{L"подлог"});

  // си -> х
  assertAnalyzesTo(a, L"кожух", std::deque<wstring>{L"кожух"});
  assertAnalyzesTo(a, L"кожуха", std::deque<wstring>{L"кожух"});
  assertAnalyzesTo(a, L"кожухът", std::deque<wstring>{L"кожух"});
  assertAnalyzesTo(a, L"кожуси", std::deque<wstring>{L"кожух"});
  assertAnalyzesTo(a, L"кожусите", std::deque<wstring>{L"кожух"});

  // ъ deletion
  assertAnalyzesTo(a, L"център", std::deque<wstring>{L"центр"});
  assertAnalyzesTo(a, L"центъра", std::deque<wstring>{L"центр"});
  assertAnalyzesTo(a, L"центърът", std::deque<wstring>{L"центр"});
  assertAnalyzesTo(a, L"центрове", std::deque<wstring>{L"центр"});
  assertAnalyzesTo(a, L"центровете", std::deque<wstring>{L"центр"});

  // е*и -> я*
  assertAnalyzesTo(a, L"промяна", std::deque<wstring>{L"промян"});
  assertAnalyzesTo(a, L"промяната", std::deque<wstring>{L"промян"});
  assertAnalyzesTo(a, L"промени", std::deque<wstring>{L"промян"});
  assertAnalyzesTo(a, L"промените", std::deque<wstring>{L"промян"});

  // ен -> н
  assertAnalyzesTo(a, L"песен", std::deque<wstring>{L"песн"});
  assertAnalyzesTo(a, L"песента", std::deque<wstring>{L"песн"});
  assertAnalyzesTo(a, L"песни", std::deque<wstring>{L"песн"});
  assertAnalyzesTo(a, L"песните", std::deque<wstring>{L"песн"});

  // -еве -> й
  // note: this is the only word i think this rule works for.
  // most -еве pluralized nouns are monosyllabic,
  // and the stemmer requires length > 6...
  assertAnalyzesTo(a, L"строй", std::deque<wstring>{L"строй"});
  assertAnalyzesTo(a, L"строеве", std::deque<wstring>{L"строй"});
  assertAnalyzesTo(a, L"строевете", std::deque<wstring>{L"строй"});
  /* note the below forms conflate with each other, but not the rest */
  assertAnalyzesTo(a, L"строя", std::deque<wstring>{L"стр"});
  assertAnalyzesTo(a, L"строят", std::deque<wstring>{L"стр"});

  delete a;
}

void TestBulgarianStemmer::testWithKeywordAttribute() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"строеве");
  shared_ptr<MockTokenizer> tokenStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenStream->setReader(make_shared<StringReader>(L"строевете строеве"));

  shared_ptr<BulgarianStemFilter> filter = make_shared<BulgarianStemFilter>(
      make_shared<SetKeywordMarkerFilter>(tokenStream, set));
  assertTokenStreamContents(filter, std::deque<wstring>{L"строй", L"строеве"});
}

void TestBulgarianStemmer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestBulgarianStemmer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestBulgarianStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBulgarianStemmer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<BulgarianStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::bg