#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis::ar
{

using namespace org::apache::lucene::analysis::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

/**
 *  Normalizer for Arabic.
 *  <p>
 *  Normalization is done in-place for efficiency, operating on a termbuffer.
 *  <p>
 *  Normalization is defined as:
 *  <ul>
 *  <li> Normalization of hamza with alef seat to a bare alef.
 *  <li> Normalization of teh marbuta to heh
 *  <li> Normalization of dotless yeh (alef maksura) to yeh.
 *  <li> Removal of Arabic diacritics (the harakat)
 *  <li> Removal of tatweel (stretching character).
 * </ul>
 *
 */
class ArabicNormalizer : public std::enable_shared_from_this<ArabicNormalizer>
{
  GET_CLASS_NAME(ArabicNormalizer)
public:
  static constexpr wchar_t ALEF = L'\u0627';
  static constexpr wchar_t ALEF_MADDA = L'\u0622';
  static constexpr wchar_t ALEF_HAMZA_ABOVE = L'\u0623';
  static constexpr wchar_t ALEF_HAMZA_BELOW = L'\u0625';

  static constexpr wchar_t YEH = L'\u064A';
  static constexpr wchar_t DOTLESS_YEH = L'\u0649';

  static constexpr wchar_t TEH_MARBUTA = L'\u0629';
  static constexpr wchar_t HEH = L'\u0647';

  static constexpr wchar_t TATWEEL = L'\u0640';

  static constexpr wchar_t FATHATAN = L'\u064B';
  static constexpr wchar_t DAMMATAN = L'\u064C';
  static constexpr wchar_t KASRATAN = L'\u064D';
  static constexpr wchar_t FATHA = L'\u064E';
  static constexpr wchar_t DAMMA = L'\u064F';
  static constexpr wchar_t KASRA = L'\u0650';
  static constexpr wchar_t SHADDA = L'\u0651';
  static constexpr wchar_t SUKUN = L'\u0652';

  /**
   * Normalize an input buffer of Arabic text
   *
   * @param s input buffer
   * @param len length of input buffer
   * @return length of input buffer after normalization
   */
  virtual int normalize(std::deque<wchar_t> &s, int len);
};

} // namespace org::apache::lucene::analysis::ar
