#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
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
namespace org::apache::lucene::spatial::prefix::tree
{

/**
 * A PrefixTree for date ranges in which the levels of the tree occur at natural
 * periods of time (e.g. years, months, ...). You pass in {@link Calendar}
 * objects with the desired fields set and the unspecified fields unset, which
 * conveys the precision.  The implementation makes some optimization
 * assumptions about a
 * {@link java.util.GregorianCalendar}; others could probably be supported
 * easily. <p> Warning: If you construct a Calendar and then get something from
 * the object like a field (e.g. year) or milliseconds, then every field is
 * fully set by side-effect. So after setting the fields, pass it to this API
 * first.
 * @lucene.experimental
 */
class DateRangePrefixTree : public NumberRangePrefixTree
{
  GET_CLASS_NAME(DateRangePrefixTree)

  /*
    WARNING  java.util.Calendar is tricky to work with:
    * If you "get" any field value, every field becomes "set". This can
    introduce a Heisenbug effect, when in a debugger in some cases. Fortunately,
    Calendar.toString() doesn't apply.
    * Beware Calendar underflow of the underlying long.  If you create a
    Calendar from LONG.MIN_VALUE, and clear a field, it will underflow and
    appear close to LONG.MAX_VALUE (BC to AD).

    There are no doubt other reasons but those two were hard fought lessons
    here.

    TODO Improvements:
    * Make max precision configurable (i.e. to SECOND).
    * Make min & max year span configurable. Use that to remove pointless top
    levels of the SPT. If year span is > 10k, then add 1k year level. If year
    span is > 10k of 1k levels, add 1M level.
    * NumberRangePrefixTree: override getTreeCellIterator for optimized case
    where the shape isn't a date span; use FilterCellIterator of the cell stack.

  */

private:
  static const std::shared_ptr<TimeZone> UTC;

  /**
   * The Java platform default {@link Calendar} with UTC &amp; ROOT Locale.
   * Generally a {@link GregorianCalendar}. Do <em>not</em> modify this!
   */
public:
  static const std::shared_ptr<Calendar> DEFAULT_CAL; // template
private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static DateRangePrefixTree::StaticConstructor staticConstructor;

  /**
   * A Calendar instance compatible with {@link java.time.ZonedDateTime} as seen
   * from
   * {@link GregorianCalendar#from(ZonedDateTime)}.
   * Do <em>not</em> modify this!
   */
public:
  static const std::shared_ptr<Calendar> JAVA_UTIL_TIME_COMPAT_CAL;

private:
  static std::deque<int> const FIELD_BY_LEVEL;

  static constexpr int YEAR_LEVEL = 3;

  // how many million years are there?
  static constexpr int NUM_MYEARS =
      586; // we assert how this was computed in the constructor

  /** An instanced based on {@link Calendar#getInstance(TimeZone, Locale)} with
   * UTC and Locale.Root. This will (always?) be a {@link GregorianCalendar}
   * with a so-called "Gregorian Change Date" of 1582.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final DateRangePrefixTree INSTANCE
  // = new DateRangePrefixTree(DEFAULT_CAL);
  static const std::shared_ptr<DateRangePrefixTree> INSTANCE;

  // Instance fields: (all are final)

private:
  const std::shared_ptr<Calendar> CAL_TMP; // template

  const std::shared_ptr<Calendar> MINCAL;
  const std::shared_ptr<Calendar> MAXCAL;

  const int BC_FIRSTYEAR;
  const int BC_LASTYEAR;
  const int BC_YEARS;
  const int AD_FIRSTYEAR;
  const int AD_LASTYEAR;
  const int AD_YEAR_BASE;

  const std::shared_ptr<UnitNRShape> minLV, maxLV;
  const std::shared_ptr<UnitNRShape> gregorianChangeDateLV;

  /** Constructs with the specified calendar used as a template to be cloned
   * whenever a new
   * Calendar needs to be created.  See {@link #DEFAULT_CAL} and {@link
   * #JAVA_UTIL_TIME_COMPAT_CAL}. */
public:
  DateRangePrefixTree(std::shared_ptr<Calendar> templateCal);

private:
  static int calFieldLen(std::shared_ptr<Calendar> cal, int field);

public:
  int getNumSubCells(std::shared_ptr<UnitNRShape> lv) override;

private:
  int fastSubCells(std::shared_ptr<UnitNRShape> lv);

