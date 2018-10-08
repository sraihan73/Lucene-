using namespace std;

#include "TestTestFramework.h"

namespace org::apache::lucene::spatial
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Rectangle;
using SpatialArgsParser =
    org::apache::lucene::spatial::query::SpatialArgsParser;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Assert;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries() throws java.io.IOException
void TestTestFramework::testQueries() 
{
  wstring name = StrategyTestCase::QTEST_Cities_Intersects_BBox;

  shared_ptr<InputStream> in_ =
      getClass().getClassLoader().getResourceAsStream(name);
  shared_ptr<SpatialContext> ctx = SpatialContext::GEO;
  Iterator<std::shared_ptr<SpatialTestQuery>> iter =
      SpatialTestQuery::getTestQueries(make_shared<SpatialArgsParser>(), ctx,
                                       name, in_); // closes the InputStream
  deque<std::shared_ptr<SpatialTestQuery>> tests =
      deque<std::shared_ptr<SpatialTestQuery>>();
  while (iter->hasNext()) {
    tests.push_back(iter->next());
    iter++;
  }
  Assert::assertEquals(3, tests.size());

  shared_ptr<SpatialTestQuery> sf = tests[0];
  // assert
  assertEquals(1, sf->ids.size());
  Assert::assertTrue(sf->ids[0] == L"G5391959");
  Assert::assertTrue(
      std::dynamic_pointer_cast<Rectangle>(sf->args->getShape()) != nullptr);
  assertEquals(SpatialOperation::Intersects, sf->args->getOperation());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void spatialExample() throws Exception
void TestTestFramework::spatialExample() 
{
  // kind of a hack so that SpatialExample is tested despite
  // it not starting or ending with "Test".
  SpatialExample::main(nullptr);
}
} // namespace org::apache::lucene::spatial