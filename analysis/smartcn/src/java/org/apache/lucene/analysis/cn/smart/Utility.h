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
namespace org::apache::lucene::analysis::cn::smart
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static Character.isSurrogate;

/**
 * SmartChineseAnalyzer utility constants and methods
 * @lucene.experimental
 */
class Utility : public std::enable_shared_from_this<Utility>
{
  GET_CLASS_NAME(Utility)

public:
  static std::deque<wchar_t> const STRING_CHAR_ARRAY;

  static std::deque<wchar_t> const NUMBER_CHAR_ARRAY;

  static std::deque<wchar_t> const START_CHAR_ARRAY;

  static std::deque<wchar_t> const END_CHAR_ARRAY;

  /**
   * Delimiters will be filtered to this character by {@link SegTokenFilter}
   */
  static std::deque<wchar_t> const COMMON_DELIMITER;

  /**
   * Space-like characters that need to be skipped: such as space, tab, newline,
   * carriage return.
   */
  static const std::wstring SPACES;

  /**
   * Maximum bigram frequency (used in the smoothing function).
   */
  static constexpr int MAX_FREQUENCE = 2079997 + 80000;

  /**
   * compare two arrays starting at the specified offsets.
   *
   * @param larray left array
   * @param lstartIndex start offset into larray
   * @param rarray right array
   * @param rstartIndex start offset into rarray
   * @return 0 if the arrays are equal，1 if larray &gt; rarray, -1 if larray
   * &lt; rarray
   */
  static int compareArray(std::deque<wchar_t> &larray, int lstartIndex,
                          std::deque<wchar_t> &rarray, int rstartIndex);

  /**
   * Compare two arrays, starting at the specified offsets, but treating
   * shortArray as a prefix to longArray. As long as shortArray is a prefix of
   * longArray, return 0. Otherwise, behave as {@link
   * Utility#compareArray(char[], int, char[], int)}
   *
   * @param shortArray prefix array
   * @param shortIndex offset into shortArray
   * @param longArray long array (word)
   * @param longIndex offset into longArray
   * @return 0 if shortArray is a prefix of longArray, otherwise act as {@link
   * Utility#compareArray(char[], int, char[], int)}
   */
  static int compareArrayByPrefix(std::deque<wchar_t> &shortArray,
                                  int shortIndex,
                                  std::deque<wchar_t> &longArray,
                                  int longIndex);

  /**
   * Return the internal {@link CharType} constant of a given character.
   * @param ch input character
   * @return constant from {@link CharType} describing the character type.
   *
   * @see CharType
   */
  static int getCharType(wchar_t ch);
};

} // namespace org::apache::lucene::analysis::cn::smart
