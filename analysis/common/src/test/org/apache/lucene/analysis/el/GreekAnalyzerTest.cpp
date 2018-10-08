using namespace std;

#include "GreekAnalyzerTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/el/GreekAnalyzer.h"

namespace org::apache::lucene::analysis::el
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

void GreekAnalyzerTest::testAnalyzer() 
{
  shared_ptr<Analyzer> a = make_shared<GreekAnalyzer>();
  // Verify the correct analysis of capitals and small accented letters, and
  // stemming
  assertAnalyzesTo(
      a,
      L"Μία εξαιρετικά καλή και πλούσια σειρά χαρακτήρων της Ελληνικής γλώσσας",
      std::deque<wstring>{L"μια", L"εξαιρετ", L"καλ", L"πλουσ", L"σειρ",
                           L"χαρακτηρ", L"ελληνικ", L"γλωσσ"});
  // Verify the correct analysis of small letters with diaeresis and the
  // elimination of punctuation marks
  assertAnalyzesTo(a, L"Προϊόντα (και)     [πολλαπλές] - ΑΝΑΓΚΕΣ",
                   std::deque<wstring>{L"προιοντ", L"πολλαπλ", L"αναγκ"});
  // Verify the correct analysis of capital accented letters and capital letters
  // with diaeresis, as well as the elimination of stop words
  assertAnalyzesTo(
      a, L"ΠΡΟΫΠΟΘΕΣΕΙΣ  Άψογος, ο μεστός και οι άλλοι",
      std::deque<wstring>{L"προυποθεσ", L"αψογ", L"μεστ", L"αλλ"});
  delete a;
}

void GreekAnalyzerTest::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<GreekAnalyzer>();
  // Verify the correct analysis of capitals and small accented letters, and
  // stemming
  assertAnalyzesTo(
      a,
      L"Μία εξαιρετικά καλή και πλούσια σειρά χαρακτήρων της Ελληνικής γλώσσας",
      std::deque<wstring>{L"μια", L"εξαιρετ", L"καλ", L"πλουσ", L"σειρ",
                           L"χαρακτηρ", L"ελληνικ", L"γλωσσ"});
  // Verify the correct analysis of small letters with diaeresis and the
  // elimination of punctuation marks
  assertAnalyzesTo(a, L"Προϊόντα (και)     [πολλαπλές] - ΑΝΑΓΚΕΣ",
                   std::deque<wstring>{L"προιοντ", L"πολλαπλ", L"αναγκ"});
  // Verify the correct analysis of capital accented letters and capital letters
  // with diaeresis, as well as the elimination of stop words
  assertAnalyzesTo(
      a, L"ΠΡΟΫΠΟΘΕΣΕΙΣ  Άψογος, ο μεστός και οι άλλοι",
      std::deque<wstring>{L"προυποθεσ", L"αψογ", L"μεστ", L"αλλ"});
  delete a;
}

void GreekAnalyzerTest::testRandomStrings() 
{
  shared_ptr<Analyzer> a = make_shared<GreekAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::el