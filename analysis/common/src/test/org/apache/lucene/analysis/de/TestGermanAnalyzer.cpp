using namespace std;

#include "TestGermanAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/de/GermanAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/de/GermanStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::de
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseTokenizer =
    org::apache::lucene::analysis::core::LowerCaseTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;

void TestGermanAnalyzer::testReusableTokenStream() 
{
  shared_ptr<Analyzer> a = make_shared<GermanAnalyzer>();
  checkOneTerm(a, L"Tisch", L"tisch");
  checkOneTerm(a, L"Tische", L"tisch");
  checkOneTerm(a, L"Tischen", L"tisch");
  delete a;
}

void TestGermanAnalyzer::testWithKeywordAttribute() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"fischen");
  shared_ptr<LowerCaseTokenizer> *const in_ = make_shared<LowerCaseTokenizer>();
  in_->setReader(make_shared<StringReader>(L"Fischen Trinken"));
  shared_ptr<GermanStemFilter> filter = make_shared<GermanStemFilter>(
      make_shared<SetKeywordMarkerFilter>(in_, set));
  assertTokenStreamContents(filter, std::deque<wstring>{L"fischen", L"trink"});
}

void TestGermanAnalyzer::testStemExclusionTable() 
{
  shared_ptr<GermanAnalyzer> a = make_shared<GermanAnalyzer>(
      CharArraySet::EMPTY_SET,
      make_shared<CharArraySet>(asSet({L"tischen"}), false));
  checkOneTerm(a, L"tischen", L"tischen");
  delete a;
}

void TestGermanAnalyzer::testGermanSpecials() 
{
  shared_ptr<GermanAnalyzer> a = make_shared<GermanAnalyzer>();
  // a/o/u + e is equivalent to the umlaut form
  checkOneTerm(a, L"Schaltflächen", L"schaltflach");
  checkOneTerm(a, L"Schaltflaechen", L"schaltflach");
  delete a;
}

void TestGermanAnalyzer::testRandomStrings() 
{
  shared_ptr<GermanAnalyzer> a = make_shared<GermanAnalyzer>();
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}
} // namespace org::apache::lucene::analysis::de