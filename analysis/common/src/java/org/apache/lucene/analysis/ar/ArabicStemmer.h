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
 *  Stemmer for Arabic.
 *  <p>
 *  Stemming  is done in-place for efficiency, operating on a termbuffer.
 *  <p>
 *  Stemming is defined as:
 *  <ul>
 *  <li> Removal of attached definite article, conjunction, and prepositions.
 *  <li> Stemming of common suffixes.
 * </ul>
 *
 */
class ArabicStemmer : public std::enable_shared_from_this<ArabicStemmer>
{
  GET_CLASS_NAME(ArabicStemmer)
public:
  static constexpr wchar_t ALEF = L'\u0627';
  static constexpr wchar_t BEH = L'\u0628';
  static constexpr wchar_t TEH_MARBUTA = L'\u0629';
  static constexpr wchar_t TEH = L'\u062A';
  static constexpr wchar_t FEH = L'\u0641';
  static constexpr wchar_t KAF = L'\u0643';
  static constexpr wchar_t LAM = L'\u0644';
  static constexpr wchar_t NOON = L'\u0646';
  static constexpr wchar_t HEH = L'\u0647';
  static constexpr wchar_t WAW = L'\u0648';
  static constexpr wchar_t YEH = L'\u064A';

  static std::deque<std::deque<wchar_t>> const prefixes;

  static std::deque<std::deque<wchar_t>> const suffixes;

  /**
   * Stem an input buffer of Arabic text.
   *
   * @param s input buffer
   * @param len length of input buffer
   * @return length of input buffer after normalization
   */
  virtual int stem(std::deque<wchar_t> &s, int len);

  /**
   * Stem a prefix off an Arabic word.
   * @param s input buffer
   * @param len length of input buffer
   * @return new length of input buffer after stemming.
   */
  virtual int stemPrefix(std::deque<wchar_t> &s, int len);

  /**
   * Stem suffix(es) off an Arabic word.
   * @param s input buffer
   * @param len length of input buffer
   * @return new length of input buffer after stemming
   */
  virtual int stemSuffix(std::deque<wchar_t> &s, int len);

  /**
   * Returns true if the prefix matches and can be stemmed
   * @param s input buffer
   * @param len length of input buffer
   * @param prefix prefix to check
   * @return true if the prefix matches and can be stemmed
   */
  virtual bool startsWithCheckLength(std::deque<wchar_t> &s, int len,
                                     std::deque<wchar_t> &prefix);

  /**
   * Returns true if the suffix matches and can be stemmed
   * @param s input buffer
   * @param len length of input buffer
   * @param suffix suffix to check
   * @return true if the suffix matches and can be stemmed
   */
  virtual bool endsWithCheckLength(std::deque<wchar_t> &s, int len,
                                   std::deque<wchar_t> &suffix);
};

} // #include  "core/src/java/org/apache/lucene/analysis/ar/
