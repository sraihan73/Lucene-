using namespace std;

#include "TestDutchAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArrayMap.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/nl/DutchAnalyzer.h"

namespace org::apache::lucene::analysis::nl
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArrayMap = org::apache::lucene::analysis::CharArrayMap;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestDutchAnalyzer::testWithSnowballExamples() 
{
  check(L"lichaamsziek", L"lichaamsziek");
  check(L"lichamelijk", L"licham");
  check(L"lichamelijke", L"licham");
  check(L"lichamelijkheden", L"licham");
  check(L"lichamen", L"licham");
  check(L"lichere", L"licher");
  check(L"licht", L"licht");
  check(L"lichtbeeld", L"lichtbeeld");
  check(L"lichtbruin", L"lichtbruin");
  check(L"lichtdoorlatende", L"lichtdoorlat");
  check(L"lichte", L"licht");
  check(L"lichten", L"licht");
  check(L"lichtende", L"lichtend");
  check(L"lichtenvoorde", L"lichtenvoord");
  check(L"lichter", L"lichter");
  check(L"lichtere", L"lichter");
  check(L"lichters", L"lichter");
  check(L"lichtgevoeligheid", L"lichtgevoel");
  check(L"lichtgewicht", L"lichtgewicht");
  check(L"lichtgrijs", L"lichtgrijs");
  check(L"lichthoeveelheid", L"lichthoevel");
  check(L"lichtintensiteit", L"lichtintensiteit");
  check(L"lichtje", L"lichtj");
  check(L"lichtjes", L"lichtjes");
  check(L"lichtkranten", L"lichtkrant");
  check(L"lichtkring", L"lichtkring");
  check(L"lichtkringen", L"lichtkring");
  check(L"lichtregelsystemen", L"lichtregelsystem");
  check(L"lichtste", L"lichtst");
  check(L"lichtstromende", L"lichtstrom");
  check(L"lichtte", L"licht");
  check(L"lichtten", L"licht");
  check(L"lichttoetreding", L"lichttoetred");
  check(L"lichtverontreinigde", L"lichtverontreinigd");
  check(L"lichtzinnige", L"lichtzinn");
  check(L"lid", L"lid");
  check(L"lidia", L"lidia");
  check(L"lidmaatschap", L"lidmaatschap");
  check(L"lidstaten", L"lidstat");
  check(L"lidvereniging", L"lidveren");
  check(L"opgingen", L"opging");
  check(L"opglanzing", L"opglanz");
  check(L"opglanzingen", L"opglanz");
  check(L"opglimlachten", L"opglimlacht");
  check(L"opglimpen", L"opglimp");
  check(L"opglimpende", L"opglimp");
  check(L"opglimping", L"opglimp");
  check(L"opglimpingen", L"opglimp");
  check(L"opgraven", L"opgrav");
  check(L"opgrijnzen", L"opgrijnz");
  check(L"opgrijzende", L"opgrijz");
  check(L"opgroeien", L"opgroei");
  check(L"opgroeiende", L"opgroei");
  check(L"opgroeiplaats", L"opgroeiplat");
  check(L"ophaal", L"ophal");
  check(L"ophaaldienst", L"ophaaldienst");
  check(L"ophaalkosten", L"ophaalkost");
  check(L"ophaalsystemen", L"ophaalsystem");
  check(L"ophaalt", L"ophaalt");
  check(L"ophaaltruck", L"ophaaltruck");
  check(L"ophalen", L"ophal");
  check(L"ophalend", L"ophal");
  check(L"ophalers", L"ophaler");
  check(L"ophef", L"ophef");
  check(L"opheldering", L"ophelder");
  check(L"ophemelde", L"ophemeld");
  check(L"ophemelen", L"ophemel");
  check(L"opheusden", L"opheusd");
  check(L"ophief", L"ophief");
  check(L"ophield", L"ophield");
  check(L"ophieven", L"ophiev");
  check(L"ophoepelt", L"ophoepelt");
  check(L"ophoog", L"ophog");
  check(L"ophoogzand", L"ophoogzand");
  check(L"ophopen", L"ophop");
  check(L"ophoping", L"ophop");
  check(L"ophouden", L"ophoud");
}

void TestDutchAnalyzer::testSnowballCorrectness() 
{
  shared_ptr<Analyzer> a = make_shared<DutchAnalyzer>();
  checkOneTerm(a, L"opheffen", L"opheff");
  checkOneTerm(a, L"opheffende", L"opheff");
  checkOneTerm(a, L"opheffing", L"opheff");
  delete a;
}

void TestDutchAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<DutchAnalyzer>();
  checkOneTerm(a, L"lichaamsziek", L"lichaamsziek");
  checkOneTerm(a, L"lichamelijk", L"licham");
  checkOneTerm(a, L"lichamelijke", L"licham");
  checkOneTerm(a, L"lichamelijkheden", L"licham");
  delete a;
}

void TestDutchAnalyzer::testExclusionTableViaCtor() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"lichamelijk");
  shared_ptr<DutchAnalyzer> a =
      make_shared<DutchAnalyzer>(CharArraySet::EMPTY_SET, set);
  assertAnalyzesTo(a, L"lichamelijk lichamelijke",
                   std::deque<wstring>{L"lichamelijk", L"licham"});
  delete a;

  a = make_shared<DutchAnalyzer>(CharArraySet::EMPTY_SET, set);
  assertAnalyzesTo(a, L"lichamelijk lichamelijke",
                   std::deque<wstring>{L"lichamelijk", L"licham"});
  delete a;
}

void TestDutchAnalyzer::testStemOverrides() 
{
  shared_ptr<DutchAnalyzer> a =
      make_shared<DutchAnalyzer>(CharArraySet::EMPTY_SET);
  checkOneTerm(a, L"fiets", L"fiets");
  delete a;
}

void TestDutchAnalyzer::testEmptyStemDictionary() 
{
  shared_ptr<DutchAnalyzer> a = make_shared<DutchAnalyzer>(
      CharArraySet::EMPTY_SET, CharArraySet::EMPTY_SET,
      CharArrayMap::emptyMap<wstring>());
  checkOneTerm(a, L"fiets", L"fiet");
  delete a;
}

void TestDutchAnalyzer::testStopwordsCasing() 
{
  shared_ptr<DutchAnalyzer> a = make_shared<DutchAnalyzer>();
  assertAnalyzesTo(a, L"Zelf", std::deque<wstring>());
  delete a;
}

void TestDutchAnalyzer::check(const wstring &input,
                              const wstring &expected) 
{
  shared_ptr<Analyzer> analyzer = make_shared<DutchAnalyzer>();
  checkOneTerm(analyzer, input, expected);
  delete analyzer;
}

void TestDutchAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<DutchAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::nl