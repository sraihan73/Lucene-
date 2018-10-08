using namespace std;

#include "TestDateTools.h"

namespace org::apache::lucene::document
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDateTools::testStringToDate() 
{

  Date d = nullptr;
  d = DateTools::stringToDate(L"2004");
  assertEquals(L"2004-01-01 00:00:00:000", isoFormat(d));
  d = DateTools::stringToDate(L"20040705");
  assertEquals(L"2004-07-05 00:00:00:000", isoFormat(d));
  d = DateTools::stringToDate(L"200407050910");
  assertEquals(L"2004-07-05 09:10:00:000", isoFormat(d));
  d = DateTools::stringToDate(L"20040705091055990");
  assertEquals(L"2004-07-05 09:10:55:990", isoFormat(d));

  expectThrows(ParseException::typeid,
               [&]() { DateTools::stringToDate(L"97"); });

  expectThrows(ParseException::typeid,
               [&]() { DateTools::stringToDate(L"200401011235009999"); });

  expectThrows(ParseException::typeid,
               [&]() { DateTools::stringToDate(L"aaaa"); });
}

void TestDateTools::testStringtoTime() 
{
  int64_t time = DateTools::stringToTime(L"197001010000");
  // we use default locale since LuceneTestCase randomizes it
  shared_ptr<Calendar> cal = make_shared<GregorianCalendar>(
      TimeZone::getTimeZone(L"GMT"), Locale::getDefault());
  cal->clear();
  cal->set(1970, 0, 1, 0, 0, 0); // hour, minute, second
  cal->set(Calendar::MILLISECOND, 0);
  assertEquals(cal->getTime().getTime(), time);
  cal->set(1980, 1, 2, 11, 5, 0); // hour, minute, second
  cal->set(Calendar::MILLISECOND, 0);
  time = DateTools::stringToTime(L"198002021105");
  assertEquals(cal->getTime().getTime(), time);
}

void TestDateTools::testDateAndTimetoString() 
{
  // we use default locale since LuceneTestCase randomizes it
  shared_ptr<Calendar> cal = make_shared<GregorianCalendar>(
      TimeZone::getTimeZone(L"GMT"), Locale::getDefault());
  cal->clear();
  cal->set(2004, 1, 3, 22, 8, 56); // hour, minute, second
  cal->set(Calendar::MILLISECOND, 333);

  wstring dateString;
  dateString =
      DateTools::dateToString(cal->getTime(), DateTools::Resolution::YEAR);
  assertEquals(L"2004", dateString);
  assertEquals(L"2004-01-01 00:00:00:000",
               isoFormat(DateTools::stringToDate(dateString)));

  dateString =
      DateTools::dateToString(cal->getTime(), DateTools::Resolution::MONTH);
  assertEquals(L"200402", dateString);
  assertEquals(L"2004-02-01 00:00:00:000",
               isoFormat(DateTools::stringToDate(dateString)));

  dateString =
      DateTools::dateToString(cal->getTime(), DateTools::Resolution::DAY);
  assertEquals(L"20040203", dateString);
  assertEquals(L"2004-02-03 00:00:00:000",
               isoFormat(DateTools::stringToDate(dateString)));

  dateString =
      DateTools::dateToString(cal->getTime(), DateTools::Resolution::HOUR);
  assertEquals(L"2004020322", dateString);
  assertEquals(L"2004-02-03 22:00:00:000",
               isoFormat(DateTools::stringToDate(dateString)));

  dateString =
      DateTools::dateToString(cal->getTime(), DateTools::Resolution::MINUTE);
  assertEquals(L"200402032208", dateString);
  assertEquals(L"2004-02-03 22:08:00:000",
               isoFormat(DateTools::stringToDate(dateString)));

  dateString =
      DateTools::dateToString(cal->getTime(), DateTools::Resolution::SECOND);
  assertEquals(L"20040203220856", dateString);
  assertEquals(L"2004-02-03 22:08:56:000",
               isoFormat(DateTools::stringToDate(dateString)));

  dateString = DateTools::dateToString(cal->getTime(),
                                       DateTools::Resolution::MILLISECOND);
  assertEquals(L"20040203220856333", dateString);
  assertEquals(L"2004-02-03 22:08:56:333",
               isoFormat(DateTools::stringToDate(dateString)));

  // date before 1970:
  cal->set(1961, 2, 5, 23, 9, 51); // hour, minute, second
  cal->set(Calendar::MILLISECOND, 444);
  dateString = DateTools::dateToString(cal->getTime(),
                                       DateTools::Resolution::MILLISECOND);
  assertEquals(L"19610305230951444", dateString);
  assertEquals(L"1961-03-05 23:09:51:444",
               isoFormat(DateTools::stringToDate(dateString)));

  dateString =
      DateTools::dateToString(cal->getTime(), DateTools::Resolution::HOUR);
  assertEquals(L"1961030523", dateString);
  assertEquals(L"1961-03-05 23:00:00:000",
               isoFormat(DateTools::stringToDate(dateString)));

  // timeToString:
  cal->set(1970, 0, 1, 0, 0, 0); // hour, minute, second
  cal->set(Calendar::MILLISECOND, 0);
  dateString = DateTools::timeToString(cal->getTime().getTime(),
                                       DateTools::Resolution::MILLISECOND);
  assertEquals(L"19700101000000000", dateString);

  cal->set(1970, 0, 1, 1, 2, 3); // hour, minute, second
  cal->set(Calendar::MILLISECOND, 0);
  dateString = DateTools::timeToString(cal->getTime().getTime(),
                                       DateTools::Resolution::MILLISECOND);
  assertEquals(L"19700101010203000", dateString);
}

