using namespace std;

#include "DateTools.h"

namespace org::apache::lucene::document
{
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
const shared_ptr<java::util::TimeZone> DateTools::GMT =
    java::util::TimeZone::getTimeZone(L"GMT");
const shared_ptr<ThreadLocal<std::shared_ptr<java::util::Calendar>>>
    DateTools::TL_CAL = make_shared<ThreadLocalAnonymousInnerClass>();

DateTools::ThreadLocalAnonymousInnerClass::ThreadLocalAnonymousInnerClass() {}

shared_ptr<Calendar> DateTools::ThreadLocalAnonymousInnerClass::initialValue()
{
  return Calendar::getInstance(GMT, Locale::ROOT);
}

const shared_ptr<
    ThreadLocal<std::deque<std::shared_ptr<java::text::SimpleDateFormat>>>>
    DateTools::TL_FORMATS = make_shared<ThreadLocalAnonymousInnerClass2>();

DateTools::ThreadLocalAnonymousInnerClass2::ThreadLocalAnonymousInnerClass2() {}

std::deque<std::shared_ptr<SimpleDateFormat>>
DateTools::ThreadLocalAnonymousInnerClass2::initialValue()
{
  std::deque<std::shared_ptr<SimpleDateFormat>> arr(
      Resolution::MILLISECOND.formatLen + 1);
  for (Resolution resolution : Resolution::values()) {
    arr[resolution.formatLen] =
        std::static_pointer_cast<SimpleDateFormat>(resolution.format::clone());
  }
  return arr;
}

DateTools::DateTools() {}

wstring DateTools::dateToString(Date date, Resolution resolution)
{
  return timeToString(date.getTime(), resolution);
}

wstring DateTools::timeToString(int64_t time, Resolution resolution)
{
  constexpr Date date = Date(round(time, resolution));
  return TL_FORMATS->get()[resolution.formatLen].format(date);
}

int64_t
DateTools::stringToTime(const wstring &dateString) 
{
  return stringToDate(dateString).getTime();
}

Date DateTools::stringToDate(const wstring &dateString) 
{
  try {
    return TL_FORMATS->get()[dateString.length()].parse(dateString);
  } catch (const runtime_error &e) {
    throw make_shared<ParseException>(
        L"Input is not a valid date string: " + dateString, 0);
  }
}

Date DateTools::round(Date date, Resolution resolution)
{
  return Date(round(date.getTime(), resolution));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") public static long round(long
// time, Resolution resolution)
int64_t DateTools::round(int64_t time, Resolution resolution)
{
  shared_ptr<Calendar> *const calInstance = TL_CAL->get();
  calInstance->setTimeInMillis(time);

  switch (resolution.innerEnumValue) {
  // NOTE: switch statement fall-through is deliberate
  case org::apache::lucene::document::DateTools::Resolution::InnerEnum::YEAR:
    calInstance->set(Calendar::MONTH, 0);
  case org::apache::lucene::document::DateTools::Resolution::InnerEnum::MONTH:
    calInstance->set(Calendar::DAY_OF_MONTH, 1);
  case org::apache::lucene::document::DateTools::Resolution::InnerEnum::DAY:
    calInstance->set(Calendar::HOUR_OF_DAY, 0);
  case org::apache::lucene::document::DateTools::Resolution::InnerEnum::HOUR:
    calInstance->set(Calendar::MINUTE, 0);
  case org::apache::lucene::document::DateTools::Resolution::InnerEnum::MINUTE:
    calInstance->set(Calendar::SECOND, 0);
  case org::apache::lucene::document::DateTools::Resolution::InnerEnum::SECOND:
    calInstance->set(Calendar::MILLISECOND, 0);
  case org::apache::lucene::document::DateTools::Resolution::InnerEnum::
      MILLISECOND:
    // don't cut off anything
    break;
  default:
    throw invalid_argument(L"unknown resolution " + resolution);
  }
  return calInstance->getTimeInMillis();
}

Resolution Resolution::YEAR(L"YEAR", InnerEnum::YEAR, 4);
Resolution Resolution::MONTH(L"MONTH", InnerEnum::MONTH, 6);
Resolution Resolution::DAY(L"DAY", InnerEnum::DAY, 8);
Resolution Resolution::HOUR(L"HOUR", InnerEnum::HOUR, 10);
Resolution Resolution::MINUTE(L"MINUTE", InnerEnum::MINUTE, 12);
Resolution Resolution::SECOND(L"SECOND", InnerEnum::SECOND, 14);
Resolution Resolution::MILLISECOND(L"MILLISECOND", InnerEnum::MILLISECOND, 17);

deque<Resolution> Resolution::valueList;

Resolution::StaticConstructor::StaticConstructor()
{
  valueList.push_back(YEAR);
  valueList.push_back(MONTH);
  valueList.push_back(DAY);
  valueList.push_back(HOUR);
  valueList.push_back(MINUTE);
  valueList.push_back(SECOND);
  valueList.push_back(MILLISECOND);
}

Resolution::StaticConstructor Resolution::staticConstructor;
int Resolution::nextOrdinal = 0;

DateTools::Resolution::Resolution(const wstring &name, InnerEnum innerEnum,
                                  shared_ptr<DateTools> outerInstance,
                                  int formatLen)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
  this->outerInstance = outerInstance;
  this->formatLen = formatLen;
  // formatLen 10's place:                     11111111
  // formatLen  1's place:            12345678901234567
  this->format = make_shared<SimpleDateFormat>(
      (wstring(L"yyyyMMddHHmmssSSS")).substr(0, formatLen), Locale::ROOT);
  this->format->setTimeZone(GMT);
}

wstring DateTools::Resolution::ToString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return __super::toString()->toLowerCase(Locale::ROOT);
}

bool Resolution::operator==(const Resolution &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Resolution::operator!=(const Resolution &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Resolution> Resolution::values() { return valueList; }

int Resolution::ordinal() { return ordinalValue; }

Resolution Resolution::valueOf(const wstring &name)
{
  for (auto enumInstance : Resolution::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}
} // namespace org::apache::lucene::document