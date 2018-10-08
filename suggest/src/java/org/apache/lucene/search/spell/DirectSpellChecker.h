#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spell
{
class StringDistance;
}

namespace org::apache::lucene::search::spell
{
class SuggestWord;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::util
{
class CharsRefBuilder;
}
namespace org::apache::lucene::search::spell
{
class ScoreTerm;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;

/**
 * Simple automaton-based spellchecker.
 * <p>
 * Candidates are presented directly from the term dictionary, based on
 * Levenshtein distance. This is an alternative to {@link SpellChecker}
 * if you are using an edit-distance-like metric such as Levenshtein
 * or {@link JaroWinklerDistance}.
 * <p>
 * A practical benefit of this spellchecker is that it requires no additional
 * datastructures (neither in RAM nor on disk) to do its work.
 *
 * @see LevenshteinAutomata
 * @see FuzzyTermsEnum
 *
 * @lucene.experimental
 */
class DirectSpellChecker
    : public std::enable_shared_from_this<DirectSpellChecker>
{
  GET_CLASS_NAME(DirectSpellChecker)
  /** The default StringDistance, Damerau-Levenshtein distance implemented
   * internally via {@link LevenshteinAutomata}. <p> Note: this is the fastest
   * distance metric, because Damerau-Levenshtein is used to draw candidates
   * from the term dictionary: this just re-uses the scoring.
   */
public:
  static const std::shared_ptr<StringDistance> INTERNAL_LEVENSHTEIN;

  /** maximum edit distance for candidate terms */
private:
  int maxEdits = LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE;
  /** minimum prefix for candidate terms */
  int minPrefix = 1;
  /** maximum number of top-N inspections per suggestion */
  int maxInspections = 5;
  /** minimum accuracy for a term to match */
  float accuracy = SpellChecker::DEFAULT_ACCURACY;
  /** value in [0..1] (or absolute number &gt;= 1) representing the minimum
   * number of documents (of the total) where a term should appear. */
  float thresholdFrequency = 0.0f;
  /** minimum length of a query word to return suggestions */
  int minQueryLength = 4;
  /** value in [0..1] (or absolute number &gt;= 1) representing the maximum
   *  number of documents (of the total) a query term can appear in to
   *  be corrected. */
  float maxQueryFrequency = 0.01f;
  /** true if the spellchecker should lowercase terms */
  bool lowerCaseTerms = true;
  /** the comparator to use */
  std::shared_ptr<Comparator<std::shared_ptr<SuggestWord>>> comparator =
      SuggestWordQueue::DEFAULT_COMPARATOR;
  /** the string distance to use */
  std::shared_ptr<StringDistance> distance = INTERNAL_LEVENSHTEIN;

  /** Creates a DirectSpellChecker with default configuration values */
public:
  DirectSpellChecker();

  /** Get the maximum number of Levenshtein edit-distances to draw
   *  candidate terms from. */
  virtual int getMaxEdits();

  /** Sets the maximum number of Levenshtein edit-distances to draw
   *  candidate terms from. This value can be 1 or 2. The default is 2.
   *  <p>
   *  Note: a large number of spelling errors occur with an edit distance
   *  of 1, by setting this value to 1 you can increase both performance
   *  and precision at the cost of recall.
   */
  virtual void setMaxEdits(int maxEdits);

  /**
   * Get the minimal number of characters that must match exactly
   */
  virtual int getMinPrefix();

  /**
   * Sets the minimal number of initial characters (default: 1)
   * that must match exactly.
   * <p>
   * This can improve both performance and accuracy of results,
   * as misspellings are commonly not the first character.
   */
  virtual void setMinPrefix(int minPrefix);

  /**
   * Get the maximum number of top-N inspections per suggestion
   */
  virtual int getMaxInspections();

  /**
   * Set the maximum number of top-N inspections (default: 5) per suggestion.
   * <p>
   * Increasing this number can improve the accuracy of results, at the cost
   * of performance.
   */
  virtual void setMaxInspections(int maxInspections);

  /**
   * Get the minimal accuracy from the StringDistance for a match
   */
  virtual float getAccuracy();

  /**
   * Set the minimal accuracy required (default: 0.5f) from a StringDistance
   * for a suggestion match.
   */
  virtual void setAccuracy(float accuracy);

  /**
   * Get the minimal threshold of documents a term must appear for a match
   */
  virtual float getThresholdFrequency();

  /**
   * Set the minimal threshold of documents a term must appear for a match.
   * <p>
   * This can improve quality by only suggesting high-frequency terms. Note that
   * very high values might decrease performance slightly, by forcing the
   * spellchecker to draw more candidates from the term dictionary, but a
   * practical value such as <code>1</code> can be very useful towards improving
   * quality. <p> This can be specified as a relative percentage of documents
   * such as 0.5f, or it can be specified as an absolute whole document
   * frequency, such as 4f. Absolute document frequencies may not be fractional.
   */
  virtual void setThresholdFrequency(float thresholdFrequency);

  /** Get the minimum length of a query term needed to return suggestions */
  virtual int getMinQueryLength();

  /**
   * Set the minimum length of a query term (default: 4) needed to return
   * suggestions. <p> Very short query terms will often cause only bad
   * suggestions with any distance metric.
   */
  virtual void setMinQueryLength(int minQueryLength);

  /**
   * Get the maximum threshold of documents a query term can appear in order
   * to provide suggestions.
   */
  virtual float getMaxQueryFrequency();

  /**
   * Set the maximum threshold (default: 0.01f) of documents a query term can
   * appear in order to provide suggestions.
   * <p>
   * Very high-frequency terms are typically spelled correctly. Additionally,
   * this can increase performance as it will do no work for the common case
   * of correctly-spelled input terms.
   * <p>
   * This can be specified as a relative percentage of documents such as 0.5f,
   * or it can be specified as an absolute whole document frequency, such as 4f.
   * Absolute document frequencies may not be fractional.
   */
  virtual void setMaxQueryFrequency(float maxQueryFrequency);

  /** true if the spellchecker should lowercase terms */
  virtual bool getLowerCaseTerms();

  /**
   * True if the spellchecker should lowercase terms (default: true)
   * <p>
   * This is a convenience method, if your index field has more complicated
   * analysis (such as StandardTokenizer removing punctuation), it's probably
   * better to turn this off, and instead run your query terms through your
   * Analyzer first.
   * <p>
   * If this option is not on, case differences count as an edit!
   */
  virtual void setLowerCaseTerms(bool lowerCaseTerms);

  /**
   * Get the current comparator in use.
   */
  virtual std::shared_ptr<Comparator<std::shared_ptr<SuggestWord>>>
  getComparator();

  /**
   * Set the comparator for sorting suggestions.
   * The default is {@link SuggestWordQueue#DEFAULT_COMPARATOR}
   */
  virtual void setComparator(
      std::shared_ptr<Comparator<std::shared_ptr<SuggestWord>>> comparator);

  /**
   * Get the string distance metric in use.
   */
  virtual std::shared_ptr<StringDistance> getDistance();

  /**
   * Set the string distance metric.
   * The default is {@link #INTERNAL_LEVENSHTEIN}
   * <p>
   * Note: because this spellchecker draws its candidates from the term
   * dictionary using Damerau-Levenshtein, it works best with an
   * edit-distance-like string metric. If you use a different metric than the
   * default, you might want to consider increasing {@link
   * #setMaxInspections(int)} to draw more candidates for your metric to rank.
   */
  virtual void setDistance(std::shared_ptr<StringDistance> distance);

  /**
   * Calls {@link #suggestSimilar(Term, int, IndexReader, SuggestMode)
   *       suggestSimilar(term, numSug, ir,
   * SuggestMode.SUGGEST_WHEN_NOT_IN_INDEX)}
   */
  virtual std::deque<std::shared_ptr<SuggestWord>>
  suggestSimilar(std::shared_ptr<Term> term, int numSug,
                 std::shared_ptr<IndexReader> ir) ;

  /**
   * Calls {@link #suggestSimilar(Term, int, IndexReader, SuggestMode, float)
   *       suggestSimilar(term, numSug, ir, suggestMode, this.accuracy)}
   *
   */
  virtual std::deque<std::shared_ptr<SuggestWord>>
  suggestSimilar(std::shared_ptr<Term> term, int numSug,
                 std::shared_ptr<IndexReader> ir,
                 SuggestMode suggestMode) ;

  /**
   * Suggest similar words.
   *
   * <p>Unlike {@link SpellChecker}, the similarity used to fetch the most
   * relevant terms is an edit distance, therefore typically a low value
   * for numSug will work very well.
   *
   * @param term Term you want to spell check on
   * @param numSug the maximum number of suggested words
   * @param ir IndexReader to find terms from
   * @param suggestMode specifies when to return suggested words
   * @param accuracy return only suggested words that match with this similarity
   * @return sorted deque of the suggested words according to the comparator
   * @throws IOException If there is a low-level I/O error.
   */
  virtual std::deque<std::shared_ptr<SuggestWord>>
  suggestSimilar(std::shared_ptr<Term> term, int numSug,
                 std::shared_ptr<IndexReader> ir, SuggestMode suggestMode,
                 float accuracy) ;

  /**
   * Provide spelling corrections based on several parameters.
   *
   * @param term The term to suggest spelling corrections for
   * @param numSug The maximum number of spelling corrections
   * @param ir The index reader to fetch the candidate spelling corrections from
   * @param docfreq The minimum document frequency a potential suggestion need
   * to have in order to be included
   * @param editDistance The maximum edit distance candidates are allowed to
   * have
   * @param accuracy The minimum accuracy a suggested spelling correction needs
   * to have in order to be included
   * @param spare a chars scratch
   * @return a collection of spelling corrections sorted by
   * <code>ScoreTerm</code>'s natural order.
   * @throws IOException If I/O related errors occur
   */
protected:
  virtual std::shared_ptr<std::deque<std::shared_ptr<ScoreTerm>>>
  suggestSimilar(std::shared_ptr<Term> term, int numSug,
                 std::shared_ptr<IndexReader> ir, int docfreq, int editDistance,
                 float accuracy,
                 std::shared_ptr<CharsRefBuilder> spare) ;

  /**
   * Holds a spelling correction for internal usage inside {@link
   * DirectSpellChecker}.
   */
protected:
  class ScoreTerm : public std::enable_shared_from_this<ScoreTerm>,
                    public Comparable<std::shared_ptr<ScoreTerm>>
  {
    GET_CLASS_NAME(ScoreTerm)

    /**
     * The actual spellcheck correction.
     */
  public:
    std::shared_ptr<BytesRef> term;

    /**
     * The boost representing the similarity from the FuzzyTermsEnum (internal
     * similarity score)
     */
    float boost = 0;

    /**
     * The df of the spellcheck correction.
     */
    int docfreq = 0;

    /**
     * The spellcheck correction represented as string, can be
     * <code>null</code>.
     */
    std::wstring termAsString;

    /**
     * The similarity score.
     */
    float score = 0;

    /**
     * Constructor.
     */
    ScoreTerm();

    int compareTo(std::shared_ptr<ScoreTerm> other) override;

    virtual int hashCode();

    bool equals(std::any obj) override;
  };
};

} // namespace org::apache::lucene::search::spell