void TestDateTools::testRound()
{
  // we use default locale since LuceneTestCase randomizes it
  shared_ptr<Calendar> cal = make_shared<GregorianCalendar>(
      TimeZone::getTimeZone(L"GMT"), Locale::getDefault());
  cal->clear();
  cal->set(2004, 1, 3, 22, 8, 56); // hour, minute, second
  cal->set(Calendar::MILLISECOND, 333);
  Date date = cal->getTime();
  assertEquals(L"2004-02-03 22:08:56:333", isoFormat(date));

  Date dateYear = DateTools::round(date, DateTools::Resolution::YEAR);
  assertEquals(L"2004-01-01 00:00:00:000", isoFormat(dateYear));

  Date dateMonth = DateTools::round(date, DateTools::Resolution::MONTH);
  assertEquals(L"2004-02-01 00:00:00:000", isoFormat(dateMonth));

  Date dateDay = DateTools::round(date, DateTools::Resolution::DAY);
  assertEquals(L"2004-02-03 00:00:00:000", isoFormat(dateDay));

  Date dateHour = DateTools::round(date, DateTools::Resolution::HOUR);
  assertEquals(L"2004-02-03 22:00:00:000", isoFormat(dateHour));

  Date dateMinute = DateTools::round(date, DateTools::Resolution::MINUTE);
  assertEquals(L"2004-02-03 22:08:00:000", isoFormat(dateMinute));

  Date dateSecond = DateTools::round(date, DateTools::Resolution::SECOND);
  assertEquals(L"2004-02-03 22:08:56:000", isoFormat(dateSecond));

  Date dateMillisecond =
      DateTools::round(date, DateTools::Resolution::MILLISECOND);
  assertEquals(L"2004-02-03 22:08:56:333", isoFormat(dateMillisecond));

  // long parameter:
  int64_t dateYearLong =
      DateTools::round(date.getTime(), DateTools::Resolution::YEAR);
  assertEquals(L"2004-01-01 00:00:00:000", isoFormat(Date(dateYearLong)));

  int64_t dateMillisecondLong =
      DateTools::round(date.getTime(), DateTools::Resolution::MILLISECOND);
  assertEquals(L"2004-02-03 22:08:56:333",
               isoFormat(Date(dateMillisecondLong)));
}

wstring TestDateTools::isoFormat(Date date)
{
  shared_ptr<SimpleDateFormat> sdf =
      make_shared<SimpleDateFormat>(L"yyyy-MM-dd HH:mm:ss:SSS", Locale::ROOT);
  sdf->setTimeZone(TimeZone::getTimeZone(L"GMT"));
  return sdf->format(date);
}

void TestDateTools::testDateToolsUTC() 
{
  // Sun, 30 Oct 2005 00:00:00 +0000 -- the last second of 2005's DST in
  // Europe/London
  int64_t time = 1130630400;
  try {
    TimeZone::setDefault(TimeZone::getTimeZone(L"Europe/London"));
    wstring d1 = DateTools::dateToString(Date(time * 1000),
                                         DateTools::Resolution::MINUTE);
    wstring d2 = DateTools::dateToString(Date((time + 3600) * 1000),
                                         DateTools::Resolution::MINUTE);
    assertFalse(L"different times", d1 == d2);
    assertEquals(L"midnight", DateTools::stringToTime(d1), time * 1000);
    assertEquals(L"later", DateTools::stringToTime(d2), (time + 3600) * 1000);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    TimeZone::setDefault(nullptr);
  }
}
} // namespace org::apache::lucene::document