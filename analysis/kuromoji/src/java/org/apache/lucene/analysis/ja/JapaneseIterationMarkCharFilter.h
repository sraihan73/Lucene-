#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::util
{
class RollingCharBuffer;
}

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
namespace org::apache::lucene::analysis::ja
{

using CharFilter = org::apache::lucene::analysis::CharFilter;
using RollingCharBuffer =
    org::apache::lucene::analysis::util::RollingCharBuffer;

/**
 * Normalizes Japanese horizontal iteration marks (odoriji) to their expanded
 * form. <p> Sequences of iteration marks are supported.  In case an illegal
 * sequence of iteration marks is encountered, the implementation emits the
 * illegal source character as-is without considering its script.  For example,
 * with input "?ゝ", we get
 * "??" even though the question mark isn't hiragana.
 * </p>
 * <p>
 * Note that a full stop punctuation character "。" (U+3002) can not be iterated
 * (see below). Iteration marks themselves can be emitted in case they are
 * illegal, i.e. if they go back past the beginning of the character stream.
 * </p>
 * <p>
 * The implementation buffers input until a full stop punctuation character
 * (U+3002) or EOF is reached in order to not keep a copy of the character
 * stream in memory. Vertical iteration marks, which are even rarer than
 * horizontal iteration marks in contemporary Japanese, are unsupported.
 * </p>
 */
class JapaneseIterationMarkCharFilter : public CharFilter
{
  GET_CLASS_NAME(JapaneseIterationMarkCharFilter)

  /** Normalize kanji iteration marks by default */
public:
  static constexpr bool NORMALIZE_KANJI_DEFAULT = true;

  /** Normalize kana iteration marks by default */
  static constexpr bool NORMALIZE_KANA_DEFAULT = true;

private:
  static constexpr wchar_t KANJI_ITERATION_MARK = L'\u3005'; // 々

  static constexpr wchar_t HIRAGANA_ITERATION_MARK = L'\u309d'; // ゝ

  static constexpr wchar_t HIRAGANA_VOICED_ITERATION_MARK = L'\u309e'; // ゞ

  static constexpr wchar_t KATAKANA_ITERATION_MARK = L'\u30fd'; // ヽ

  static constexpr wchar_t KATAKANA_VOICED_ITERATION_MARK = L'\u30fe'; // ヾ

  static constexpr wchar_t FULL_STOP_PUNCTUATION = L'\u3002'; // 。

  // Hiragana to dakuten map_obj (lookup using code point - 0x30ab（か）*/
  static std::deque<wchar_t> h2d;

  // Katakana to dakuten map_obj (lookup using code point - 0x30ab（カ
  static std::deque<wchar_t> k2d;

  const std::shared_ptr<RollingCharBuffer> buffer =
      std::make_shared<RollingCharBuffer>();

  int bufferPosition = 0;

  int iterationMarksSpanSize = 0;

  int iterationMarkSpanEndPosition = 0;

  bool normalizeKanji = false;

  bool normalizeKana = false;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static JapaneseIterationMarkCharFilter::StaticConstructor staticConstructor;

  /**
   * Constructor. Normalizes both kanji and kana iteration marks by default.
   *
   * @param input char stream
   */
public:
  JapaneseIterationMarkCharFilter(std::shared_ptr<Reader> input);

  /**
   * Constructor
   *
   * @param input          char stream
   * @param normalizeKanji indicates whether kanji iteration marks should be
   * normalized
   * @param normalizeKana indicates whether kana iteration marks should be
   * normalized
   */
  JapaneseIterationMarkCharFilter(std::shared_ptr<Reader> input,
                                  bool normalizeKanji, bool normalizeKana);

  int read(std::deque<wchar_t> &buffer, int offset,
           int length)  override;

  int read()  override;

  /**
   * Normalizes the iteration mark character c
   *
   * @param c iteration mark character to normalize
   * @return normalized iteration mark
   * @throws IOException If there is a low-level I/O error.
   */
private:
  wchar_t normalizeIterationMark(wchar_t c) ;

