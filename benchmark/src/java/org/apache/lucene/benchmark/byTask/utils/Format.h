#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::benchmark::byTask::utils
{

/**
 * Formatting utilities (for reports).
 */
class Format : public std::enable_shared_from_this<Format>
{
  GET_CLASS_NAME(Format)

private:
  static std::deque<std::shared_ptr<NumberFormat>> numFormat;
  static const std::wstring padd;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static Format::StaticConstructor staticConstructor;

  /**
   * Padd a number from left.
   * @param numFracDigits number of digits in fraction part - must be 0 or 1
   * or 2.
   * @param f number to be formatted.
   * @param col column name (used for deciding on length).
   * @return formatted string.
   */
public:
  static std::wstring format(int numFracDigits, float f,
                             const std::wstring &col);

  static std::wstring format(int numFracDigits, double f,
                             const std::wstring &col);

  /**
   * Pad a number from right.
   * @param numFracDigits number of digits in fraction part - must be 0 or 1
   * or 2.
   * @param f number to be formatted.
   * @param col column name (used for deciding on length).
   * @return formatted string.
   */
  static std::wstring formatPaddRight(int numFracDigits, float f,
                                      const std::wstring &col);

  static std::wstring formatPaddRight(int numFracDigits, double f,
                                      const std::wstring &col);

  /**
   * Pad a number from left.
   * @param n number to be formatted.
   * @param col column name (used for deciding on length).
   * @return formatted string.
   */
  static std::wstring format(int n, const std::wstring &col);

  /**
   * Pad a string from right.
   * @param s string to be formatted.
   * @param col column name (used for deciding on length).
   * @return formatted string.
   */
  static std::wstring format(const std::wstring &s, const std::wstring &col);

  /**
   * Pad a string from left.
   * @param s string to be formatted.
   * @param col column name (used for deciding on length).
   * @return formatted string.
   */
  static std::wstring formatPaddLeft(const std::wstring &s,
                                     const std::wstring &col);
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/
