using namespace std;

#include "DateRangePrefixTreeTest.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using com::carrotsearch::randomizedtesting::annotations::ParametersFactory;
using UnitNRShape = org::apache::lucene::spatial::prefix::tree::
    NumberRangePrefixTree::UnitNRShape;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @ParametersFactory(argumentFormatting = "calendar=%s") public
// static Iterable<Object[]> parameters()
deque<std::deque<any>> DateRangePrefixTreeTest::parameters()
{
  return Arrays::asList(std::deque<std::deque<any>>{
      std::deque<any>{L"default", DateRangePrefixTree::DEFAULT_CAL},
      std::deque<any>{L"compat",
                       DateRangePrefixTree::JAVA_UTIL_TIME_COMPAT_CAL}});
}

DateRangePrefixTreeTest::DateRangePrefixTreeTest(
    const wstring &suiteName, shared_ptr<Calendar> templateCal)
    : tree(make_shared<DateRangePrefixTree>(templateCal))
{
}

void DateRangePrefixTreeTest::testRoundTrip() 
{
  shared_ptr<Calendar> cal = tree->newCal();

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"*", tree->toString(cal));

  // test no underflow
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(
      tree->toShape(std::deque<int>{0}, 1)->toString()->startsWith(L"-"));

  // Some arbitrary date
  cal->set(2014, Calendar::MAY, 9);
  roundTrip(cal);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"2014-05-09", tree->toString(cal));

  // Earliest date
  cal->setTimeInMillis(numeric_limits<int64_t>::min());
  roundTrip(cal);

  // Farthest date
  cal->setTimeInMillis(numeric_limits<int64_t>::max());
  roundTrip(cal);

  // 1BC is "0000".
  cal->clear();
  cal->set(Calendar::ERA, GregorianCalendar::BC);
  cal->set(Calendar::YEAR, 1);
  roundTrip(cal);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"0000", tree->toString(cal));
  // adding a "+" parses to the same; and a trailing 'Z' is fine too
  assertEquals(cal, tree->parseCalendar(L"+0000Z"));

  // 2BC is "-0001"
  cal->clear();
  cal->set(Calendar::ERA, GregorianCalendar::BC);
  cal->set(Calendar::YEAR, 2);
  roundTrip(cal);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"-0001", tree->toString(cal));

  // 1AD is "0001"
  cal->clear();
  cal->set(Calendar::YEAR, 1);
  roundTrip(cal);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"0001", tree->toString(cal));

  // test random
  cal->setTimeInMillis(random()->nextLong());
  roundTrip(cal);
}

