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
namespace org::apache::lucene::analysis::ckb
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.delete;

/**
 * Normalizes the Unicode representation of Sorani text.
 * <p>
 * Normalization consists of:
 * <ul>
 *   <li>Alternate forms of 'y' (0064, 0649) are converted to 06CC (FARSI YEH)
 *   <li>Alternate form of 'k' (0643) is converted to 06A9 (KEHEH)
 *   <li>Alternate forms of vowel 'e' (0647+200C, word-final 0647, 0629) are
 * converted to 06D5 (AE) <li>Alternate (joining) form of 'h' (06BE) is
 * converted to 0647 <li>Alternate forms of 'rr' (0692, word-initial 0631) are
 * converted to 0695 (REH WITH SMALL V BELOW) <li>Harakat, tatweel, and
 * formatting characters such as directional controls are removed.
 * </ul>
 */
class SoraniNormalizer : public std::enable_shared_from_this<SoraniNormalizer>
{
  GET_CLASS_NAME(SoraniNormalizer)

public:
  static constexpr wchar_t YEH = L'\u064A';
  static constexpr wchar_t DOTLESS_YEH = L'\u0649';
  static constexpr wchar_t FARSI_YEH = L'\u06CC';

  static constexpr wchar_t KAF = L'\u0643';
  static constexpr wchar_t KEHEH = L'\u06A9';

  static constexpr wchar_t HEH = L'\u0647';
  static constexpr wchar_t AE = L'\u06D5';
  static constexpr wchar_t ZWNJ = L'\u200C';
  static constexpr wchar_t HEH_DOACHASHMEE = L'\u06BE';
  static constexpr wchar_t TEH_MARBUTA = L'\u0629';

  static constexpr wchar_t REH = L'\u0631';
  static constexpr wchar_t RREH = L'\u0695';
  static constexpr wchar_t RREH_ABOVE = L'\u0692';

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
   * Normalize an input buffer of Sorani text
   *
   * @param s input buffer
   * @param len length of input buffer
   * @return length of input buffer after normalization
   */
  virtual int normalize(std::deque<wchar_t> &s, int len);
};

} // namespace org::apache::lucene::analysis::ckb
