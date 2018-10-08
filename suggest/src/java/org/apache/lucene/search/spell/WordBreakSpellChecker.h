#pragma once
#include "stringhelper.h"
#include <limits>
#include <deque>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search::spell
{
class SuggestWord;
}
namespace org::apache::lucene::search::spell
{
class CombineSuggestion;
}
namespace org::apache::lucene::search::spell
{
class SuggestWordArrayWrapper;
}
namespace org::apache::lucene::search::spell
{
class CombineSuggestionWrapper;
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
namespace org::apache::lucene::search::spell
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using SuggestMode = org::apache::lucene::search::spell::SuggestMode;

/**
 * <p>
 * A spell checker whose sole function is to offer suggestions by combining
 * multiple terms into one word and/or breaking terms into multiple words.
 * </p>
 */
class WordBreakSpellChecker
    : public std::enable_shared_from_this<WordBreakSpellChecker>
{
  GET_CLASS_NAME(WordBreakSpellChecker)
private:
  int minSuggestionFrequency = 1;
  int minBreakWordLength = 1;
  int maxCombineWordLength = 20;
  int maxChanges = 1;
  int maxEvaluations = 1000;

  /** Term that can be used to prohibit adjacent terms from being combined */
public:
  static const std::shared_ptr<Term> SEPARATOR_TERM;

  /**
   * Creates a new spellchecker with default configuration values
   * @see #setMaxChanges(int)
   * @see #setMaxCombineWordLength(int)
   * @see #setMaxEvaluations(int)
   * @see #setMinBreakWordLength(int)
   * @see #setMinSuggestionFrequency(int)
   */
  WordBreakSpellChecker();

  /**
   * <p>
   * Determines the order to deque word break suggestions
   * </p>
   */
public:
  enum class BreakSuggestionSortMethod {
    GET_CLASS_NAME(BreakSuggestionSortMethod)
    /**
     * <p>
     * Sort by Number of word breaks, then by the Sum of all the component
     * term's frequencies
     * </p>
     */
    NUM_CHANGES_THEN_SUMMED_FREQUENCY,
    /**
     * <p>
     * Sort by Number of word breaks, then by the Maximum of all the component
     * term's frequencies
     * </p>
     */
    NUM_CHANGES_THEN_MAX_FREQUENCY
  };

  /**
   * <p>
   * Generate suggestions by breaking the passed-in term into multiple words.
   * The scores returned are equal to the number of word breaks needed so a
   * lower score is generally preferred over a higher score.
   * </p>
   *
   * @param suggestMode
   *          - default = {@link SuggestMode#SUGGEST_WHEN_NOT_IN_INDEX}
   * @param sortMethod
   *          - default =
   *          {@link BreakSuggestionSortMethod#NUM_CHANGES_THEN_MAX_FREQUENCY}
   * @return one or more arrays of words formed by breaking up the original term
   * @throws IOException If there is a low-level I/O error.
   */
public:
  virtual std::deque<std::deque<std::shared_ptr<SuggestWord>>>
  suggestWordBreaks(std::shared_ptr<Term> term, int maxSuggestions,
                    std::shared_ptr<IndexReader> ir, SuggestMode suggestMode,
                    BreakSuggestionSortMethod sortMethod) ;

  /**
   * <p>
   * Generate suggestions by combining one or more of the passed-in terms into
   * single words. The returned {@link CombineSuggestion} contains both a
   * {@link SuggestWord} and also an array detailing which passed-in terms were
   * involved in creating this combination. The scores returned are equal to the
   * number of word combinations needed, also one less than the length of the
   * array {@link CombineSuggestion#originalTermIndexes}. Generally, a
   * suggestion with a lower score is preferred over a higher score.
   * </p>
   * <p>
   * To prevent two adjacent terms from being combined (for instance, if one is
   * mandatory and the other is prohibited), separate the two terms with
   * {@link WordBreakSpellChecker#SEPARATOR_TERM}
   * </p>
   * <p>
   * When suggestMode equals {@link SuggestMode#SUGGEST_WHEN_NOT_IN_INDEX}, each
   * suggestion will include at least one term not in the index.
   * </p>
   * <p>
   * When suggestMode equals {@link SuggestMode#SUGGEST_MORE_POPULAR}, each
   * suggestion will have the same, or better frequency than the most-popular
   * included term.
   * </p>
   *
   * @return an array of words generated by combining original terms
   * @throws IOException If there is a low-level I/O error.
   */
  virtual std::deque<std::shared_ptr<CombineSuggestion>>
  suggestWordCombinations(std::deque<std::shared_ptr<Term>> &terms,
                          int maxSuggestions, std::shared_ptr<IndexReader> ir,
                          SuggestMode suggestMode) ;

private:
  int generateBreakUpSuggestions(
      std::shared_ptr<Term> term, std::shared_ptr<IndexReader> ir,
      int numberBreaks, int maxSuggestions, int useMinSuggestionFrequency,
      std::deque<std::shared_ptr<SuggestWord>> &prefix,
      std::deque<std::shared_ptr<SuggestWordArrayWrapper>> &suggestions,
      int totalEvaluations,
      BreakSuggestionSortMethod sortMethod) ;

  std::deque<std::shared_ptr<SuggestWord>>
  newPrefix(std::deque<std::shared_ptr<SuggestWord>> &oldPrefix,
            std::shared_ptr<SuggestWord> append);

  std::deque<std::shared_ptr<SuggestWord>>
  newSuggestion(std::deque<std::shared_ptr<SuggestWord>> &prefix,
                std::shared_ptr<SuggestWord> append1,
                std::shared_ptr<SuggestWord> append2);

  std::shared_ptr<SuggestWord>
  generateSuggestWord(std::shared_ptr<IndexReader> ir,
                      const std::wstring &fieldname,
                      const std::wstring &text) ;

  /**
   * Returns the minimum frequency a term must have
   * to be part of a suggestion.
   * @see #setMinSuggestionFrequency(int)
   */
public:
  virtual int getMinSuggestionFrequency();

  /**
   * Returns the maximum length of a combined suggestion
   * @see #setMaxCombineWordLength(int)
   */
  virtual int getMaxCombineWordLength();

  /**
   * Returns the minimum size of a broken word
   * @see #setMinBreakWordLength(int)
   */
  virtual int getMinBreakWordLength();

  /**
   * Returns the maximum number of changes to perform on the input
   * @see #setMaxChanges(int)
   */
  virtual int getMaxChanges();

  /**
   * Returns the maximum number of word combinations to evaluate.
   * @see #setMaxEvaluations(int)
   */
  virtual int getMaxEvaluations();

  /**
   * <p>
   * The minimum frequency a term must have to be included as part of a
   * suggestion. Default=1 Not applicable when used with
   * {@link SuggestMode#SUGGEST_MORE_POPULAR}
   * </p>
   *
   * @see #getMinSuggestionFrequency()
   */
  virtual void setMinSuggestionFrequency(int minSuggestionFrequency);

  /**
   * <p>
   * The maximum length of a suggestion made by combining 1 or more original
   * terms. Default=20
   * </p>
   *
   * @see #getMaxCombineWordLength()
   */
  virtual void setMaxCombineWordLength(int maxCombineWordLength);

  /**
   * <p>
   * The minimum length to break words down to. Default=1
   * </p>
   *
   * @see #getMinBreakWordLength()
   */
  virtual void setMinBreakWordLength(int minBreakWordLength);

  /**
   * <p>
   * The maximum numbers of changes (word breaks or combinations) to make on the
   * original term(s). Default=1
   * </p>
   *
   * @see #getMaxChanges()
   */
  virtual void setMaxChanges(int maxChanges);

  /**
   * <p>
   * The maximum number of word combinations to evaluate. Default=1000. A higher
   * value might improve result quality. A lower value might improve
   * performance.
   * </p>
   *
   * @see #getMaxEvaluations()
   */
  virtual void setMaxEvaluations(int maxEvaluations);

private:
  class LengthThenMaxFreqComparator
      : public std::enable_shared_from_this<LengthThenMaxFreqComparator>,
        public Comparator<std::shared_ptr<SuggestWordArrayWrapper>>
  {
    GET_CLASS_NAME(LengthThenMaxFreqComparator)
  public:
    int compare(std::shared_ptr<SuggestWordArrayWrapper> o1,
                std::shared_ptr<SuggestWordArrayWrapper> o2) override;
  };

private:
  class LengthThenSumFreqComparator
      : public std::enable_shared_from_this<LengthThenSumFreqComparator>,
        public Comparator<std::shared_ptr<SuggestWordArrayWrapper>>
  {
    GET_CLASS_NAME(LengthThenSumFreqComparator)
  public:
    int compare(std::shared_ptr<SuggestWordArrayWrapper> o1,
                std::shared_ptr<SuggestWordArrayWrapper> o2) override;
  };

private:
  class CombinationsThenFreqComparator
      : public std::enable_shared_from_this<CombinationsThenFreqComparator>,
        public Comparator<std::shared_ptr<CombineSuggestionWrapper>>
  {
    GET_CLASS_NAME(CombinationsThenFreqComparator)
  public:
    int compare(std::shared_ptr<CombineSuggestionWrapper> o1,
                std::shared_ptr<CombineSuggestionWrapper> o2) override;
  };

private:
  class SuggestWordArrayWrapper
      : public std::enable_shared_from_this<SuggestWordArrayWrapper>
  {
    GET_CLASS_NAME(SuggestWordArrayWrapper)
  public:
    std::deque<std::shared_ptr<SuggestWord>> const suggestWords;
    const int freqMax;
    const int freqSum;

    SuggestWordArrayWrapper(
        std::deque<std::shared_ptr<SuggestWord>> &suggestWords);
  };

private:
  class CombineSuggestionWrapper
      : public std::enable_shared_from_this<CombineSuggestionWrapper>
  {
    GET_CLASS_NAME(CombineSuggestionWrapper)
  public:
    const std::shared_ptr<CombineSuggestion> combineSuggestion;
    const int numCombinations;

    CombineSuggestionWrapper(
        std::shared_ptr<CombineSuggestion> combineSuggestion,
        int numCombinations);
  };
};

} // namespace org::apache::lucene::search::spell
