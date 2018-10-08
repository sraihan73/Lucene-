using namespace std;

#include "DateNRStrategyTest.h"

namespace org::apache::lucene::spatial::prefix
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using DateRangePrefixTree =
    org::apache::lucene::spatial::prefix::tree::DateRangePrefixTree;
using UnitNRShape = org::apache::lucene::spatial::prefix::tree::
    NumberRangePrefixTree::UnitNRShape;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::junit::Before;
using org::junit::Test;
using org::locationtech::spatial4j::shape::Shape;
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void setUp() throws Exception
void DateNRStrategyTest::setUp() 
{
  RandomSpatialOpStrategyTestCase::setUp();
  tree =
      org::apache::lucene::spatial::prefix::tree->DateRangePrefixTree::INSTANCE;
  strategy = make_shared<NumberRangePrefixTreeStrategy>(tree, L"dateRange");
  (std::static_pointer_cast<NumberRangePrefixTreeStrategy>(strategy))
      ->setPointsOnly(randomInt() % 5 == 0);
  shared_ptr<Calendar> tmpCal = tree->newCal();
  int randomCalWindowField =
      randomIntBetween(Calendar::YEAR, Calendar::MILLISECOND);
  tmpCal->add(randomCalWindowField, 2'000);
  randomCalWindowMs = max(2000LL, tmpCal->getTimeInMillis());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
// testIntersects() throws java.io.IOException
void DateNRStrategyTest::testIntersects() 
{
  testOperationRandomShapes(SpatialOperation::Intersects);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
// testWithin() throws java.io.IOException
void DateNRStrategyTest::testWithin() 
{
  testOperationRandomShapes(SpatialOperation::IsWithin);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
// testContains() throws java.io.IOException
void DateNRStrategyTest::testContains() 
{
  testOperationRandomShapes(SpatialOperation::Contains);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithinSame() throws java.io.IOException
void DateNRStrategyTest::testWithinSame() 
{
  shared_ptr<Shape> shape = randomIndexedShape();
  testOperation(shape, SpatialOperation::IsWithin, shape,
                true); // is within itself
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWorld() throws java.io.IOException
void DateNRStrategyTest::testWorld() 
{
  (std::static_pointer_cast<NumberRangePrefixTreeStrategy>(strategy))
      ->setPointsOnly(false);
  testOperation(tree->toShape(tree->newCal()), SpatialOperation::Contains,
                tree->toShape(randomCalendar()), true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBugInitIterOptimization() throws
// Exception
void DateNRStrategyTest::testBugInitIterOptimization() 
{
  (std::static_pointer_cast<NumberRangePrefixTreeStrategy>(strategy))
      ->setPointsOnly(false);
  // bug due to fast path initIter() optimization
  testOperation(tree->parseShape(L"[2014-03-27T23 TO 2014-04-01T01]"),
                SpatialOperation::Intersects,
                tree->parseShape(L"[2014-04 TO 2014-04-01T02]"), true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLastMillionYearPeriod() throws Exception
void DateNRStrategyTest::testLastMillionYearPeriod() 
{
  testOperation(tree->parseShape(L"+292220922-05-17T18:01:57.572"),
                SpatialOperation::Intersects, tree->parseShape(L"[1970 TO *]"),
                true);
}

shared_ptr<Shape> DateNRStrategyTest::randomIndexedShape()
{
  if ((std::static_pointer_cast<NumberRangePrefixTreeStrategy>(strategy))
          ->isPointsOnly()) {
    shared_ptr<Calendar> cal = tree->newCal();
    cal->setTimeInMillis(random()->nextLong());
    return tree->toShape(cal);
  } else {
    return randomShape();
  }
}

shared_ptr<Shape> DateNRStrategyTest::randomQueryShape()
{
  return randomShape();
}

shared_ptr<Shape> DateNRStrategyTest::randomShape()
{
  shared_ptr<Calendar> cal1 = randomCalendar();
  shared_ptr<UnitNRShape> s1 = tree->toShape(cal1);
  if (rarely()) {
    return s1;
  }
  try {
    shared_ptr<Calendar> cal2 = randomCalendar();
    shared_ptr<UnitNRShape> s2 = tree->toShape(cal2);
    if (cal1->compareTo(cal2) < 0) {
      return tree->toRangeShape(s1, s2);
    } else {
      return tree->toRangeShape(s2, s1);
    }
  } catch (const invalid_argument &e) {
    assert(e.what()->startsWith(L"Differing precision"));
    return s1;
  }
}

shared_ptr<Calendar> DateNRStrategyTest::randomCalendar()
{
  shared_ptr<Calendar> cal = tree->newCal();
  cal->setTimeInMillis(random()->nextLong() % randomCalWindowMs);
  try {
    tree->clearFieldsAfter(cal,
                           random()->nextInt(Calendar::FIELD_COUNT + 1) - 1);
  } catch (const AssertionError &e) {
    if (!e->getMessage().equals(L"Calendar underflow")) {
      throw e;
    }
  }
  return cal;
}
} // namespace org::apache::lucene::spatial::prefix