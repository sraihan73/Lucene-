#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/highlight/TokenGroup.h"

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
namespace org::apache::lucene::search::highlight
{
/**
 * Formats text with different color intensity depending on the score of the
 * term.
 *
 */
class GradientFormatter
    : public std::enable_shared_from_this<GradientFormatter>,
      public Formatter
{
  GET_CLASS_NAME(GradientFormatter)
private:
  float maxScore = 0;

public:
  int fgRMin = 0, fgGMin = 0, fgBMin = 0;

  int fgRMax = 0, fgGMax = 0, fgBMax = 0;

protected:
  bool highlightForeground = false;

public:
  int bgRMin = 0, bgGMin = 0, bgBMin = 0;

  int bgRMax = 0, bgGMax = 0, bgBMax = 0;

protected:
  bool highlightBackground = false;

  /**
   * Sets the color range for the IDF scores
   *
   * @param maxScore
   *            The score (and above) displayed as maxColor (See {@link
   * QueryScorer#getMaxTermWeight()} which can be used to calibrate scoring
   * scale)
   * @param minForegroundColor
   *            The hex color used for representing IDF scores of zero eg
   *            #FFFFFF (white) or null if no foreground color required
   * @param maxForegroundColor
   *            The largest hex color used for representing IDF scores eg
   *            #000000 (black) or null if no foreground color required
   * @param minBackgroundColor
   *            The hex color used for representing IDF scores of zero eg
   *            #FFFFFF (white) or null if no background color required
   * @param maxBackgroundColor
   *            The largest hex color used for representing IDF scores eg
   *            #000000 (black) or null if no background color required
   */
public:
  GradientFormatter(float maxScore, const std::wstring &minForegroundColor,
                    const std::wstring &maxForegroundColor,
                    const std::wstring &minBackgroundColor,
                    const std::wstring &maxBackgroundColor);

  std::wstring highlightTerm(const std::wstring &originalText,
                             std::shared_ptr<TokenGroup> tokenGroup) override;

protected:
  virtual std::wstring getForegroundColorString(float score);

  virtual std::wstring getBackgroundColorString(float score);

private:
  int getColorVal(int colorMin, int colorMax, float score);

  static std::deque<wchar_t> hexDigits;

  static std::wstring intToHex(int i);

  /**
   * Converts a hex string into an int. Integer.parseInt(hex, 16) assumes the
   * input is nonnegative unless there is a preceding minus sign. This method
   * reads the input as twos complement instead, so if the input is 8 bytes
   * long, it will correctly restore a negative int produced by
   * Integer.toHexString() but not necessarily one produced by
   * Integer.toString(x,16) since that method will produce a string like '-FF'
   * for negative integer values.
   *
   * @param hex
   *            A string in capital or lower case hex, of no more then 16
   *            characters.
   * @throws NumberFormatException
   *             if the string is more than 16 characters long, or if any
   *             character is not in the set [0-9a-fA-f]
   */
public:
  static int hexToInt(const std::wstring &hex);
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
