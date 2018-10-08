using namespace std;

#include "TestCollationKeyAnalyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../java/org/apache/lucene/collation/CollationKeyAnalyzer.h"

namespace org::apache::lucene::collation
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CollationTestBase = org::apache::lucene::analysis::CollationTestBase;
using BytesRef = org::apache::lucene::util::BytesRef;

void TestCollationKeyAnalyzer::setUp() 
{
  CollationTestBase::setUp();
  analyzer = make_shared<CollationKeyAnalyzer>(collator);
}

void TestCollationKeyAnalyzer::tearDown() 
{
  delete analyzer;
  CollationTestBase::tearDown();
}

void TestCollationKeyAnalyzer::testFarsiRangeFilterCollating() throw(
    runtime_error)
{
  testFarsiRangeFilterCollating(analyzer, firstRangeBeginning, firstRangeEnd,
                                secondRangeBeginning, secondRangeEnd);
}

void TestCollationKeyAnalyzer::testFarsiRangeQueryCollating() throw(
    runtime_error)
{
  testFarsiRangeQueryCollating(analyzer, firstRangeBeginning, firstRangeEnd,
                               secondRangeBeginning, secondRangeEnd);
}

void TestCollationKeyAnalyzer::testFarsiTermRangeQuery() 
{
  testFarsiTermRangeQuery(analyzer, firstRangeBeginning, firstRangeEnd,
                          secondRangeBeginning, secondRangeEnd);
}

void TestCollationKeyAnalyzer::testThreadSafe() 
{
  int iters = 20 * RANDOM_MULTIPLIER;
  for (int i = 0; i < iters; i++) {
    shared_ptr<Collator> collator = Collator::getInstance(Locale::GERMAN);
    collator->setStrength(Collator::PRIMARY);
    shared_ptr<Analyzer> analyzer = make_shared<CollationKeyAnalyzer>(collator);
    assertThreadSafe(analyzer);
    delete analyzer;
  }
}
} // namespace org::apache::lucene::collation