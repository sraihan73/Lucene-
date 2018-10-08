#pragma once
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <string>

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
namespace org::apache::lucene::analysis::br
{

/**
 * A stemmer for Brazilian Portuguese words.
 */
class BrazilianStemmer : public std::enable_shared_from_this<BrazilianStemmer>
{
  GET_CLASS_NAME(BrazilianStemmer)
private:
  static const std::shared_ptr<Locale> locale;

  /**
   * Changed term
   */
  std::wstring TERM;
  std::wstring CT;
  std::wstring R1;
  std::wstring R2;
  std::wstring RV;

public:
  BrazilianStemmer();

  /**
   * Stems the given term to an unique <tt>discriminator</tt>.
   *
   * @param term  The term that should be stemmed.
   * @return      Discriminator for <tt>term</tt>
   */
protected:
  virtual std::wstring stem(const std::wstring &term);

  /**
   * Checks a term if it can be processed correctly.
   *
   * @return  true if, and only if, the given term consists in letters.
   */
private:
  bool isStemmable(const std::wstring &term);

  /**
   * Checks a term if it can be processed indexed.
   *
   * @return  true if it can be indexed
   */
  bool isIndexable(const std::wstring &term);

  /**
   * See if string is 'a','e','i','o','u'
   *
   * @return true if is vowel
   */
  bool isVowel(wchar_t value);

  /**
   * Gets R1
   *
   * R1 - is the region after the first non-vowel following a vowel,
   *      or is the null region at the end of the word if there is
   *      no such non-vowel.
   *
   * @return null or a string representing R1
   */
  std::wstring getR1(const std::wstring &value);

  /**
   * Gets RV
   *
   * RV - IF the second letter is a consonant, RV is the region after
   *      the next following vowel,
   *
   *      OR if the first two letters are vowels, RV is the region
   *      after the next consonant,
   *
   *      AND otherwise (consonant-vowel case) RV is the region after
   *      the third letter.
   *
   *      BUT RV is the end of the word if this positions cannot be
   *      found.
   *
   * @return null or a string representing RV
   */
  std::wstring getRV(const std::wstring &value);

  /**
   * 1) Turn to lowercase
   * 2) Remove accents
   * 3) ã -&gt; a ; õ -&gt; o
   * 4) ç -&gt; c
   *
   * @return null or a string transformed
   */
  std::wstring changeTerm(const std::wstring &value);

  /**
   * Check if a string ends with a suffix
   *
   * @return true if the string ends with the specified suffix
   */
  bool suffix(const std::wstring &value, const std::wstring &suffix);

  /**
   * Replace a string suffix by another
   *
   * @return the replaced std::wstring
   */
  std::wstring replaceSuffix(const std::wstring &value,
                             const std::wstring &toReplace,
                             const std::wstring &changeTo);

  /**
   * Remove a string suffix
   *
   * @return the std::wstring without the suffix
   */
  std::wstring removeSuffix(const std::wstring &value,
                            const std::wstring &toRemove);

  /**
   * See if a suffix is preceded by a std::wstring
   *
   * @return true if the suffix is preceded
   */
  bool suffixPreceded(const std::wstring &value, const std::wstring &suffix,
                      const std::wstring &preceded);

  /**
   * Creates CT (changed term) , substituting * 'ã' and 'õ' for 'a~' and 'o~'.
   */
  void createCT(const std::wstring &term);

  /**
   * Standard suffix removal.
   * Search for the longest among the following suffixes, and perform
   * the following actions:
   *
   * @return false if no ending was removed
   */
  bool step1();

  /**
   * Verb suffixes.
   *
   * Search for the longest among the following suffixes in RV,
   * and if found, delete.
   *
   * @return false if no ending was removed
   */
  bool step2();

  /**
   * Delete suffix 'i' if in RV and preceded by 'c'
   *
   */
  void step3();

  /**
   * Residual suffix
   *
   * If the word ends with one of the suffixes (os a i o á í ó)
   * in RV, delete it
   *
   */
  void step4();

  /**
   * If the word ends with one of ( e é ê) in RV,delete it,
   * and if preceded by 'gu' (or 'ci') with the 'u' (or 'i') in RV,
   * delete the 'u' (or 'i')
   *
   * Or if the word ends ç remove the cedilha
   *
   */
  void step5();

  /**
   * For log and debug purpose
   *
   * @return  TERM, CT, RV, R1 and R2
   */
public:
  virtual std::wstring log();
};

} // #include  "core/src/java/org/apache/lucene/analysis/br/