  int slowSubCells(std::shared_ptr<UnitNRShape> lv);

  /** Calendar utility method:
   * Returns a clone of the {@link Calendar} passed to the constructor with all
   * fields cleared. */
public:
  virtual std::shared_ptr<Calendar> newCal();

  /** Calendar utility method:
   * Returns the spatial prefix tree level for the corresponding {@link
   * java.util.Calendar} field, such as
   * {@link java.util.Calendar#YEAR}.  If there's no match, the next greatest
   * level is returned as a negative value.
   */
  virtual int getTreeLevelForCalendarField(int calField);

  /** Calendar utility method:
   * Gets the Calendar field code of the last field that is set prior to an
   * unset field. It only examines fields relevant to the prefix tree. If no
   * fields are set, it returns -1. */
  virtual int getCalPrecisionField(std::shared_ptr<Calendar> cal);

  /** Calendar utility method:
   * Calls {@link Calendar#clear(int)} for every field after {@code field}.
   * Beware of Calendar underflow. */
  virtual void clearFieldsAfter(std::shared_ptr<Calendar> cal, int field);

  /** Converts {@code value} from a {@link Calendar} or {@link Date} to a {@link
   * Shape}. Other arguments result in a {@link
   * java.lang.IllegalArgumentException}. If a Calendar is passed in, there
   * might be problems if it is not created via {@link #newCal()}.
   */
  std::shared_ptr<UnitNRShape> toUnitShape(std::any value) override;

  /** Converts the Calendar into a Shape.
   * The isSet() state of the Calendar is re-instated when done.
   * If a Calendar is passed in, there might be problems if it is not created
   * via {@link #newCal()}.
   */
  virtual std::shared_ptr<UnitNRShape> toShape(std::shared_ptr<Calendar> cal);

  /** Calls {@link
   * #toCalendar(org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape)}.
   */
  std::any toObject(std::shared_ptr<UnitNRShape> shape) override;

  /** Converts the {@link
   * org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape}
   * shape to a corresponding Calendar that is cleared below its level. */
  virtual std::shared_ptr<Calendar> toCalendar(std::shared_ptr<UnitNRShape> lv);

protected:
  std::wstring toString(std::shared_ptr<UnitNRShape> lv) override;

  /** Calendar utility method consistent with {@link
   * java.time.format.DateTimeFormatter#ISO_INSTANT} except has no trailing 'Z',
   * and will be truncated to the units given according to
   * {@link Calendar#isSet(int)}.
   * A fully cleared calendar will yield the string "*".
   * The isSet() state of the Calendar is re-instated when done. */
public:
  virtual std::wstring toString(std::shared_ptr<Calendar> cal);

private:
  void appendPadded(std::shared_ptr<StringBuilder> builder, int integer,
                    short positions);

protected:
  std::shared_ptr<UnitNRShape>
  parseUnitShape(const std::wstring &str)  override;

  /** Calendar utility method:
   * The reverse of {@link #toString(java.util.Calendar)}. It will only set the
   * fields found, leaving the remainder in an un-set state. A leading '-' or
   * '+' is optional (positive assumed), and a trailing 'Z' is also optional.
   * @param str not null and not empty
   * @return not null
   */
public:
  virtual std::shared_ptr<Calendar>
  parseCalendar(const std::wstring &str) ;

protected:
  std::shared_ptr<DateRangePrefixTree> shared_from_this()
  {
    return std::static_pointer_cast<DateRangePrefixTree>(
        NumberRangePrefixTree::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
