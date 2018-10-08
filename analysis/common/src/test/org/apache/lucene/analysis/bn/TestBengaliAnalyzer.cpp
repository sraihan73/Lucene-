using namespace std;

#include "TestBengaliAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/bn/BengaliAnalyzer.h"

namespace org::apache::lucene::analysis::bn
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

void TestBengaliAnalyzer::testResourcesAvailable()
{
  delete (make_shared<BengaliAnalyzer>());
}

void TestBengaliAnalyzer::testBasics() 
{
  shared_ptr<Analyzer> a = make_shared<BengaliAnalyzer>();

  checkOneTerm(a, L"বাড়ী", L"বার");
  checkOneTerm(a, L"বারী", L"বার");
  delete a;
}

void TestBengaliAnalyzer::testDigits() 
{
  shared_ptr<BengaliAnalyzer> a = make_shared<BengaliAnalyzer>();
  checkOneTerm(a, L"১২৩৪৫৬৭৮৯০", L"1234567890");
  delete a;
}

void TestBengaliAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<BengaliAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::bn