void DateRangePrefixTreeTest::testToStringISO8601()
{
  shared_ptr<Calendar> cal = tree->newCal();
  cal->setTimeInMillis(random()->nextLong());
  //  create ZonedDateTime from the calendar, then get toInstant.toString which
  //  is the ISO8601 we emulate
  //   note: we don't simply init off of millisEpoch because of possible
  //   GregorianChangeDate discrepancy.
  int year = cal->get(Calendar::YEAR);
  if (cal->get(Calendar::ERA) == 0) { // BC
    year = -year + 1;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring expectedISO8601 =
      ZonedDateTime::of(
          year, cal->get(Calendar::MONTH) + 1, cal->get(Calendar::DAY_OF_MONTH),
          cal->get(Calendar::HOUR_OF_DAY), cal->get(Calendar::MINUTE),
          cal->get(Calendar::SECOND),
          cal->get(Calendar::MILLISECOND) * 1'000'000, ZoneOffset::UTC)
          .toInstant()
          ->toString();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring resultToString = tree->toString(cal) + StringHelper::toString(L'Z');
  assertEquals(expectedISO8601, resultToString);
}

std::deque<int> const DateRangePrefixTreeTest::CAL_FIELDS = {
    java::util::Calendar::YEAR,         java::util::Calendar::MONTH,
    java::util::Calendar::DAY_OF_MONTH, java::util::Calendar::HOUR_OF_DAY,
    java::util::Calendar::MINUTE,       java::util::Calendar::SECOND,
    java::util::Calendar::MILLISECOND};

void DateRangePrefixTreeTest::roundTrip(shared_ptr<Calendar> calOrig) throw(
    ParseException)
{
  shared_ptr<Calendar> cal =
      std::static_pointer_cast<Calendar>(calOrig->clone());
  wstring lastString = L"";
  while (true) {
    wstring calString;
    {
      shared_ptr<Calendar> preToStringCalClone =
          std::static_pointer_cast<Calendar>(cal->clone());
      // C++ TODO: There is no native C++ equivalent to 'toString':
      calString = tree->toString(cal);
      assertEquals(preToStringCalClone,
                   cal); // ensure toString doesn't modify cal state
    }

    // test parseCalendar
    assertEquals(cal, tree->parseCalendar(calString));

    // to Shape and back to Cal
    shared_ptr<UnitNRShape> shape = tree->toShape(cal);
    shared_ptr<Calendar> cal2 = tree->toCalendar(shape);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(calString, tree->toString(cal2));

    if (calString != L"*") { // not world cell
      // to Term and back to Cell
      shared_ptr<Cell> cell = std::static_pointer_cast<Cell>(shape);
      shared_ptr<BytesRef> term = cell->getTokenBytesNoLeaf(nullptr);
      shared_ptr<Cell> cell2 =
          tree->readCell(BytesRef::deepCopyOf(term), nullptr);
      assertEquals(calString, cell, cell2);
      shared_ptr<Calendar> cal3 = tree->toCalendar(
          std::static_pointer_cast<UnitNRShape>(cell2->getShape()));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertEquals(calString, tree->toString(cal3));

      // setLeaf comparison
      cell2->setLeaf();
      shared_ptr<BytesRef> termLeaf = cell2->getTokenBytesWithLeaf(nullptr);
      assertTrue(term->compareTo(termLeaf) < 0);
      assertEquals(termLeaf->length, term->length + 1);
      assertEquals(0, termLeaf->bytes[termLeaf->offset + termLeaf->length - 1]);
      assertTrue(cell->isPrefixOf(cell2));
    }

    // end of loop; decide if should loop again with lower precision
    constexpr int calPrecField = tree->getCalPrecisionField(cal);
    if (calPrecField == -1) {
      break;
    }
    int fieldIdx = Arrays::binarySearch(CAL_FIELDS, calPrecField);
    assert(fieldIdx >= 0);
    int prevPrecField = (fieldIdx == 0 ? -1 : CAL_FIELDS[--fieldIdx]);
    try {
      tree->clearFieldsAfter(cal, prevPrecField);
    } catch (const AssertionError &e) {
      if (e->getMessage().equals(L"Calendar underflow")) {
        return;
      }
      throw e;
    }
    lastString = calString;
  }
}

void DateRangePrefixTreeTest::testShapeRelations() 
{
  // note: left range is 264000 at the thousand year level whereas right value
  // is exact year
  assertEquals(SpatialRelation::WITHIN,
               tree->parseShape(L"[-264000 TO -264000-11-20]")
                   .relate(tree->parseShape(L"-264000")));

  shared_ptr<Shape> shapeA = tree->parseShape(L"[3122-01-23 TO 3122-11-27]");
  shared_ptr<Shape> shapeB = tree->parseShape(L"[3122-08 TO 3122-11]");
  assertEquals(SpatialRelation::INTERSECTS, shapeA->relate(shapeB));

  shapeA = tree->parseShape(L"3122");
  shapeB = tree->parseShape(L"[* TO 3122-10-31]");
  assertEquals(SpatialRelation::INTERSECTS, shapeA->relate(shapeB));

  shapeA = tree->parseShape(L"[3122-05-28 TO 3122-06-29]");
  shapeB = tree->parseShape(L"[3122 TO 3122-04]");
  assertEquals(SpatialRelation::DISJOINT, shapeA->relate(shapeB));
}

void DateRangePrefixTreeTest::testShapeRangeOptimizer() 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[2014-08 TO 2014-09]",
               tree->parseShape(L"[2014-08-01 TO 2014-09-30]")->toString());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"2014",
               tree->parseShape(L"[2014-01-01 TO 2014-12-31]")->toString());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"2014", tree->parseShape(L"[2014-01 TO 2014]")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"2014-01", tree->parseShape(L"[2014 TO 2014-01]")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"2014-12", tree->parseShape(L"[2014-12 TO 2014]")->toString());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[2014 TO 2014-04-06]",
               tree->parseShape(L"[2014-01 TO 2014-04-06]")->toString());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"*", tree->parseShape(L"[* TO *]")->toString());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"2014-08-01",
               tree->parseShape(L"[2014-08-01 TO 2014-08-01]")->toString());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[2014 TO 2014-09-15]",
               tree->parseShape(L"[2014 TO 2014-09-15]")->toString());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[* TO 2014-09-15]",
               tree->parseShape(L"[* TO 2014-09-15]")->toString());
}
} // namespace org::apache::lucene::spatial::prefix::tree