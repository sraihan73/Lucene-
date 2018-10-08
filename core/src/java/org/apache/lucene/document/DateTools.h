#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::document
{

/**
 * Provides support for converting dates to strings and vice-versa.
 * The strings are structured so that lexicographic sorting orders
 * them by date, which makes them suitable for use as field values
 * and search terms.
 *
 * <P>This class also helps you to limit the resolution of your dates. Do not
 * save dates with a finer resolution than you really need, as then
 * {@link TermRangeQuery} and {@link PrefixQuery} will require more memory and
 * become slower.
 *
 * <P>
 * Another approach is {@link LongPoint}, which indexes the
 * values in sorted order.
 * For indexing a {@link Date} or {@link Calendar}, just get the unix timestamp
 * as <code>long</code> using {@link Date#getTime} or {@link
 * Calendar#getTimeInMillis} and index this as a numeric value with {@link
 * LongPoint} and use {@link org.apache.lucene.search.PointRangeQuery} to query
 * it.
 */
class DateTools : public std::enable_shared_from_this<DateTools>
{
  GET_CLASS_NAME(DateTools)

public:
  static const std::shared_ptr<TimeZone> GMT;

private:
  static const std::shared_ptr<ThreadLocal<std::shared_ptr<Calendar>>> TL_CAL;

private:
  class ThreadLocalAnonymousInnerClass
      : public ThreadLocal<std::shared_ptr<Calendar>>
  {
    GET_CLASS_NAME(ThreadLocalAnonymousInnerClass)
  public:
    ThreadLocalAnonymousInnerClass();

  protected:
    std::shared_ptr<Calendar> initialValue() override;

  protected:
    std::shared_ptr<ThreadLocalAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadLocalAnonymousInnerClass>(
          ThreadLocal<java.util.Calendar>::shared_from_this());
    }
  };

  // indexed by format length
private:
  static const std::shared_ptr<
      ThreadLocal<std::deque<std::shared_ptr<SimpleDateFormat>>>>
      TL_FORMATS;

private:
  class ThreadLocalAnonymousInnerClass2
      : public ThreadLocal<std::deque<std::shared_ptr<SimpleDateFormat>>>
  {
    GET_CLASS_NAME(ThreadLocalAnonymousInnerClass2)
  public:
    ThreadLocalAnonymousInnerClass2();

  protected:
    std::deque<std::shared_ptr<SimpleDateFormat>> initialValue() override;

  protected:
    std::shared_ptr<ThreadLocalAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadLocalAnonymousInnerClass2>(
          ThreadLocal<java.text.SimpleDateFormat[]>::shared_from_this());
    }
  };

  // cannot create, the class has static methods only
private:
  DateTools();

  /**
   * Converts a Date to a string suitable for indexing.
   *
   * @param date the date to be converted
   * @param resolution the desired resolution, see
   *  {@link #round(Date, DateTools.Resolution)}
   * @return a string in format <code>yyyyMMddHHmmssSSS</code> or shorter,
   *  depending on <code>resolution</code>; using GMT as timezone
   */
public:
  static std::wstring dateToString(Date date, Resolution resolution);

  /**
   * Converts a millisecond time to a string suitable for indexing.
   *
   * @param time the date expressed as milliseconds since January 1, 1970,
   * 00:00:00 GMT
   * @param resolution the desired resolution, see
   *  {@link #round(long, DateTools.Resolution)}
   * @return a string in format <code>yyyyMMddHHmmssSSS</code> or shorter,
   *  depending on <code>resolution</code>; using GMT as timezone
   */
  static std::wstring timeToString(int64_t time, Resolution resolution);

  /**
   * Converts a string produced by <code>timeToString</code> or
   * <code>dateToString</code> back to a time, represented as the
   * number of milliseconds since January 1, 1970, 00:00:00 GMT.
   *
   * @param dateString the date string to be converted
   * @return the number of milliseconds since January 1, 1970, 00:00:00 GMT
   * @throws ParseException if <code>dateString</code> is not in the
   *  expected format
   */
  static int64_t
  stringToTime(const std::wstring &dateString) ;

  /**
   * Converts a string produced by <code>timeToString</code> or
   * <code>dateToString</code> back to a time, represented as a
   * Date object.
   *
   * @param dateString the date string to be converted
   * @return the parsed time as a Date object
   * @throws ParseException if <code>dateString</code> is not in the
   *  expected format
   */
  static Date
  stringToDate(const std::wstring &dateString) ;

  /**
   * Limit a date's resolution. For example, the date <code>2004-09-21
   * 13:50:11</code> will be changed to <code>2004-09-01 00:00:00</code> when
   * using <code>Resolution.MONTH</code>.
   *
   * @param resolution The desired resolution of the date to be returned
   * @return the date with all values more precise than <code>resolution</code>
   *  set to 0 or 1
   */
  static Date round(Date date, Resolution resolution);

  /**
   * Limit a date's resolution. For example, the date <code>1095767411000</code>
   * (which represents 2004-09-21 13:50:11) will be changed to
   * <code>1093989600000</code> (2004-09-01 00:00:00) when using
   * <code>Resolution.MONTH</code>.
   *
   * @param resolution The desired resolution of the date to be returned
   * @return the date with all values more precise than <code>resolution</code>
   *  set to 0 or 1, expressed as milliseconds since January 1, 1970, 00:00:00
   * GMT
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") public static long
  // round(long time, Resolution resolution)
  static int64_t round(int64_t time, Resolution resolution);

  /** Specifies the time granularity. */
public:
  class Resolution final
  {
    GET_CLASS_NAME(Resolution)

    /** Limit a date's resolution to year granularity. */
  public:
    static Resolution YEAR;
    /** Limit a date's resolution to month granularity. */
    static Resolution MONTH;
    /** Limit a date's resolution to day granularity. */
    static Resolution DAY;
    /** Limit a date's resolution to hour granularity. */
    static Resolution HOUR;
    /** Limit a date's resolution to minute granularity. */
    static Resolution MINUTE;
    /** Limit a date's resolution to second granularity. */
    static Resolution SECOND;
    /** Limit a date's resolution to millisecond granularity. */
    static Resolution MILLISECOND;

  private:
    static std::deque<Resolution> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum {
      GET_CLASS_NAME(InnerEnum) YEAR,
      MONTH,
      DAY,
      HOUR,
      MINUTE,
      SECOND,
      MILLISECOND
    };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

  public:
    const int formatLen;
    const std::shared_ptr<java::text::SimpleDateFormat>
        format; // should be cloned before use, since it's not threadsafe

    Resolution(const std::wstring &name, InnerEnum innerEnum,
               std::shared_ptr<DateTools> outerInstance, int formatLen);

    /** this method returns the name of the resolution
     * in lowercase (for backwards compatibility) */
    std::wstring ToString() override;

  public:
    bool operator==(const Resolution &other);

    bool operator!=(const Resolution &other);

    static std::deque<Resolution> values();

    int ordinal();

    static Resolution valueOf(const std::wstring &name);
  };
};

} // #include  "core/src/java/org/apache/lucene/document/