  /**
   * Finds the number of subsequent next iteration marks
   *
   * @return number of iteration marks starting at the current buffer position
   * @throws IOException If there is a low-level I/O error.
   */
  int nextIterationMarkSpanSize() ;

  /**
   * Returns the source character for a given position and iteration mark span
   * size
   *
   * @param position buffer position (should not exceed bufferPosition)
   * @param spanSize iteration mark span size
   * @return source character
   * @throws IOException If there is a low-level I/O error.
   */
  wchar_t sourceCharacter(int position, int spanSize) ;

  /**
   * Normalize a character
   *
   * @param c character to normalize
   * @param m repetition mark referring to c
   * @return normalized character - return c on illegal iteration marks
   */
  wchar_t normalize(wchar_t c, wchar_t m);

  /**
   * Normalize hiragana character
   *
   * @param c hiragana character
   * @param m repetition mark referring to c
   * @return normalized character - return c on illegal iteration marks
   */
  wchar_t normalizedHiragana(wchar_t c, wchar_t m);

  /**
   * Normalize katakana character
   *
   * @param c katakana character
   * @param m repetition mark referring to c
   * @return normalized character - return c on illegal iteration marks
   */
  wchar_t normalizedKatakana(wchar_t c, wchar_t m);

  /**
   * Iteration mark character predicate
   *
   * @param c character to test
   * @return true if c is an iteration mark character.  Otherwise false.
   */
  bool isIterationMark(wchar_t c);

  /**
   * Hiragana iteration mark character predicate
   *
   * @param c character to test
   * @return true if c is a hiragana iteration mark character.  Otherwise false.
   */
  bool isHiraganaIterationMark(wchar_t c);

  /**
   * Katakana iteration mark character predicate
   *
   * @param c character to test
   * @return true if c is a katakana iteration mark character.  Otherwise false.
   */
  bool isKatakanaIterationMark(wchar_t c);

  /**
   * Kanji iteration mark character predicate
   *
   * @param c character to test
   * @return true if c is a kanji iteration mark character.  Otherwise false.
   */
  bool isKanjiIterationMark(wchar_t c);

  /**
   * Look up hiragana dakuten
   *
   * @param c character to look up
   * @return hiragana dakuten variant of c or c itself if no dakuten variant
   * exists
   */
  wchar_t lookupHiraganaDakuten(wchar_t c);

  /**
   * Look up katakana dakuten. Only full-width katakana are supported.
   *
   * @param c character to look up
   * @return katakana dakuten variant of c or c itself if no dakuten variant
   * exists
   */
  wchar_t lookupKatakanaDakuten(wchar_t c);

  /**
   * Hiragana dakuten predicate
   *
   * @param c character to check
   * @return true if c is a hiragana dakuten and otherwise false
   */
  bool isHiraganaDakuten(wchar_t c);

  /**
   * Katakana dakuten predicate
   *
   * @param c character to check
   * @return true if c is a hiragana dakuten and otherwise false
   */
  bool isKatakanaDakuten(wchar_t c);

  /**
   * Looks up a character in dakuten map_obj and returns the dakuten variant if it
   * exists. Otherwise return the character being looked up itself
   *
   * @param c      character to look up
   * @param map_obj    dakuten map_obj
   * @param offset code point offset from c
   * @return mapped character or c if no mapping exists
   */
  wchar_t lookup(wchar_t c, std::deque<wchar_t> &map_obj, wchar_t offset);

  /**
   * Predicate indicating if the lookup character is within dakuten map_obj range
   *
   * @param c      character to look up
   * @param map_obj    dakuten map_obj
   * @param offset code point offset from c
   * @return true if c is mapped by map_obj and otherwise false
   */
  bool inside(wchar_t c, std::deque<wchar_t> &map_obj, wchar_t offset);

protected:
  int correct(int currentOff) override;

protected:
  std::shared_ptr<JapaneseIterationMarkCharFilter> shared_from_this()
  {
    return std::static_pointer_cast<JapaneseIterationMarkCharFilter>(
        org.apache.lucene.analysis.CharFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ja
