using namespace std;

#include "DateRangePrefixTree.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::shape::Shape;
const shared_ptr<java::util::TimeZone> DateRangePrefixTree::UTC =
    java::util::TimeZone::getTimeZone(L"UTC");
const shared_ptr<java::util::Calendar> DateRangePrefixTree::DEFAULT_CAL;

DateRangePrefixTree::StaticConstructor::StaticConstructor()
{
  DEFAULT_CAL = Calendar::getInstance(UTC, Locale::ROOT);
  DEFAULT_CAL->clear();
  // see source of GregorianCalendar.from(ZonedDateTime)
  shared_ptr<GregorianCalendar> cal =
      make_shared<GregorianCalendar>(UTC, Locale::ROOT);
  cal->setGregorianChange(Date(numeric_limits<int64_t>::min()));
  cal->setFirstDayOfWeek(Calendar::MONDAY); // might not matter?
  cal->setMinimalDaysInFirstWeek(4);        // might not matter
  cal->clear();
  JAVA_UTIL_TIME_COMPAT_CAL = cal;
}

DateRangePrefixTree::StaticConstructor DateRangePrefixTree::staticConstructor;
const shared_ptr<java::util::Calendar>
    DateRangePrefixTree::JAVA_UTIL_TIME_COMPAT_CAL;
std::deque<int> const DateRangePrefixTree::FIELD_BY_LEVEL = {
    -1,
    -1,
    -1,
    java::util::Calendar::YEAR,
    java::util::Calendar::MONTH,
    java::util::Calendar::DAY_OF_MONTH,
    java::util::Calendar::HOUR_OF_DAY,
    java::util::Calendar::MINUTE,
    java::util::Calendar::SECOND,
    java::util::Calendar::MILLISECOND};
const shared_ptr<DateRangePrefixTree> DateRangePrefixTree::INSTANCE =
    make_shared<DateRangePrefixTree>(DEFAULT_CAL);

