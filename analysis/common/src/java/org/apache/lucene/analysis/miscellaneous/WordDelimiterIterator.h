#pragma once
#include "stringhelper.h"
#include <cctype>
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
namespace org::apache::lucene::analysis::miscellaneous
{

/**
 * A BreakIterator-like API for iterating over subwords in text, according to
 * WordDelimiterGraphFilter rules.
 * @lucene.internal
 */
class WordDelimiterIterator final
    : public std::enable_shared_from_this<WordDelimiterIterator>
{
  GET_CLASS_NAME(WordDelimiterIterator)

public:
  static constexpr int LOWER = 0x01;
  static constexpr int UPPER = 0x02;
  static constexpr int DIGIT = 0x04;
  static constexpr int SUBWORD_DELIM = 0x08;

  // combinations: for testing, not for setting bits
  static constexpr int ALPHA = 0x03;
  static constexpr int ALPHANUM = 0x07;

  /** Indicates the end of iteration */
  static constexpr int DONE = -1;

  static std::deque<char> const DEFAULT_WORD_DELIM_TABLE;

  std::deque<wchar_t> text;
  int length = 0;

  /** start position of text, excluding leading delimiters */
  int startBounds = 0;
  /** end position of text, excluding trailing delimiters */
  int endBounds = 0;

  /** Beginning of subword */
  int current = 0;
  /** End of subword */
  int end = 0;

  /* does this string end with a possessive such as 's */
private:
  bool hasFinalPossessive = false;

  /**
   * If false, causes case changes to be ignored (subwords will only be
   * generated given SUBWORD_DELIM tokens). (Defaults to true)
   */
public:
  const bool splitOnCaseChange;

  /**
   * If false, causes numeric changes to be ignored (subwords will only be
   * generated given SUBWORD_DELIM tokens). (Defaults to true)
   */
  const bool splitOnNumerics;

  /**
   * If true, causes trailing "'s" to be removed for each subword. (Defaults to
   * true) <p/> "O'Neil's" =&gt; "O", "Neil"
   */
  const bool stemEnglishPossessive;

private:
  std::deque<char> const charTypeTable;

  /** if true, need to skip over a possessive found in the last call to next()
   */
  bool skipPossessive = false;

  // TODO: should there be a WORD_DELIM category for chars that only separate
  // words (no catenation of subwords will be done if separated by these chars?)
  // "," would be an obvious candidate...
private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static WordDelimiterIterator::StaticConstructor staticConstructor;

  /**
   * Create a new WordDelimiterIterator operating with the supplied rules.
   *
   * @param charTypeTable table containing character types
   * @param splitOnCaseChange if true, causes "PowerShot" to be two tokens;
   * ("Power-Shot" remains two parts regardless)
   * @param splitOnNumerics if true, causes "j2se" to be three tokens; "j" "2"
   * "se"
   * @param stemEnglishPossessive if true, causes trailing "'s" to be removed
   * for each subword: "O'Neil's" =&gt; "O", "Neil"
   */
public:
  WordDelimiterIterator(std::deque<char> &charTypeTable,
                        bool splitOnCaseChange, bool splitOnNumerics,
                        bool stemEnglishPossessive);

  /**
   * Advance to the next subword in the string.
   *
   * @return index of the next subword, or {@link #DONE} if all subwords have
   * been returned
   */
  int next();

  /**
   * Return the type of the current subword.
   * This currently uses the type of the first character in the subword.
   *
   * @return type of the current word
   */
  int type();

  /**
   * Reset the text to a new value, and reset all state
   *
   * @param text New text
   * @param length length of the text
   */
  void setText(std::deque<wchar_t> &text, int length);

  // ================================================= Helper Methods
  // ================================================

  /**
   * Determines whether the transition from lastType to type indicates a break
   *
   * @param lastType Last subword type
   * @param type Current subword type
   * @return {@code true} if the transition indicates a break, {@code false}
   * otherwise
   */
private:
  bool isBreak(int lastType, int type);

  /**
   * Determines if the current word contains only one subword.  Note, it could
   * be potentially surrounded by delimiters
   *
   * @return {@code true} if the current word contains only one subword, {@code
   * false} otherwise
   */
public:
  bool isSingleWord();

  /**
   * Set the internal word bounds (remove leading and trailing delimiters).
   * Note, if a possessive is found, don't remove it yet, simply note it.
   */
private:
  void setBounds();

  /**
   * Determines if the text at the given position indicates an English
   * possessive which should be removed
   *
   * @param pos Position in the text to check if it indicates an English
   * possessive
   * @return {@code true} if the text at the position indicates an English
   * posessive, {@code false} otherwise
   */
  bool endsWithPossessive(int pos);

  /**
   * Determines the type of the given character
   *
   * @param ch Character whose type is to be determined
   * @return Type of the character
   */
  int charType(int ch);

  /**
   * Computes the type of the given character
   *
   * @param ch Character whose type is to be determined
   * @return Type of the character
   */
public:
  static char getType(int ch);

  /**
   * Checks if the given word type includes {@link #ALPHA}
   *
   * @param type Word type to check
   * @return {@code true} if the type contains ALPHA, {@code false} otherwise
   */
  static bool isAlpha(int type);

  /**
   * Checks if the given word type includes {@link #DIGIT}
   *
   * @param type Word type to check
   * @return {@code true} if the type contains DIGIT, {@code false} otherwise
   */
  static bool isDigit(int type);

  /**
   * Checks if the given word type includes {@link #SUBWORD_DELIM}
   *
   * @param type Word type to check
   * @return {@code true} if the type contains SUBWORD_DELIM, {@code false}
   * otherwise
   */
  static bool isSubwordDelim(int type);

  /**
   * Checks if the given word type includes {@link #UPPER}
   *
   * @param type Word type to check
   * @return {@code true} if the type contains UPPER, {@code false} otherwise
   */
  static bool isUpper(int type);
};

} // namespace org::apache::lucene::analysis::miscellaneous
