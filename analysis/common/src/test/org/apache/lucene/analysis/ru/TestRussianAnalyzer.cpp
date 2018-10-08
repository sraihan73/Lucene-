using namespace std;

#include "TestRussianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/ru/RussianAnalyzer.h"

namespace org::apache::lucene::analysis::ru
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestRussianAnalyzer::testDigitsInRussianCharset() 
{
  shared_ptr<RussianAnalyzer> ra = make_shared<RussianAnalyzer>();
  assertAnalyzesTo(ra, L"text 1000", std::deque<wstring>{L"text", L"1000"});
  delete ra;
}

void TestRussianAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<RussianAnalyzer>();
  assertAnalyzesTo(
      a,
      L"Вместе с тем о силе электромагнитной энергии имели представление еще",
      std::deque<wstring>{L"вмест", L"сил", L"электромагнитн", L"энерг",
                           L"имел", L"представлен"});
  assertAnalyzesTo(a, L"Но знание это хранилось в тайне",
                   std::deque<wstring>{L"знан", L"эт", L"хран", L"тайн"});
  delete a;
}

void TestRussianAnalyzer::testWithStemExclusionSet() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"представление");
  shared_ptr<Analyzer> a =
      make_shared<RussianAnalyzer>(RussianAnalyzer::getDefaultStopSet(), set);
  assertAnalyzesTo(
      a,
      L"Вместе с тем о силе электромагнитной энергии имели представление еще",
      std::deque<wstring>{L"вмест", L"сил", L"электромагнитн", L"энерг",
                           L"имел", L"представление"});
  delete a;
}

void TestRussianAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<RussianAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::ru