DateRangePrefixTree::DateRangePrefixTree(shared_ptr<Calendar> templateCal)
    : NumberRangePrefixTree(new int[]{
          NUM_MYEARS, 1000, 1000, calFieldLen(templateCal, Calendar::MONTH),
          calFieldLen(templateCal, Calendar::DAY_OF_MONTH),
          calFieldLen(templateCal, Calendar::HOUR_OF_DAY),
          calFieldLen(templateCal, Calendar::MINUTE),
          calFieldLen(templateCal, Calendar::SECOND),
          calFieldLen(templateCal, Calendar::MILLISECOND)}),
      CAL_TMP(std::static_pointer_cast<Calendar>(templateCal->clone())) /
          *defensive copy * /
      , MINCAL(std::static_pointer_cast<Calendar>(CAL_TMP->clone())),
      MAXCAL(std::static_pointer_cast<Calendar>(CAL_TMP->clone())),
      BC_FIRSTYEAR(MINCAL->getActualMaximum(Calendar::YEAR)),
      BC_LASTYEAR(MINCAL->getActualMinimum(Calendar::YEAR)) / *1 * /,
      BC_YEARS(BC_FIRSTYEAR - BC_LASTYEAR + 1),
      AD_FIRSTYEAR(MAXCAL->getActualMinimum(Calendar::YEAR)) / *1 * /,
      AD_LASTYEAR(MAXCAL->getActualMaximum(Calendar::YEAR)),
      AD_YEAR_BASE((((BC_YEARS - 1) / 1000'000) + 1) * 1000'000) /
          *align year 0 at an even #of million years * /
      , minLV(toShape(std::static_pointer_cast<Calendar>(MINCAL->clone()))),
      maxLV(toShape(std::static_pointer_cast<Calendar>(MAXCAL->clone())))
{
  MINCAL->setTimeInMillis(numeric_limits<int64_t>::min());
  MAXCAL->setTimeInMillis(numeric_limits<int64_t>::max());
  // BC years are decreasing, remember.  Yet ActualMaximum is the numerically
  // high value, ActualMinimum is 1.
  assert(BC_LASTYEAR == 1 && AD_FIRSTYEAR == 1);
  assert(NUM_MYEARS == (AD_YEAR_BASE + AD_LASTYEAR) / 1000'000 + 1);

  if (std::dynamic_pointer_cast<GregorianCalendar>(MAXCAL) != nullptr) {
    shared_ptr<GregorianCalendar> gCal =
        std::static_pointer_cast<GregorianCalendar>(MAXCAL);
    gregorianChangeDateLV = toUnitShape(gCal->getGregorianChange());
  } else {
    gregorianChangeDateLV.reset();
  }
}

int DateRangePrefixTree::calFieldLen(shared_ptr<Calendar> cal, int field)
{
  return cal->getMaximum(field) - cal->getMinimum(field) + 1;
}

int DateRangePrefixTree::getNumSubCells(shared_ptr<UnitNRShape> lv)
{
  int cmp = comparePrefix(lv, maxLV);
  assert(cmp <= 0);
  if (cmp == 0) // edge case (literally!)
  {
    return maxLV->getValAtLevel(lv->getLevel() + 1) + 1;
  }

  // if using GregorianCalendar and we're after the "Gregorian change date" then
  // we'll compute
  //  the sub-cells ourselves more efficiently without the need to construct a
  //  Calendar.
  cmp = gregorianChangeDateLV != nullptr
            ? comparePrefix(lv, gregorianChangeDateLV)
            : -1;
  // TODO consider also doing fast-path if field is <= hours even if before greg
  // change date
  if (cmp >= 0) {
    int result = fastSubCells(lv);
    assert(
        (result == slowSubCells(lv), L"fast/slow numSubCells inconsistency"));
    return result;
  } else {
    return slowSubCells(lv);
  }
}

int DateRangePrefixTree::fastSubCells(shared_ptr<UnitNRShape> lv)
{
  if (lv->getLevel() == YEAR_LEVEL + 1) { // month
    switch (lv->getValAtLevel(lv->getLevel())) {
    case Calendar::SEPTEMBER:
    case Calendar::APRIL:
    case Calendar::JUNE:
    case Calendar::NOVEMBER:
      return 30;
    case Calendar::FEBRUARY: {
      // get the year (negative numbers for BC)
      int yearAdj = lv->getValAtLevel(1) * 1'000'000;
      yearAdj += lv->getValAtLevel(2) * 1000;
      yearAdj += lv->getValAtLevel(3);
      int year = yearAdj - AD_YEAR_BASE;
      if (year % 4 == 0 && !(year % 100 == 0 && year % 400 != 0)) // leap year
      {
        return 29;
      } else {
        return 28;
      }
    }
    default:
      return 31;
    }
  } else { // typical:
    return NumberRangePrefixTree::getNumSubCells(lv);
  }
}

int DateRangePrefixTree::slowSubCells(shared_ptr<UnitNRShape> lv)
{
  int field = FIELD_BY_LEVEL[lv->getLevel() + 1];
  // short-circuit optimization (GregorianCalendar assumptions)
  if (field == -1 || field == Calendar::YEAR ||
      field >= Calendar::HOUR_OF_DAY) // TODO make configurable
  {
    return NumberRangePrefixTree::getNumSubCells(lv);
  }
  shared_ptr<Calendar> cal =
      toCalendar(lv); // somewhat heavyweight op; ideally should be stored on
                      // UnitNRShape somehow
  return cal->getActualMaximum(field) - cal->getActualMinimum(field) + 1;
}

shared_ptr<Calendar> DateRangePrefixTree::newCal()
{
  return std::static_pointer_cast<Calendar>(CAL_TMP->clone());
}

int DateRangePrefixTree::getTreeLevelForCalendarField(int calField)
{
  for (int i = YEAR_LEVEL; i < FIELD_BY_LEVEL.size(); i++) {
    if (FIELD_BY_LEVEL[i] == calField) {
      return i;
    } else if (FIELD_BY_LEVEL[i] > calField) {
      return -1 * i;
    }
  }
  throw invalid_argument(L"Bad calendar field?: " + to_wstring(calField));
}

int DateRangePrefixTree::getCalPrecisionField(shared_ptr<Calendar> cal)
{
  int lastField = -1;
  for (int level = YEAR_LEVEL; level < FIELD_BY_LEVEL.size(); level++) {
    int field = FIELD_BY_LEVEL[level];
    if (!cal->isSet(field)) {
      break;
    }
    lastField = field;
  }
  return lastField;
}

void DateRangePrefixTree::clearFieldsAfter(shared_ptr<Calendar> cal, int field)
{
  int assertEra = -1;
  assert(assertEra = ((std::static_pointer_cast<Calendar>(cal->clone()))
                          ->get(Calendar::ERA))) >=
      0; // a trick to only get this if assert enabled
  // note: Calendar.ERA == 0;
  for (int f = field + 1; f <= Calendar::MILLISECOND; f++) {
    cal->clear(f);
  }
  assert((field + 1 == Calendar::ERA ||
              (std::static_pointer_cast<Calendar>(cal->clone()))
                      ->get(Calendar::ERA) == assertEra,
          L"Calendar underflow"));
}

shared_ptr<UnitNRShape> DateRangePrefixTree::toUnitShape(any value)
{
  if (std::dynamic_pointer_cast<Calendar>(value) != nullptr) {
    return toShape(any_cast<std::shared_ptr<Calendar>>(value));
  } else if (dynamic_cast<Date>(value) != nullptr) {
    shared_ptr<Calendar> cal = newCal();
    cal->setTime(any_cast<Date>(value));
    return toShape(cal);
  }
  throw invalid_argument(L"Expecting Calendar or Date but got: " +
                         value.type());
}

shared_ptr<UnitNRShape> DateRangePrefixTree::toShape(shared_ptr<Calendar> cal)
{
  // Convert a Calendar into a stack of cell numbers
  constexpr int calPrecField =
      getCalPrecisionField(cal); // must call first; getters set all fields
  try {
    std::deque<int> valStack(maxLevels); // starts at level 1, not 0
    int len = 0;
    if (calPrecField >= Calendar::YEAR) { // year or better precision
      int year = cal->get(Calendar::YEAR);
      int yearAdj = cal->get(Calendar::ERA) == 0 ? AD_YEAR_BASE - (year - 1)
                                                 : AD_YEAR_BASE + year;

      valStack[len++] = yearAdj / 1000'000;
      yearAdj -= valStack[len - 1] * 1000'000;
      valStack[len++] = yearAdj / 1000;
      yearAdj -= valStack[len - 1] * 1000;
      valStack[len++] = yearAdj;
      for (int level = YEAR_LEVEL + 1; level < FIELD_BY_LEVEL.size(); level++) {
        int field = FIELD_BY_LEVEL[level];
        if (field > calPrecField) {
          break;
        }
        valStack[len++] = cal->get(field) - cal->getActualMinimum(field);
      }
    }

    return toShape(valStack, len);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    clearFieldsAfter(cal,
                     calPrecField); // restore precision state modified by get()
  }
}

any DateRangePrefixTree::toObject(shared_ptr<UnitNRShape> shape)
{
  return toCalendar(shape);
}

shared_ptr<Calendar> DateRangePrefixTree::toCalendar(shared_ptr<UnitNRShape> lv)
{
  if (lv->getLevel() == 0) {
    return newCal();
  }
  if (comparePrefix(lv, minLV) <=
      0) { // shouldn't typically happen; sometimes in a debugger
    return std::static_pointer_cast<Calendar>(
        MINCAL->clone()); // full precision; truncation would cause underflow
  }
  assert((comparePrefix(lv, maxLV) <= 0));
  shared_ptr<Calendar> cal = newCal();

  int yearAdj = lv->getValAtLevel(1) * 1'000'000;
  if (lv->getLevel() > 1) {
    yearAdj += lv->getValAtLevel(2) * 1000;
    if (lv->getLevel() > 2) {
      yearAdj += lv->getValAtLevel(3);
    }
  }
  if (yearAdj > AD_YEAR_BASE) {
    cal->set(Calendar::ERA, 1);
    cal->set(Calendar::YEAR,
             yearAdj - AD_YEAR_BASE); // setting the year resets the era
  } else {
    cal->set(Calendar::ERA, 0); // we assert this "sticks" at the end
    cal->set(Calendar::YEAR, (AD_YEAR_BASE - yearAdj) + 1);
  }
  for (int level = YEAR_LEVEL + 1; level <= lv->getLevel(); level++) {
    int field = FIELD_BY_LEVEL[level];
    cal->set(field, lv->getValAtLevel(level) + cal->getActualMinimum(field));
  }
  assert((yearAdj > AD_YEAR_BASE ||
              (std::static_pointer_cast<Calendar>(cal->clone()))
                      ->get(Calendar::ERA) == 0,
          L"ERA / YEAR underflow"));
  return cal;
}

wstring DateRangePrefixTree::toString(shared_ptr<UnitNRShape> lv)
{
  return toString(toCalendar(lv));
}

wstring DateRangePrefixTree::toString(shared_ptr<Calendar> cal)
{
  constexpr int calPrecField =
      getCalPrecisionField(cal); // must call first; getters set all fields
  if (calPrecField == -1) {
    return L"*";
  }
  try {
    shared_ptr<StringBuilder> builder = make_shared<StringBuilder>(
        (wstring(L"yyyy-MM-dd'T'HH:mm:ss.SSS")).length()); // typical
    int year =
        cal->get(Calendar::YEAR); // within the era (thus always positve). >= 1.
    if (cal->get(Calendar::ERA) == 0) { // BC
      year -= 1;                        // 1BC should be "0000", so shift by one
      if (year > 0) {
        builder->append(L'-');
      }
    } else if (year > 9999) {
      builder->append(L'+');
    }
    appendPadded(builder, year, static_cast<short>(4));
    if (calPrecField >= Calendar::MONTH) {
      builder->append(L'-');
      appendPadded(builder, cal->get(Calendar::MONTH) + 1,
                   static_cast<short>(2)); // +1 since first is 0
    }
    if (calPrecField >= Calendar::DAY_OF_MONTH) {
      builder->append(L'-');
      appendPadded(builder, cal->get(Calendar::DAY_OF_MONTH),
                   static_cast<short>(2));
    }
    if (calPrecField >= Calendar::HOUR_OF_DAY) {
      builder->append(L'T');
      appendPadded(builder, cal->get(Calendar::HOUR_OF_DAY),
                   static_cast<short>(2));
    }
    if (calPrecField >= Calendar::MINUTE) {
      builder->append(L':');
      appendPadded(builder, cal->get(Calendar::MINUTE), static_cast<short>(2));
    }
    if (calPrecField >= Calendar::SECOND) {
      builder->append(L':');
      appendPadded(builder, cal->get(Calendar::SECOND), static_cast<short>(2));
    }
    if (calPrecField >= Calendar::MILLISECOND &&
        cal->get(Calendar::MILLISECOND) > 0) { // only if non-zero
      builder->append(L'.');
      appendPadded(builder, cal->get(Calendar::MILLISECOND),
                   static_cast<short>(3));
    }

    return builder->toString();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    clearFieldsAfter(cal,
                     calPrecField); // restore precision state modified by get()
  }
}

void DateRangePrefixTree::appendPadded(shared_ptr<StringBuilder> builder,
                                       int integer, short positions)
{
  assert(integer >= 0 && positions >= 1 && positions <= 4);
  int preBuilderLen = builder->length();
  int intStrLen;
  if (integer > 999) {
    intStrLen = 4;
  } else if (integer > 99) {
    intStrLen = 3;
  } else if (integer > 9) {
    intStrLen = 2;
  } else {
    intStrLen = 1;
  }
  for (int i = 0; i < positions - intStrLen; i++) {
    builder->append(L'0');
  }
  builder->append(integer);
}

shared_ptr<UnitNRShape>
DateRangePrefixTree::parseUnitShape(const wstring &str) 
{
  return toShape(parseCalendar(str));
}

shared_ptr<Calendar>
DateRangePrefixTree::parseCalendar(const wstring &str) 
{
  // example: +2014-10-23T21:22:33.159Z
  if (str == L"" || str.isEmpty()) {
    throw invalid_argument(L"str is null or blank");
  }
  shared_ptr<Calendar> cal = newCal();
  if (str == L"*") {
    return cal;
  }
  int offset = 0; // a pointer
  try {
    // year & era:
    int lastOffset =
        str[str.length() - 1] == L'Z' ? str.length() - 1 : str.length();
    int hyphenIdx = (int)str.find(L'-', 1); // look past possible leading hyphen
    if (hyphenIdx < 0) {
      hyphenIdx = lastOffset;
    }
    int year = stoi(str.substr(offset, hyphenIdx - offset));
    cal->set(Calendar::ERA, year <= 0 ? 0 : 1);
    cal->set(Calendar::YEAR, year <= 0 ? -1 * year + 1 : year);
    offset = hyphenIdx + 1;
    if (lastOffset < offset) {
      return cal;
    }

    // NOTE: We aren't validating separator chars, and we unintentionally accept
    // leading +/-.
    // The str.substring()'s hopefully get optimized to be stack-allocated.

    // month:
    cal->set(Calendar::MONTH, stoi(str.substr(offset, 2)) - 1); // starts at 0
    offset += 3;
    if (lastOffset < offset) {
      return cal;
    }
    // day:
    cal->set(Calendar::DAY_OF_MONTH, stoi(str.substr(offset, 2)));
    offset += 3;
    if (lastOffset < offset) {
      return cal;
    }
    // hour:
    cal->set(Calendar::HOUR_OF_DAY, stoi(str.substr(offset, 2)));
    offset += 3;
    if (lastOffset < offset) {
      return cal;
    }
    // minute:
    cal->set(Calendar::MINUTE, stoi(str.substr(offset, 2)));
    offset += 3;
    if (lastOffset < offset) {
      return cal;
    }
    // second:
    cal->set(Calendar::SECOND, stoi(str.substr(offset, 2)));
    offset += 3;
    if (lastOffset < offset) {
      return cal;
    }
    // ms:
    cal->set(Calendar::MILLISECOND, stoi(str.substr(offset, 3)));
    offset += 3; // last one, move to next char
    if (lastOffset == offset) {
      return cal;
    }
  } catch (const runtime_error &e) {
    shared_ptr<ParseException> pe = make_shared<ParseException>(
        L"Improperly formatted date: " + str, offset);
    pe->initCause(e);
    throw pe;
  }
  throw make_shared<ParseException>(L"Improperly formatted date: " + str,
                                    offset);
}
} // namespace org::apache::lucene::spatial::prefix::tree