using namespace std;

#include "TestICUCollationKeyAnalyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../java/org/apache/lucene/collation/ICUCollationKeyAnalyzer.h"

namespace org::apache::lucene::collation
{
using com::ibm::icu::text::Collator;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CollationTestBase = org::apache::lucene::analysis::CollationTestBase;
using BytesRef = org::apache::lucene::util::BytesRef;

void TestICUCollationKeyAnalyzer::setUp() 
{
  CollationTestBase::setUp();
  analyzer = make_shared<ICUCollationKeyAnalyzer>(collator);
}

void TestICUCollationKeyAnalyzer::tearDown() 
{
  delete analyzer;
  CollationTestBase::tearDown();
}

void TestICUCollationKeyAnalyzer::testFarsiRangeFilterCollating() throw(
    runtime_error)
{
  testFarsiRangeFilterCollating(analyzer, firstRangeBeginning, firstRangeEnd,
                                secondRangeBeginning, secondRangeEnd);
}

void TestICUCollationKeyAnalyzer::testFarsiRangeQueryCollating() throw(
    runtime_error)
{
  testFarsiRangeQueryCollating(analyzer, firstRangeBeginning, firstRangeEnd,
                               secondRangeBeginning, secondRangeEnd);
}

void TestICUCollationKeyAnalyzer::testFarsiTermRangeQuery() 
{
  testFarsiTermRangeQuery(analyzer, firstRangeBeginning, firstRangeEnd,
                          secondRangeBeginning, secondRangeEnd);
}

void TestICUCollationKeyAnalyzer::testThreadSafe() 
{
  int iters = 20 * RANDOM_MULTIPLIER;
  for (int i = 0; i < iters; i++) {
    shared_ptr<Locale> locale = Locale::GERMAN;
    shared_ptr<Collator> collator = Collator::getInstance(locale);
    collator->setStrength(Collator::IDENTICAL);
    shared_ptr<Analyzer> a = make_shared<ICUCollationKeyAnalyzer>(collator);
    assertThreadSafe(a);
    delete a;
  }
}
} // namespace org::apache::lucene::collation