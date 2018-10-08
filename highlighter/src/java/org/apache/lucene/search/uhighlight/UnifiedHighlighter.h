#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::util::automaton
{
class CharacterRunAutomaton;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search::uhighlight
{
class PassageScorer;
}
namespace org::apache::lucene::search::uhighlight
{
class PassageFormatter;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::search
{
class TopDocs;
}
namespace org::apache::lucene::search::uhighlight
{
class FieldHighlighter;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search::uhighlight
{
class PhraseHelper;
}
namespace org::apache::lucene::search::uhighlight
{
class FieldOffsetStrategy;
}
namespace org::apache::lucene::search::spans
{
class SpanQuery;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search::uhighlight
{
class LimitedStoredFieldVisitor;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class CacheHelper;
}
namespace org::apache::lucene::index
{
class Fields;
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
namespace org::apache::lucene::search::uhighlight
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TopDocs = org::apache::lucene::search::TopDocs;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * A Highlighter that can get offsets from either
 * postings ({@link IndexOptions#DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS}),
 * term vectors ({@link FieldType#setStoreTermVectorOffsets(bool)}),
 * or via re-analyzing text.
 * <p>
 * This highlighter treats the single original document as the whole corpus, and
 * then scores individual passages as if they were documents in this corpus. It
 * uses a {@link BreakIterator} to find passages in the text; by default it
 * breaks using {@link BreakIterator#getSentenceInstance(Locale)
 * getSentenceInstance(Locale.ROOT)}. It then iterates in parallel (merge
 * sorting by offset) through the positions of all terms from the query,
 * coalescing those hits that occur in a single passage into a {@link Passage},
 * and then scores each Passage using a separate {@link PassageScorer}. Passages
 * are finally formatted into highlighted snippets with a {@link
 * PassageFormatter}. <p> You can customize the behavior by calling some of the
 * setters, or by subclassing and overriding some methods. Some important hooks:
 * <ul>
 * <li>{@link #getBreakIterator(std::wstring)}: Customize how the text is divided into
 * passages. <li>{@link #getScorer(std::wstring)}: Customize how passages are ranked.
 * <li>{@link #getFormatter(std::wstring)}: Customize how snippets are formatted.
 * </ul>
 * <p>
 * This is thread-safe.
 *
 * @lucene.experimental
 */
class UnifiedHighlighter
    : public std::enable_shared_from_this<UnifiedHighlighter>
{
  GET_CLASS_NAME(UnifiedHighlighter)

protected:
  static const wchar_t MULTIVAL_SEP_CHAR = static_cast<wchar_t>(0);

public:
  static constexpr int DEFAULT_MAX_LENGTH = 10000;

  static constexpr int DEFAULT_CACHE_CHARS_THRESHOLD =
      524288; // ~ 1 MB (2 byte chars)

  static const std::shared_ptr<IndexSearcher> EMPTY_INDEXSEARCHER;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static UnifiedHighlighter::StaticConstructor staticConstructor;

protected:
  static std::deque<std::shared_ptr<CharacterRunAutomaton>> const
      ZERO_LEN_AUTOMATA_ARRAY;

  const std::shared_ptr<IndexSearcher>
      searcher; // if null, can only use highlightWithoutSearcher

  const std::shared_ptr<Analyzer> indexAnalyzer;

private:
  bool defaultHandleMtq = true; // e.g. wildcards

  bool defaultHighlightPhrasesStrictly =
      true; // AKA "accuracy" or "query debugging"

  bool defaultPassageRelevancyOverSpeed =
      true; // For analysis, prefer MemoryIndexOffsetStrategy

  int maxLength = DEFAULT_MAX_LENGTH;

  // BreakIterator is stateful so we use a Supplier factory method
  std::function<BreakIterator *()> defaultBreakIterator = [&]() {
    java::text::BreakIterator::getSentenceInstance(java::util::Locale::ROOT);
  };

  std::function<bool(const std::wstring &)> defaultFieldMatcher;

  std::shared_ptr<PassageScorer> defaultScorer =
      std::make_shared<PassageScorer>();

  std::shared_ptr<PassageFormatter> defaultFormatter =
      std::make_shared<DefaultPassageFormatter>();

  int defaultMaxNoHighlightPassages = -1;

  // lazy initialized with double-check locking; protected so subclass can init
protected:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile org.apache.lucene.index.FieldInfos
  // fieldInfos;
  std::shared_ptr<FieldInfos> fieldInfos;

private:
  int cacheFieldValCharsThreshold = DEFAULT_CACHE_CHARS_THRESHOLD;

  /**
   * Calls {@link Weight#extractTerms(Set)} on an empty index for the query.
   */
protected:
  static std::shared_ptr<Set<std::shared_ptr<Term>>>
  extractTerms(std::shared_ptr<Query> query) ;

  /**
   * Constructs the highlighter with the given index searcher and analyzer.
   *
   * @param indexSearcher Usually required, unless {@link
   * #highlightWithoutSearcher(std::wstring, Query, std::wstring, int)} is used, in which
   * case this needs to be null.
   * @param indexAnalyzer Required, even if in some circumstances it isn't used.
   */
public:
  UnifiedHighlighter(std::shared_ptr<IndexSearcher> indexSearcher,
                     std::shared_ptr<Analyzer> indexAnalyzer);

  virtual void setHandleMultiTermQuery(bool handleMtq);

  virtual void setHighlightPhrasesStrictly(bool highlightPhrasesStrictly);

  virtual void setMaxLength(int maxLength);

  virtual void
  setBreakIterator(std::function<BreakIterator *()> &breakIterator);

  virtual void setScorer(std::shared_ptr<PassageScorer> scorer);

  virtual void setFormatter(std::shared_ptr<PassageFormatter> formatter);

  virtual void setMaxNoHighlightPassages(int defaultMaxNoHighlightPassages);

  virtual void setCacheFieldValCharsThreshold(int cacheFieldValCharsThreshold);

  virtual void
  setFieldMatcher(std::function<bool(const std::wstring &)> &predicate);

  /**
   * Returns whether {@link MultiTermQuery} derivatives will be highlighted.  By
   * default it's enabled.  MTQ highlighting can be expensive, particularly when
   * using offsets in postings.
   */
protected:
  virtual bool shouldHandleMultiTermQuery(const std::wstring &field);

  /**
   * Returns whether position sensitive queries (e.g. phrases and {@link
   * SpanQuery}ies) should be highlighted strictly based on query matches
   * (slower) versus any/all occurrences of the underlying terms.  By default
   * it's enabled, but there's no overhead if such queries aren't used.
   */
  virtual bool shouldHighlightPhrasesStrictly(const std::wstring &field);

  virtual bool shouldPreferPassageRelevancyOverSpeed(const std::wstring &field);

  /**
   * Returns the predicate to use for extracting the query part that must be
   * highlighted. By default only queries that target the current field are
   * kept. (AKA requireFieldMatch)
   */
public:
  virtual std::shared_ptr<protected> std::function <
      bool(std::shared_ptr<const> std);

  /**
   * The maximum content size to process.  Content will be truncated to this
   * size before highlighting. Typically snippets closer to the beginning of the
   * document better summarize its content.
   */
  virtual int getMaxLength();

  /**
   * Returns the {@link BreakIterator} to use for
   * dividing text into passages.  This returns
   * {@link BreakIterator#getSentenceInstance(Locale)} by default;
   * subclasses can override to customize.
   * <p>
   * Note: this highlighter will call
   * {@link BreakIterator#preceding(int)} and {@link BreakIterator#next()} many
   * times on it. The default generic JDK implementation of {@code preceding}
   * performs poorly.
   */
protected:
  virtual std::shared_ptr<BreakIterator>
  getBreakIterator(const std::wstring &field);

  /**
   * Returns the {@link PassageScorer} to use for
   * ranking passages.  This
   * returns a new {@code PassageScorer} by default;
   * subclasses can override to customize.
   */
  virtual std::shared_ptr<PassageScorer> getScorer(const std::wstring &field);

  /**
   * Returns the {@link PassageFormatter} to use for
   * formatting passages into highlighted snippets.  This
   * returns a new {@code PassageFormatter} by default;
   * subclasses can override to customize.
   */
  virtual std::shared_ptr<PassageFormatter>
  getFormatter(const std::wstring &field);

  /**
   * Returns the number of leading passages (as delineated by the {@link
   * BreakIterator}) when no highlights could be found.  If it's less than 0
   * (the default) then this defaults to the {@code maxPassages} parameter given
   * for each request.  If this is 0 then the resulting highlight is null (not
   * formatted).
   */
  virtual int getMaxNoHighlightPassages(const std::wstring &field);

  /**
   * Limits the amount of field value pre-fetching until this threshold is
   * passed.  The highlighter internally highlights in batches of documents
   * sized on the sum field value length (in chars) of the fields to be
   * highlighted (bounded by {@link #getMaxLength()} for each field).  By
   * setting this to 0, you can force documents to be fetched and highlighted
   * one at a time, which you usually shouldn't do. The default is 524288 chars
   * which translates to about a megabyte.  However, note that the highlighter
   * sometimes ignores this and highlights one document at a time (without
   * caching a bunch of documents in advance) when it can detect there's no
   * point in it -- such as when all fields will be highlighted via re-analysis
   * as one example.
   */
public:
  virtual int getCacheFieldValCharsThreshold();

  /**
   * ... as passed in from constructor.
   */
  virtual std::shared_ptr<IndexSearcher> getIndexSearcher();

  /**
   * ... as passed in from constructor.
   */
  virtual std::shared_ptr<Analyzer> getIndexAnalyzer();

  /**
   * Source of term offsets; essential for highlighting.
   */
public:
  enum class OffsetSource {
    GET_CLASS_NAME(OffsetSource) POSTINGS,
    TERM_VECTORS,
    ANALYSIS,
    POSTINGS_WITH_TERM_VECTORS,
    NONE_NEEDED
  };

  /**
   * Determine the offset source for the specified field.  The default algorithm
   * is as follows: <ol> <li>This calls {@link #getFieldInfo(std::wstring)}. Note this
   * returns null if there is no searcher or if the field isn't found
   * there.</li> <li> If there's a field info it has
   * {@link IndexOptions#DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS} then {@link
   * OffsetSource#POSTINGS} is returned.</li> <li>If there's a field info and
   * {@link FieldInfo#hasVectors()} then {@link OffsetSource#TERM_VECTORS} is
   * returned (note we can't check here if the TV has offsets; if there isn't
   * then an exception will get thrown down the line).</li> <li>Fall-back:
   * {@link OffsetSource#ANALYSIS} is returned.</li>
   * </ol>
   * <p>
   * Note that the highlighter sometimes switches to something else based on the
   * query, such as if you have
   * {@link OffsetSource#POSTINGS_WITH_TERM_VECTORS} but in fact don't need term
   * vectors.
   */
protected:
  virtual OffsetSource getOffsetSource(const std::wstring &field);

  /**
   * Called by the default implementation of {@link #getOffsetSource(std::wstring)}.
   * If there is no searcher then we simply always return null.
   */
  virtual std::shared_ptr<FieldInfo> getFieldInfo(const std::wstring &field);

  /**
   * Highlights the top passages from a single field.
   *
   * @param field   field name to highlight.
   *                Must have a stored string value and also be indexed with
   * offsets.
   * @param query   query to highlight.
   * @param topDocs TopDocs containing the summary result documents to
   * highlight.
   * @return Array of formatted snippets corresponding to the documents in
   * <code>topDocs</code>. If no highlights were found for a document, the first
   * sentence for the field will be returned.
   * @throws IOException              if an I/O error occurred during processing
   * @throws IllegalArgumentException if <code>field</code> was indexed without
   *                                  {@link
   * IndexOptions#DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS}
   */
public:
  virtual std::deque<std::wstring>
  highlight(const std::wstring &field, std::shared_ptr<Query> query,
            std::shared_ptr<TopDocs> topDocs) ;

  /**
   * Highlights the top-N passages from a single field.
   *
   * @param field       field name to highlight. Must have a stored string
   * value.
   * @param query       query to highlight.
   * @param topDocs     TopDocs containing the summary result documents to
   * highlight.
   * @param maxPassages The maximum number of top-N ranked passages used to
   *                    form the highlighted snippets.
   * @return Array of formatted snippets corresponding to the documents in
   * <code>topDocs</code>. If no highlights were found for a document, the first
   * {@code maxPassages} sentences from the field will be returned.
   * @throws IOException              if an I/O error occurred during processing
   * @throws IllegalArgumentException if <code>field</code> was indexed without
   *                                  {@link
   * IndexOptions#DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS}
   */
  virtual std::deque<std::wstring>
  highlight(const std::wstring &field, std::shared_ptr<Query> query,
            std::shared_ptr<TopDocs> topDocs,
            int maxPassages) ;

  /**
   * Highlights the top passages from multiple fields.
   * <p>
   * Conceptually, this behaves as a more efficient form of:
   * <pre class="prettyprint">
   * Map m = new HashMap();
   * for (std::wstring field : fields) {
   * m.put(field, highlight(field, query, topDocs));
   * }
   * return m;
   * </pre>
   *
   * @param fields  field names to highlight. Must have a stored string value.
   * @param query   query to highlight.
   * @param topDocs TopDocs containing the summary result documents to
   * highlight.
   * @return Map keyed on field name, containing the array of formatted snippets
   * corresponding to the documents in <code>topDocs</code>.
   * If no highlights were found for a document, the
   * first sentence from the field will be returned.
   * @throws IOException              if an I/O error occurred during processing
   * @throws IllegalArgumentException if <code>field</code> was indexed without
   *                                  {@link
   * IndexOptions#DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS}
   */
  virtual std::unordered_map<std::wstring, std::deque<std::wstring>>
  highlightFields(std::deque<std::wstring> &fields,
                  std::shared_ptr<Query> query,
                  std::shared_ptr<TopDocs> topDocs) ;

  /**
   * Highlights the top-N passages from multiple fields.
   * <p>
   * Conceptually, this behaves as a more efficient form of:
   * <pre class="prettyprint">
   * Map m = new HashMap();
   * for (std::wstring field : fields) {
   * m.put(field, highlight(field, query, topDocs, maxPassages));
   * }
   * return m;
   * </pre>
   *
   * @param fields      field names to highlight. Must have a stored string
   * value.
   * @param query       query to highlight.
   * @param topDocs     TopDocs containing the summary result documents to
   * highlight.
   * @param maxPassages The maximum number of top-N ranked passages per-field
   * used to form the highlighted snippets.
   * @return Map keyed on field name, containing the array of formatted snippets
   * corresponding to the documents in <code>topDocs</code>.
   * If no highlights were found for a document, the
   * first {@code maxPassages} sentences from the
   * field will be returned.
   * @throws IOException              if an I/O error occurred during processing
   * @throws IllegalArgumentException if <code>field</code> was indexed without
   *                                  {@link
   * IndexOptions#DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS}
   */
  virtual std::unordered_map<std::wstring, std::deque<std::wstring>>
  highlightFields(std::deque<std::wstring> &fields,
                  std::shared_ptr<Query> query,
                  std::shared_ptr<TopDocs> topDocs,
                  std::deque<int> &maxPassages) ;

  /**
   * Highlights the top-N passages from multiple fields,
   * for the provided int[] docids.
   *
   * @param fieldsIn      field names to highlight. Must have a stored string
   * value.
   * @param query         query to highlight.
   * @param docidsIn      containing the document IDs to highlight.
   * @param maxPassagesIn The maximum number of top-N ranked passages per-field
   * used to form the highlighted snippets.
   * @return Map keyed on field name, containing the array of formatted snippets
   * corresponding to the documents in <code>docidsIn</code>.
   * If no highlights were found for a document, the
   * first {@code maxPassages} from the field will
   * be returned.
   * @throws IOException              if an I/O error occurred during processing
   * @throws IllegalArgumentException if <code>field</code> was indexed without
   *                                  {@link
   * IndexOptions#DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS}
   */
  virtual std::unordered_map<std::wstring, std::deque<std::wstring>>
  highlightFields(std::deque<std::wstring> &fieldsIn,
                  std::shared_ptr<Query> query, std::deque<int> &docidsIn,
                  std::deque<int> &maxPassagesIn) ;

  /**
   * Expert: highlights the top-N passages from multiple fields,
   * for the provided int[] docids, to custom Object as
   * returned by the {@link PassageFormatter}.  Use
   * this API to render to something other than std::wstring.
   *
   * @param fieldsIn      field names to highlight. Must have a stored string
   * value.
   * @param query         query to highlight.
   * @param docIdsIn      containing the document IDs to highlight.
   * @param maxPassagesIn The maximum number of top-N ranked passages per-field
   * used to form the highlighted snippets.
   * @return Map keyed on field name, containing the array of formatted snippets
   * corresponding to the documents in <code>docIdsIn</code>.
   * If no highlights were found for a document, the
   * first {@code maxPassages} from the field will
   * be returned.
   * @throws IOException              if an I/O error occurred during processing
   * @throws IllegalArgumentException if <code>field</code> was indexed without
   *                                  {@link
   * IndexOptions#DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS}
   */
protected:
  virtual std::unordered_map<std::wstring, std::deque<std::any>>
  highlightFieldsAsObjects(std::deque<std::wstring> &fieldsIn,
                           std::shared_ptr<Query> query,
                           std::deque<int> &docIdsIn,
                           std::deque<int> &maxPassagesIn) ;

  /**
   * When cacheCharsThreshold is 0, loadFieldValues() only fetches one document
   * at a time.  We override it to be 0 in two circumstances:
   */
private:
  int calculateOptimalCacheCharsThreshold(int numTermVectors, int numPostings);

  void copyAndSortFieldsWithMaxPassages(std::deque<std::wstring> &fieldsIn,
                                        std::deque<int> &maxPassagesIn,
                                        std::deque<std::wstring> &fields,
                                        std::deque<int> &maxPassages);

private:
  class InPlaceMergeSorterAnonymousInnerClass : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass)
  private:
    std::shared_ptr<UnifiedHighlighter> outerInstance;

    std::deque<std::wstring> fields;
    std::deque<int> maxPassages;

  public:
    InPlaceMergeSorterAnonymousInnerClass(
        std::shared_ptr<UnifiedHighlighter> outerInstance,
        std::deque<std::wstring> &fields, std::deque<int> &maxPassages);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

  protected:
    std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

private:
  void copyAndSortDocIdsWithIndex(std::deque<int> &docIdsIn,
                                  std::deque<int> &docIds,
                                  std::deque<int> &docInIndexes);

private:
  class InPlaceMergeSorterAnonymousInnerClass2 : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass2)
  private:
    std::shared_ptr<UnifiedHighlighter> outerInstance;

    std::deque<int> docIds;
    std::deque<int> docInIndexes;

  public:
    InPlaceMergeSorterAnonymousInnerClass2(
        std::shared_ptr<UnifiedHighlighter> outerInstance,
        std::deque<int> &docIds, std::deque<int> &docInIndexes);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

  protected:
    std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass2>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

  /**
   * Highlights text passed as a parameter.  This requires the {@link
   * IndexSearcher} provided to this highlighter is null.  This use-case is more
   * rare.  Naturally, the mode of operation will be {@link
   * OffsetSource#ANALYSIS}. The result of this method is whatever the {@link
   * PassageFormatter} returns.  For the {@link DefaultPassageFormatter} and
   * assuming {@code content} has non-zero length, the result will be a non-null
   * string -- so it's safe to call {@link Object#toString()} on it in that
   * case.
   *
   * @param field       field name to highlight (as found in the query).
   * @param query       query to highlight.
   * @param content     text to highlight.
   * @param maxPassages The maximum number of top-N ranked passages used to
   *                    form the highlighted snippets.
   * @return result of the {@link PassageFormatter} -- probably a std::wstring.  Might
   * be null.
   * @throws IOException if an I/O error occurred during processing
   */
  // TODO make content a List? and return a List? and ensure getEmptyHighlight
  // is never invoked multiple times?
public:
  virtual std::any highlightWithoutSearcher(const std::wstring &field,
                                            std::shared_ptr<Query> query,
                                            const std::wstring &content,
                                            int maxPassages) ;

protected:
  virtual std::shared_ptr<FieldHighlighter>
  getFieldHighlighter(const std::wstring &field, std::shared_ptr<Query> query,
                      std::shared_ptr<Set<std::shared_ptr<Term>>> allTerms,
                      int maxPassages);

  static std::deque<std::shared_ptr<BytesRef>>
  filterExtractedTerms(std::function<bool(const std::wstring &)> &fieldMatcher,
                       std::shared_ptr<Set<std::shared_ptr<Term>>> queryTerms);

  virtual std::shared_ptr<Set<HighlightFlag>>
  getFlags(const std::wstring &field);

  virtual std::shared_ptr<PhraseHelper>
  getPhraseHelper(const std::wstring &field, std::shared_ptr<Query> query,
                  std::shared_ptr<Set<HighlightFlag>> highlightFlags);

  virtual std::deque<std::shared_ptr<CharacterRunAutomaton>>
  getAutomata(const std::wstring &field, std::shared_ptr<Query> query,
              std::shared_ptr<Set<HighlightFlag>> highlightFlags);

  virtual OffsetSource getOptimizedOffsetSource(
      const std::wstring &field, std::deque<std::shared_ptr<BytesRef>> &terms,
      std::shared_ptr<PhraseHelper> phraseHelper,
      std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata);

  virtual std::shared_ptr<FieldOffsetStrategy> getOffsetStrategy(
      OffsetSource offsetSource, const std::wstring &field,
      std::deque<std::shared_ptr<BytesRef>> &terms,
      std::shared_ptr<PhraseHelper> phraseHelper,
      std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
      std::shared_ptr<Set<HighlightFlag>> highlightFlags);

  /**
   * When highlighting phrases accurately, we need to know which {@link
   * SpanQuery}'s need to have
   * {@link Query#rewrite(IndexReader)} called on them.  It helps performance to
   * avoid it if it's not needed. This method will be invoked on all SpanQuery
   * instances recursively. If you have custom SpanQuery queries then override
   * this to check instanceof and provide a definitive answer. If the query
   * isn't your custom one, simply return null to have the default rules apply,
   * which govern the ones included in Lucene.
   */
  virtual std::optional<bool>
  requiresRewrite(std::shared_ptr<SpanQuery> spanQuery);

  /**
   * When highlighting phrases accurately, we may need to handle custom queries
   * that aren't supported in the
   * {@link org.apache.lucene.search.highlight.WeightedSpanTermExtractor} as
   * called by the {@code PhraseHelper}. Should custom query types be needed,
   * this method should be overriden to return a collection of queries if
   * appropriate, or null if nothing to do. If the query is not custom, simply
   * returning null will allow the default rules to apply.
   *
   * @param query Query to be highlighted
   * @return A std::deque of Query object(s) if needs to be rewritten, otherwise
   * null.
   */
  virtual std::shared_ptr<std::deque<std::shared_ptr<Query>>>
  preSpanQueryRewrite(std::shared_ptr<Query> query);

  /**
   * When dealing with multi term queries / span queries, we may need to handle
   * custom queries that aren't supported by the default automata extraction in
   * {@code MultiTermHighlighting}. This can be overridden to return a
   * collection of queries if appropriate, or null if nothing to do. If query is
   * not custom, simply returning null will allow the default rules to apply.
   *
   * @param query Query to be highlighted
   * @return A std::deque of Query object(s) if needst o be rewritten, otherwise
   * null.
   */
  virtual std::shared_ptr<std::deque<std::shared_ptr<Query>>>
  preMultiTermQueryRewrite(std::shared_ptr<Query> query);

private:
  std::shared_ptr<DocIdSetIterator>
  asDocIdSetIterator(std::deque<int> &sortedDocIds);

private:
  class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
  {
    GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<UnifiedHighlighter> outerInstance;

    std::deque<int> sortedDocIds;

  public:
    DocIdSetIteratorAnonymousInnerClass(
        std::shared_ptr<UnifiedHighlighter> outerInstance,
        std::deque<int> &sortedDocIds);

    int idx = 0;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
          org.apache.lucene.search.DocIdSetIterator::shared_from_this());
    }
  };

  /**
   * Loads the std::wstring values for each docId by field to be highlighted.  By
   * default this loads from stored fields by the same name as given, but a
   * subclass can change the source.  The returned Strings must be identical to
   * what was indexed (at least for postings or term-vectors offset sources).
   * This method must load fields for at least one document from the given
   * {@link DocIdSetIterator} but need not return all of them; by default the
   * character lengths are summed and this method will return early when {@code
   * cacheCharsThreshold} is exceeded.  Specifically if that number is 0, then
   * only one document is fetched no matter what.  Values in the array of {@link
   * std::wstring} will be null if no value was found.
   */
protected:
  virtual std::deque<std::deque<std::shared_ptr<std::wstring>>>
  loadFieldValues(std::deque<std::wstring> &fields,
                  std::shared_ptr<DocIdSetIterator> docIter,
                  int cacheCharsThreshold) ;

  /**
   * @lucene.internal
   */
  virtual std::shared_ptr<LimitedStoredFieldVisitor>
  newLimitedStoredFieldsVisitor(std::deque<std::wstring> &fields);

  /**
   * Fetches stored fields for highlighting. Uses a multi-val separator char and
   * honors a max length to retrieve.
   * @lucene.internal
   */
protected:
  class LimitedStoredFieldVisitor : public StoredFieldVisitor
  {
    GET_CLASS_NAME(LimitedStoredFieldVisitor)
  protected:
    std::deque<std::wstring> const fields;
    const wchar_t valueSeparator;
    const int maxLength;
    std::deque<std::shared_ptr<std::wstring>>
        values; // starts off as std::wstring; may become StringBuilder.
    int currentField = 0;

  public:
    LimitedStoredFieldVisitor(std::deque<std::wstring> &fields,
                              wchar_t valueSeparator, int maxLength);

    virtual void init();

    void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &byteValue)  override;

    Status needsField(std::shared_ptr<FieldInfo> fieldInfo) throw(
        IOException) override;

    virtual std::deque<std::shared_ptr<std::wstring>> getValuesByField();

  protected:
    std::shared_ptr<LimitedStoredFieldVisitor> shared_from_this()
    {
      return std::static_pointer_cast<LimitedStoredFieldVisitor>(
          org.apache.lucene.index.StoredFieldVisitor::shared_from_this());
    }
  };

  /**
   * Wraps an IndexReader that remembers/caches the last call to {@link
   * LeafReader#getTermVectors(int)} so that if the next call has the same ID,
   * then it is reused.  If TV's were column-stride (like doc-values), there
   * would be no need for this.
   */
private:
  class TermVectorReusingLeafReader : public FilterLeafReader
  {
    GET_CLASS_NAME(TermVectorReusingLeafReader)

  public:
    static std::shared_ptr<IndexReader>
    wrap(std::shared_ptr<IndexReader> reader) ;

  private:
    class BaseCompositeReaderAnonymousInnerClass
        : public BaseCompositeReader<std::shared_ptr<IndexReader>>
    {
      GET_CLASS_NAME(BaseCompositeReaderAnonymousInnerClass)
    private:
      std::shared_ptr<IndexReader> reader;

    public:
      BaseCompositeReaderAnonymousInnerClass(
          std::deque<std::shared_ptr<LeafReader>> &leafReaders,
          std::shared_ptr<IndexReader> reader);

    protected:
      void doClose()  override;

    public:
      std::shared_ptr<IndexReader::CacheHelper> getReaderCacheHelper() override;

    protected:
      std::shared_ptr<BaseCompositeReaderAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<BaseCompositeReaderAnonymousInnerClass>(
            org.apache.lucene.index.BaseCompositeReader<
                org.apache.lucene.index.IndexReader>::shared_from_this());
      }
    };

  private:
    int lastDocId = -1;
    std::shared_ptr<Fields> tvFields;

  public:
    TermVectorReusingLeafReader(std::shared_ptr<LeafReader> in_);

    std::shared_ptr<Fields>
    getTermVectors(int docID)  override;

    std::shared_ptr<IndexReader::CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<IndexReader::CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<TermVectorReusingLeafReader> shared_from_this()
    {
      return std::static_pointer_cast<TermVectorReusingLeafReader>(
          org.apache.lucene.index.FilterLeafReader::shared_from_this());
    }
  };

  /**
   * Flags for controlling highlighting behavior.
   */
public:
  enum class HighlightFlag {
    GET_CLASS_NAME(HighlightFlag) PHRASES,
    MULTI_TERM_QUERY,
    PASSAGE_RELEVANCY_OVER_SPEED
    // TODO: ignoreQueryFields
    // TODO: useQueryBoosts
  };
};

} // namespace org::apache::lucene::search::uhighlight
