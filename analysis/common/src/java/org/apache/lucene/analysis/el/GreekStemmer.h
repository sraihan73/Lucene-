#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
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
namespace org::apache::lucene::analysis::el
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;

/**
 * A stemmer for Greek words, according to: <i>Development of a Stemmer for the
 * Greek Language.</i> Georgios Ntais
 * <p>
 * NOTE: Input is expected to be casefolded for Greek (including folding of
 * final sigma to sigma), and with diacritics removed. This can be achieved with
 * either {@link GreekLowerCaseFilter} or ICUFoldingFilter.
 * @lucene.experimental
 */
class GreekStemmer : public std::enable_shared_from_this<GreekStemmer>
{
  GET_CLASS_NAME(GreekStemmer)

  /**
   * Stems a word contained in a leading portion of a char[] array.
   * The word is passed through a number of rules that modify its length.
   *
   * @param s A char[] array that contains the word to be stemmed.
   * @param len The length of the char[] array.
   * @return The new length of the stemmed word.
   */
public:
  virtual int stem(std::deque<wchar_t> &s, int len);

private:
  int rule0(std::deque<wchar_t> &s, int len);

  int rule1(std::deque<wchar_t> &s, int len);

  int rule2(std::deque<wchar_t> &s, int len);

  int rule3(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc4;

  int rule4(std::deque<wchar_t> &s, int len);

  int rule5(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc6;

  int rule6(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc7;

  int rule7(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc8a;

  static const std::shared_ptr<CharArraySet> exc8b;

  int rule8(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc9;

  int rule9(std::deque<wchar_t> &s, int len);

  int rule10(std::deque<wchar_t> &s, int len);

  int rule11(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc12a;

  static const std::shared_ptr<CharArraySet> exc12b;

  int rule12(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc13;

  int rule13(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc14;

  int rule14(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc15a;

  static const std::shared_ptr<CharArraySet> exc15b;

  int rule15(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc16;

  int rule16(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc17;

  int rule17(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc18;

  int rule18(std::deque<wchar_t> &s, int len);

  static const std::shared_ptr<CharArraySet> exc19;

  int rule19(std::deque<wchar_t> &s, int len);

  int rule20(std::deque<wchar_t> &s, int len);

  int rule21(std::deque<wchar_t> &s, int len);

  int rule22(std::deque<wchar_t> &s, int len);

  /**
   * Checks if the word contained in the leading portion of char[] array ,
   * ends with the suffix given as parameter.
   *
   * @param s A char[] array that represents a word.
   * @param len The length of the char[] array.
   * @param suffix A {@link std::wstring} object to check if the word given ends with
   * these characters.
   * @return True if the word ends with the suffix given , false otherwise.
   */
  bool endsWith(std::deque<wchar_t> &s, int len, const std::wstring &suffix);

  /**
   * Checks if the word contained in the leading portion of char[] array ,
   * ends with a Greek vowel.
   *
   * @param s A char[] array that represents a word.
   * @param len The length of the char[] array.
   * @return True if the word contained in the leading portion of char[] array ,
   * ends with a vowel , false otherwise.
   */
  bool endsWithVowel(std::deque<wchar_t> &s, int len);

  /**
   * Checks if the word contained in the leading portion of char[] array ,
   * ends with a Greek vowel.
   *
   * @param s A char[] array that represents a word.
   * @param len The length of the char[] array.
   * @return True if the word contained in the leading portion of char[] array ,
   * ends with a vowel , false otherwise.
   */
  bool endsWithVowelNoY(std::deque<wchar_t> &s, int len);
};

} // namespace org::apache::lucene::analysis::el
