using namespace std;

#include "TestArabicAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/ar/ArabicAnalyzer.h"

namespace org::apache::lucene::analysis::ar
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

void TestArabicAnalyzer::testResourcesAvailable()
{
  delete (make_shared<ArabicAnalyzer>());
}

void TestArabicAnalyzer::testBasicFeatures() 
{
  shared_ptr<ArabicAnalyzer> a = make_shared<ArabicAnalyzer>();
  assertAnalyzesTo(a, L"كبير", std::deque<wstring>{L"كبير"});
  assertAnalyzesTo(a, L"كبيرة",
                   std::deque<wstring>{L"كبير"}); // feminine marker

  assertAnalyzesTo(a, L"مشروب", std::deque<wstring>{L"مشروب"});
  assertAnalyzesTo(a, L"مشروبات", std::deque<wstring>{L"مشروب"}); // plural -at

  assertAnalyzesTo(a, L"أمريكيين",
                   std::deque<wstring>{L"امريك"}); // plural -in
  assertAnalyzesTo(a, L"امريكي",
                   std::deque<wstring>{L"امريك"}); // singular with bare alif

  assertAnalyzesTo(a, L"كتاب", std::deque<wstring>{L"كتاب"});
  assertAnalyzesTo(a, L"الكتاب",
                   std::deque<wstring>{L"كتاب"}); // definite article

  assertAnalyzesTo(a, L"ما ملكت أيمانكم",
                   std::deque<wstring>{L"ملكت", L"ايمانكم"});
  assertAnalyzesTo(a, L"الذين ملكت أيمانكم",
                   std::deque<wstring>{L"ملكت", L"ايمانكم"}); // stopwords
  delete a;
}

void TestArabicAnalyzer::testReusableTokenStream() 
{
  shared_ptr<ArabicAnalyzer> a = make_shared<ArabicAnalyzer>();
  assertAnalyzesTo(a, L"كبير", std::deque<wstring>{L"كبير"});
  assertAnalyzesTo(a, L"كبيرة",
                   std::deque<wstring>{L"كبير"}); // feminine marker
  delete a;
}

void TestArabicAnalyzer::testEnglishInput() 
{
  shared_ptr<ArabicAnalyzer> a = make_shared<ArabicAnalyzer>();
  assertAnalyzesTo(a, L"English text.",
                   std::deque<wstring>{L"english", L"text"});
  delete a;
}

void TestArabicAnalyzer::testCustomStopwords() 
{
  shared_ptr<CharArraySet> set =
      make_shared<CharArraySet>(asSet({L"the", L"and", L"a"}), false);
  shared_ptr<ArabicAnalyzer> a = make_shared<ArabicAnalyzer>(set);
  assertAnalyzesTo(a, L"The quick brown fox.",
                   std::deque<wstring>{L"quick", L"brown", L"fox"});
  delete a;
}

void TestArabicAnalyzer::testWithStemExclusionSet() 
{
  shared_ptr<CharArraySet> set =
      make_shared<CharArraySet>(asSet({L"ساهدهات"}), false);
  shared_ptr<ArabicAnalyzer> a =
      make_shared<ArabicAnalyzer>(CharArraySet::EMPTY_SET, set);
  assertAnalyzesTo(a, L"كبيرة the quick ساهدهات",
                   std::deque<wstring>{L"كبير", L"the", L"quick", L"ساهدهات"});
  assertAnalyzesTo(a, L"كبيرة the quick ساهدهات",
                   std::deque<wstring>{L"كبير", L"the", L"quick", L"ساهدهات"});
  delete a;

  a = make_shared<ArabicAnalyzer>(CharArraySet::EMPTY_SET,
                                  CharArraySet::EMPTY_SET);
  assertAnalyzesTo(a, L"كبيرة the quick ساهدهات",
                   std::deque<wstring>{L"كبير", L"the", L"quick", L"ساهد"});
  assertAnalyzesTo(a, L"كبيرة the quick ساهدهات",
                   std::deque<wstring>{L"كبير", L"the", L"quick", L"ساهد"});
  delete a;
}

void TestArabicAnalyzer::testDigits() 
{
  shared_ptr<ArabicAnalyzer> a = make_shared<ArabicAnalyzer>();
  checkOneTerm(a, L"١٢٣٤", L"1234");
  delete a;
}

void TestArabicAnalyzer::testRandomStrings() 
{
  shared_ptr<ArabicAnalyzer> a = make_shared<ArabicAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::ar