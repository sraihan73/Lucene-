#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::search::similarities
{
class TFIDFSimilarity;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::util
{
template <typename T>
class PriorityQueue;
}
namespace org::apache::lucene::queries::mlt
{
class ScoreTerm;
}
namespace org::apache::lucene::queries::mlt
{
class Int;
}
namespace org::apache::lucene::index
{
class Terms;
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
namespace org::apache::lucene::queries::mlt
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using Terms = org::apache::lucene::index::Terms;
using Query = org::apache::lucene::search::Query;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Generate "more like this" similarity queries.
 * Based on this mail:
 * <pre><code>
 * Lucene does let you access the document frequency of terms, with
 * IndexReader.docFreq(). Term frequencies can be computed by re-tokenizing the
 * text, which, for a single document, is usually fast enough.  But looking up
 * the docFreq() of every term in the document is probably too slow.
 *
 * You can use some heuristics to prune the set of terms, to avoid calling
 * docFreq() too much, or at all.  Since you're trying to maximize a tf*idf
 * score, you're probably most interested in terms with a high tf. Choosing a tf
 * threshold even as low as two or three will radically reduce the number of
 * terms under consideration.  Another heuristic is that terms with a high idf
 * (i.e., a low df) tend to be longer.  So you could threshold the terms by the
 * number of characters, not selecting anything less than, e.g., six or seven
 * characters. With these sorts of heuristics you can usually find small set of,
 * e.g., ten or fewer terms that do a pretty good job of characterizing a
 * document.
 *
 * It all depends on what you're trying to do.  If you're trying to eek out that
 * last percent of precision and recall regardless of computational difficulty
 * so that you can win a TREC competition, then the techniques I mention above
 * are useless.  But if you're trying to provide a "more like this" button on a
 * search results page that does a decent job and has good performance, such
 * techniques might be useful.
 *
 * An efficient, effective "more-like-this" query generator would be a great
 * contribution, if anyone's interested.  I'd imagine that it would take a
 * Reader or a std::wstring (the document's text), analyzer Analyzer, and return a set
 * of representative terms using heuristics like those above.  The frequency and
 * length thresholds could be parameters, etc.
 *
 * Doug
 * </code></pre>
 * <h3>Initial Usage</h3>
 * <p>
 * This class has lots of options to try to make it efficient and flexible.
 * The simplest possible usage is as follows. The bold
 * fragment is specific to this class.
 * <br>
 * <pre class="prettyprint">
 * IndexReader ir = ...
 * IndexSearcher is = ...
 *
 * MoreLikeThis mlt = new MoreLikeThis(ir);
 * Reader target = ... // orig source of doc you want to find similarities to
 * Query query = mlt.like( target);
 *
 * Hits hits = is.search(query);
 * // now the usual iteration thru 'hits' - the only thing to watch for is to
 * make sure
 * //you ignore the doc if it matches your 'target' document, as it should be
 * similar to itself
 *
 * </pre>
 * <p>
 * Thus you:
 * <ol>
 * <li> do your normal, Lucene setup for searching,
 * <li> create a MoreLikeThis,
 * <li> get the text of the doc you want to find similarities to
 * <li> then call one of the like() calls to generate a similarity query
 * <li> call the searcher to find the similar docs
 * </ol>
 * <br>
 * <h3>More Advanced Usage</h3>
 * <p>
 * You may want to use {@link #setFieldNames setFieldNames(...)} so you can
 * examine multiple fields (e.g. body and title) for similarity. <p> Depending
 * on the size of your index and the size and makeup of your documents you may
 * want to call the other set methods to control how the similarity queries are
 * generated:
 * <ul>
 * <li> {@link #setMinTermFreq setMinTermFreq(...)}
 * <li> {@link #setMinDocFreq setMinDocFreq(...)}
 * <li> {@link #setMaxDocFreq setMaxDocFreq(...)}
 * <li> {@link #setMaxDocFreqPct setMaxDocFreqPct(...)}
 * <li> {@link #setMinWordLen setMinWordLen(...)}
 * <li> {@link #setMaxWordLen setMaxWordLen(...)}
 * <li> {@link #setMaxQueryTerms setMaxQueryTerms(...)}
 * <li> {@link #setMaxNumTokensParsed setMaxNumTokensParsed(...)}
 * <li> {@link #setStopWords setStopWord(...)}
 * </ul>
 * <br>
 * <hr>
 * <pre>
 * Changes: Mark Harwood 29/02/04
 * Some bugfixing, some refactoring, some optimisation.
 * - bugfix: retrieveTerms(int docNum) was not working for indexes without a
 * termvector -added missing code
 * - bugfix: No significant terms being created for fields with a termvector -
 * because was only counting one occurrence per term/field pair in
 * calculations(ie not including frequency info from TermVector)
 * - refactor: moved common code into isNoiseWord()
 * - optimise: when no termvector support available - used maxNumTermsParsed to
 * limit amount of tokenization
 * </pre>
 */
class MoreLikeThis final : public std::enable_shared_from_this<MoreLikeThis>
{
  GET_CLASS_NAME(MoreLikeThis)

  /**
   * Default maximum number of tokens to parse in each example doc field that is
   * not stored with TermVector support.
   *
   * @see #getMaxNumTokensParsed
   */
public:
  static constexpr int DEFAULT_MAX_NUM_TOKENS_PARSED = 5000;

  /**
   * Ignore terms with less than this frequency in the source doc.
   *
   * @see #getMinTermFreq
   * @see #setMinTermFreq
   */
  static constexpr int DEFAULT_MIN_TERM_FREQ = 2;

  /**
   * Ignore words which do not occur in at least this many docs.
   *
   * @see #getMinDocFreq
   * @see #setMinDocFreq
   */
  static constexpr int DEFAULT_MIN_DOC_FREQ = 5;

  /**
   * Ignore words which occur in more than this many docs.
   *
   * @see #getMaxDocFreq
   * @see #setMaxDocFreq
   * @see #setMaxDocFreqPct
   */
  static const int DEFAULT_MAX_DOC_FREQ = std::numeric_limits<int>::max();

  /**
   * Boost terms in query based on score.
   *
   * @see #isBoost
   * @see #setBoost
   */
  static constexpr bool DEFAULT_BOOST = false;

  /**
   * Default field names. Null is used to specify that the field names should be
   * looked up at runtime from the provided reader.
   */
  static std::deque<std::wstring> const DEFAULT_FIELD_NAMES;

  /**
   * Ignore words less than this length or if 0 then this has no effect.
   *
   * @see #getMinWordLen
   * @see #setMinWordLen
   */
  static constexpr int DEFAULT_MIN_WORD_LENGTH = 0;

  /**
   * Ignore words greater than this length or if 0 then this has no effect.
   *
   * @see #getMaxWordLen
   * @see #setMaxWordLen
   */
  static constexpr int DEFAULT_MAX_WORD_LENGTH = 0;

  /**
   * Default set of stopwords.
   * If null means to allow stop words.
   *
   * @see #setStopWords
   * @see #getStopWords
   */
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: public static final std::unordered_set<?> DEFAULT_STOP_WORDS =
  // null;
  static const std::shared_ptr < Set < ? >> DEFAULT_STOP_WORDS;

  /**
   * Current set of stop words.
   */
private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private std::unordered_set<?> stopWords = DEFAULT_STOP_WORDS;
  std::shared_ptr < Set < ? >> stopWords = DEFAULT_STOP_WORDS;

  /**
   * Return a Query with no more than this many terms.
   *
   * @see BooleanQuery#getMaxClauseCount
   * @see #getMaxQueryTerms
   * @see #setMaxQueryTerms
   */
public:
  static constexpr int DEFAULT_MAX_QUERY_TERMS = 25;

  /**
   * Analyzer that will be used to parse the doc.
   */
private:
  std::shared_ptr<Analyzer> analyzer = nullptr;

  /**
   * Ignore words less frequent that this.
   */
  int minTermFreq = DEFAULT_MIN_TERM_FREQ;

  /**
   * Ignore words which do not occur in at least this many docs.
   */
  int minDocFreq = DEFAULT_MIN_DOC_FREQ;

  /**
   * Ignore words which occur in more than this many docs.
   */
  int maxDocFreq = DEFAULT_MAX_DOC_FREQ;

  /**
   * Should we apply a boost to the Query based on the scores?
   */
  bool boost = DEFAULT_BOOST;

  /**
   * Field name we'll analyze.
   */
  std::deque<std::wstring> fieldNames = DEFAULT_FIELD_NAMES;

  /**
   * The maximum number of tokens to parse in each example doc field that is not
   * stored with TermVector support
   */
  int maxNumTokensParsed = DEFAULT_MAX_NUM_TOKENS_PARSED;

  /**
   * Ignore words if less than this len.
   */
  int minWordLen = DEFAULT_MIN_WORD_LENGTH;

  /**
   * Ignore words if greater than this len.
   */
  int maxWordLen = DEFAULT_MAX_WORD_LENGTH;

  /**
   * Don't return a query longer than this.
   */
  int maxQueryTerms = DEFAULT_MAX_QUERY_TERMS;

  /**
   * For idf() calculations.
   */
  std::shared_ptr<TFIDFSimilarity> similarity; // = new DefaultSimilarity();

  /**
   * IndexReader to use
   */
  const std::shared_ptr<IndexReader> ir;

  /**
   * Boost factor to use when boosting the terms
   */
  float boostFactor = 1;

  /**
   * Returns the boost factor used when boosting terms
   *
   * @return the boost factor used when boosting terms
   * @see #setBoostFactor(float)
   */
public:
  float getBoostFactor();

  /**
   * Sets the boost factor to use when boosting terms
   *
   * @see #getBoostFactor()
   */
  void setBoostFactor(float boostFactor);

  /**
   * Constructor requiring an IndexReader.
   */
  MoreLikeThis(std::shared_ptr<IndexReader> ir);

  MoreLikeThis(std::shared_ptr<IndexReader> ir,
               std::shared_ptr<TFIDFSimilarity> sim);

  std::shared_ptr<TFIDFSimilarity> getSimilarity();

  void setSimilarity(std::shared_ptr<TFIDFSimilarity> similarity);

  /**
   * Returns an analyzer that will be used to parse source doc with. The default
   * analyzer is not set.
   *
   * @return the analyzer that will be used to parse source doc with.
   */
  std::shared_ptr<Analyzer> getAnalyzer();

  /**
   * Sets the analyzer to use. An analyzer is not required for generating a
   * query with the
   * {@link #like(int)} method, all other 'like' methods require an analyzer.
   *
   * @param analyzer the analyzer to use to tokenize text.
   */
  void setAnalyzer(std::shared_ptr<Analyzer> analyzer);

  /**
   * Returns the frequency below which terms will be ignored in the source doc.
   * The default frequency is the {@link #DEFAULT_MIN_TERM_FREQ}.
   *
   * @return the frequency below which terms will be ignored in the source doc.
   */
  int getMinTermFreq();

  /**
   * Sets the frequency below which terms will be ignored in the source doc.
   *
   * @param minTermFreq the frequency below which terms will be ignored in the
   * source doc.
   */
  void setMinTermFreq(int minTermFreq);

  /**
   * Returns the frequency at which words will be ignored which do not occur in
   * at least this many docs. The default frequency is {@link
   * #DEFAULT_MIN_DOC_FREQ}.
   *
   * @return the frequency at which words will be ignored which do not occur in
   * at least this many docs.
   */
  int getMinDocFreq();

  /**
   * Sets the frequency at which words will be ignored which do not occur in at
   * least this many docs.
   *
   * @param minDocFreq the frequency at which words will be ignored which do not
   * occur in at least this many docs.
   */
  void setMinDocFreq(int minDocFreq);

  /**
   * Returns the maximum frequency in which words may still appear.
   * Words that appear in more than this many docs will be ignored. The default
   * frequency is
   * {@link #DEFAULT_MAX_DOC_FREQ}.
   *
   * @return get the maximum frequency at which words are still allowed,
   *         words which occur in more docs than this are ignored.
   */
  int getMaxDocFreq();

  /**
   * Set the maximum frequency in which words may still appear. Words that
   * appear in more than this many docs will be ignored.
   *
   * @param maxFreq the maximum count of documents that a term may appear
   * in to be still considered relevant
   */
  void setMaxDocFreq(int maxFreq);

  /**
   * Set the maximum percentage in which words may still appear. Words that
   * appear in more than this many percent of all docs will be ignored.
   *
   * This method calls {@link #setMaxDocFreq(int)} internally (both conditions
   * cannot be used at the same time).
   *
   * @param maxPercentage the maximum percentage of documents (0-100) that a
   * term may appear in to be still considered relevant.
   */
  void setMaxDocFreqPct(int maxPercentage);

  /**
   * Returns whether to boost terms in query based on "score" or not. The
   * default is
   * {@link #DEFAULT_BOOST}.
   *
   * @return whether to boost terms in query based on "score" or not.
   * @see #setBoost
   */
  bool isBoost();

  /**
   * Sets whether to boost terms in query based on "score" or not.
   *
   * @param boost true to boost terms in query based on "score", false
   * otherwise.
   * @see #isBoost
   */
  void setBoost(bool boost);

  /**
   * Returns the field names that will be used when generating the 'More Like
   * This' query. The default field names that will be used is {@link
   * #DEFAULT_FIELD_NAMES}.
   *
   * @return the field names that will be used when generating the 'More Like
   * This' query.
   */
  std::deque<std::wstring> getFieldNames();

  /**
   * Sets the field names that will be used when generating the 'More Like This'
   * query. Set this to null for the field names to be determined at runtime
   * from the IndexReader provided in the constructor.
   *
   * @param fieldNames the field names that will be used when generating the
   * 'More Like This' query.
   */
  void setFieldNames(std::deque<std::wstring> &fieldNames);

  /**
   * Returns the minimum word length below which words will be ignored. Set this
   * to 0 for no minimum word length. The default is {@link
   * #DEFAULT_MIN_WORD_LENGTH}.
   *
   * @return the minimum word length below which words will be ignored.
   */
  int getMinWordLen();

  /**
   * Sets the minimum word length below which words will be ignored.
   *
   * @param minWordLen the minimum word length below which words will be
   * ignored.
   */
  void setMinWordLen(int minWordLen);

  /**
   * Returns the maximum word length above which words will be ignored. Set this
   * to 0 for no maximum word length. The default is {@link
   * #DEFAULT_MAX_WORD_LENGTH}.
   *
   * @return the maximum word length above which words will be ignored.
   */
  int getMaxWordLen();

  /**
   * Sets the maximum word length above which words will be ignored.
   *
   * @param maxWordLen the maximum word length above which words will be
   * ignored.
   */
  void setMaxWordLen(int maxWordLen);

  /**
   * Set the set of stopwords.
   * Any word in this set is considered "uninteresting" and ignored.
   * Even if your Analyzer allows stopwords, you might want to tell the
   * MoreLikeThis code to ignore them, as for the purposes of document
   * similarity it seems reasonable to assume that "a stop word is never
   * interesting".
   *
   * @param stopWords set of stopwords, if null it means to allow stop words
   * @see #getStopWords
   */
  template <typename T1>
  void setStopWords(std::shared_ptr<Set<T1>> stopWords);

  /**
   * Get the current stop words being used.
   *
   * @see #setStopWords
   */
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: public std::unordered_set<?> getStopWords()
  std::shared_ptr < Set < ? >> getStopWords();

  /**
   * Returns the maximum number of query terms that will be included in any
   * generated query. The default is {@link #DEFAULT_MAX_QUERY_TERMS}.
   *
   * @return the maximum number of query terms that will be included in any
   * generated query.
   */
  int getMaxQueryTerms();

  /**
   * Sets the maximum number of query terms that will be included in any
   * generated query.
   *
   * @param maxQueryTerms the maximum number of query terms that will be
   * included in any generated query.
   */
  void setMaxQueryTerms(int maxQueryTerms);

  /**
   * @return The maximum number of tokens to parse in each example doc field
   * that is not stored with TermVector support
   * @see #DEFAULT_MAX_NUM_TOKENS_PARSED
   */
  int getMaxNumTokensParsed();

  /**
   * @param i The maximum number of tokens to parse in each example doc field
   * that is not stored with TermVector support
   */
  void setMaxNumTokensParsed(int i);

  /**
   * Return a query that will return docs like the passed lucene document ID.
   *
   * @param docNum the documentID of the lucene doc to generate the 'More Like
   * This" query for.
   * @return a query that will return docs like the passed lucene document ID.
   */
  std::shared_ptr<Query> like(int docNum) ;

  /**
   *
   * @param filteredDocument Document with field values extracted for selected
   * fields.
   * @return More Like This query for the passed document.
   */
  std::shared_ptr<Query>
  like(std::unordered_map<std::wstring, std::deque<std::any>>
           &filteredDocument) ;

  /**
   * Return a query that will return docs like the passed Readers.
   * This was added in order to treat multi-value fields.
   *
   * @return a query that will return docs like the passed Readers.
   */
  std::shared_ptr<Query> like(const std::wstring &fieldName,
                              std::deque<Reader> &readers) ;

  /**
   * Create the More like query from a PriorityQueue
   */
private:
  std::shared_ptr<Query>
  createQuery(std::shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> q);

  /**
   * Create a PriorityQueue from a word-&gt;tf map_obj.
   *
   * @param perFieldTermFrequencies a per field map_obj of words keyed on the
   * word(std::wstring) with Int objects as the values.
   */
  std::shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> createQueue(
      std::unordered_map<std::wstring,
                         std::unordered_map<std::wstring, std::shared_ptr<Int>>>
          &perFieldTermFrequencies) ;

  int getTermsCount(
      std::unordered_map<std::wstring,
                         std::unordered_map<std::wstring, std::shared_ptr<Int>>>
          &perFieldTermFrequencies);

  /**
   * Describe the parameters that control how the "more like this" query is
   * formed.
   */
public:
  std::wstring describeParams();

  /**
   * Find words for a more-like-this query former.
   *
   * @param docNum the id of the lucene document from which to find terms
   */
private:
  std::shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>>
  retrieveTerms(int docNum) ;

  std::shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>>
  retrieveTerms(std::unordered_map<std::wstring, std::deque<std::any>>
                    &field2fieldValues) ;
  /**
   * Adds terms and frequencies found in deque into the Map termFreqMap
   *
   * @param field2termFreqMap a Map of terms and their frequencies per field
   * @param deque List of terms and their frequencies for a doc/field
   */
  void addTermFrequencies(
      std::unordered_map<std::wstring,
                         std::unordered_map<std::wstring, std::shared_ptr<Int>>>
          &field2termFreqMap,
      std::shared_ptr<Terms> deque,
      const std::wstring &fieldName) ;

  /**
   * Adds term frequencies found by tokenizing text from reader into the Map
   * words
   *
   * @param r a source of text to be tokenized
   * @param perFieldTermFrequencies a Map of terms and their frequencies per
   * field
   * @param fieldName Used by analyzer for any special per-field analysis
   */
  void addTermFrequencies(
      std::shared_ptr<Reader> r,
      std::unordered_map<std::wstring,
                         std::unordered_map<std::wstring, std::shared_ptr<Int>>>
          &perFieldTermFrequencies,
      const std::wstring &fieldName) ;

  /**
   * determines if the passed term is likely to be of interest in "more like"
   * comparisons
   *
   * @param term The word being considered
   * @return true if should be ignored, false if should be used in further
   * analysis
   */
  bool isNoiseWord(const std::wstring &term);

  /**
   * Find words for a more-like-this query former.
   * The result is a priority queue of arrays with one entry for <b>every
   * word</b> in the document. Each array has 6 elements. The elements are: <ol>
   * <li> The word (std::wstring)
   * <li> The top field that this word comes from (std::wstring)
   * <li> The score for this word (Float)
   * <li> The IDF value (Float)
   * <li> The frequency of this word in the index (Integer)
   * <li> The frequency of this word in the source document (Integer)
   * </ol>
   * This is a somewhat "advanced" routine, and in general only the 1st entry in
   * the array is of interest. This method is exposed so that you can identify
   * the "interesting words" in a document. For an easier method to call see
   * {@link #retrieveInterestingTerms retrieveInterestingTerms()}.
   *
   * @param r the reader that has the content of the document
   * @param fieldName field passed to the analyzer to use when analyzing the
   * content
   * @return the most interesting words in the document ordered by score, with
   * the highest scoring, or best entry, first
   * @see #retrieveInterestingTerms
   */
  std::shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>>
  retrieveTerms(std::shared_ptr<Reader> r,
                const std::wstring &fieldName) ;

  /**
   * @see #retrieveInterestingTerms(java.io.Reader, std::wstring)
   */
public:
  std::deque<std::wstring>
  retrieveInterestingTerms(int docNum) ;

  /**
   * Convenience routine to make it easy to return the most interesting words in
   * a document. More advanced users will call {@link #retrieveTerms(Reader,
   * std::wstring) retrieveTerms()} directly.
   *
   * @param r the source document
   * @param fieldName field passed to analyzer to use when analyzing the content
   * @return the most interesting words in the document
   * @see #retrieveTerms(java.io.Reader, std::wstring)
   * @see #setMaxQueryTerms
   */
  std::deque<std::wstring>
  retrieveInterestingTerms(std::shared_ptr<Reader> r,
                           const std::wstring &fieldName) ;

  /**
   * PriorityQueue that orders words by score.
   */
private:
  class FreqQ : public PriorityQueue<std::shared_ptr<ScoreTerm>>
  {
    GET_CLASS_NAME(FreqQ)
  public:
    FreqQ(int maxSize);

  protected:
    bool lessThan(std::shared_ptr<ScoreTerm> a,
                  std::shared_ptr<ScoreTerm> b) override;

  protected:
    std::shared_ptr<FreqQ> shared_from_this()
    {
      return std::static_pointer_cast<FreqQ>(
          org.apache.lucene.util.PriorityQueue<ScoreTerm>::shared_from_this());
    }
  };

private:
  class ScoreTerm : public std::enable_shared_from_this<ScoreTerm>
  {
    GET_CLASS_NAME(ScoreTerm)
    // only really need 1st 3 entries, other ones are for troubleshooting
  public:
    std::wstring word;
    std::wstring topField;
    float score = 0;
    float idf = 0;
    int docFreq = 0;
    int tf = 0;

    ScoreTerm(const std::wstring &word, const std::wstring &topField,
              float score, float idf, int docFreq, int tf);

    virtual void update(const std::wstring &word, const std::wstring &topField,
                        float score, float idf, int docFreq, int tf);
  };

  /**
   * Use for frequencies and to avoid renewing Integers.
   */
private:
  class Int : public std::enable_shared_from_this<Int>
  {
    GET_CLASS_NAME(Int)
  public:
    int x = 0;

    Int();
  };
};

} // namespace org::apache::lucene::queries::mlt
