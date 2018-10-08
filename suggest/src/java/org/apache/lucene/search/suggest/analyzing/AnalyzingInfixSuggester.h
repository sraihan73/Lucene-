#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include "core/src/java/org/apache/lucene/analysis/Analyzer.h"
//#include "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include "core/src/java/org/apache/lucene/store/Directory.h"
#include "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "core/src/java/org/apache/lucene/search/SearcherManager.h"
#include "core/src/java/org/apache/lucene/search/Sort.h"
#include "core/src/java/org/apache/lucene/search/Query.h"
#include "core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "core/src/java/org/apache/lucene/search/BooleanQuery.h"
//#include "core/src/java/org/apache/lucene/search/Builder.h"
#include "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "core/src/java/org/apache/lucene/search/TopFieldDocs.h"
//#include "core/src/java/org/apache/lucene/search/DataOutput.h"
//#include "core/src/java/org/apache/lucene/search/DataInput.h"
//#include "core/src/java/org/apache/lucene/search/Accountable.h" 
//#include "core/src/java/org/apache/lucene/search/suggest/InputIterator.h"
//#include "core/src/java/org/apache/lucene/search/suggest/Lookup.h"
//#include "core/src/java/org/apache/lucene/search/suggest/LookupResult.h"
#include "core/src/java/org/apache/lucene/util/BytesRef.h"
#include "core/src/java/org/apache/lucene/document/FieldType.h"

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
namespace org::apache::lucene::search::suggest::analyzing
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using SearcherManager = org::apache::lucene::search::SearcherManager;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;

// TODO:
//   - a PostingsFormat that stores super-high-freq terms as
//     a bitset should be a win for the prefix terms?
//     (LUCENE-5052)
//   - we could offer a better integration with
//     DocumentDictionary and NRT?  so that your suggester
//     "automatically" keeps in sync w/ your index

/** Analyzes the input text and then suggests matches based
 *  on prefix matches to any tokens in the indexed text.
 *  This also highlights the tokens that match.
 *
 *  <p>This suggester supports payloads.  Matches are sorted only
 *  by the suggest weight; it would be nice to support
 *  blended score + weight sort in the future.  This means
 *  this suggester best applies when there is a strong
 *  a-priori ranking of all the suggestions.
 *
 *  <p>This suggester supports contexts, including arbitrary binary
 *  terms.
 *
 * @lucene.experimental */

class AnalyzingInfixSuggester : public Lookup
{
  GET_CLASS_NAME(AnalyzingInfixSuggester)

  /** edgegrams for searching short prefixes without Prefix Query
   * that's  controlled by {@linkplain #minPrefixChars} */
protected:
  static const std::wstring TEXTGRAMS_FIELD_NAME;

  /** Field name used for the indexed text. */
  static const std::wstring TEXT_FIELD_NAME;

  /** Field name used for the indexed text, as a
   *  StringField, for exact lookup. */
  static const std::wstring EXACT_TEXT_FIELD_NAME;

  /** Field name used for the indexed context, as a
   *  StringField and a SortedSetDVField, for filtering. */
  static const std::wstring CONTEXTS_FIELD_NAME;

  /** Analyzer used at search time */
  const std::shared_ptr<Analyzer> queryAnalyzer;
  /** Analyzer used at index time */
  const std::shared_ptr<Analyzer> indexAnalyzer;

private:
  const std::shared_ptr<Directory> dir;

public:
  const int minPrefixChars;

private:
  const bool allTermsRequired;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool highlight_;

  const bool commitOnBuild;
  const bool closeIndexWriterOnBuild;

  /** Used for ongoing NRT additions/updates. */
protected:
  std::shared_ptr<IndexWriter> writer;

  /** {@link IndexSearcher} used for lookups. */
  std::shared_ptr<SearcherManager> searcherMgr;

  /** Used to manage concurrent access to searcherMgr */
public:
  std::mutex searcherMgrLock;

  /** Default minimum number of leading characters before
   *  PrefixQuery is used (4). */
  static constexpr int DEFAULT_MIN_PREFIX_CHARS = 4;

  /** Default bool clause option for multiple terms matching (all terms
   * required). */
  static constexpr bool DEFAULT_ALL_TERMS_REQUIRED = true;

  /** Default higlighting option. */
  static constexpr bool DEFAULT_HIGHLIGHT = true;

  /** Default option to close the IndexWriter once the index has been built. */
protected:
  static constexpr bool DEFAULT_CLOSE_INDEXWRITER_ON_BUILD = true;

  /** How we sort the postings and search results. */
private:
  static const std::shared_ptr<Sort> SORT;

  /** Create a new instance, loading from a previously built
   *  AnalyzingInfixSuggester directory, if it exists.  This directory must be
   *  private to the infix suggester (i.e., not an external
   *  Lucene index).  Note that {@link #close}
   *  will also close the provided directory. */
public:
  AnalyzingInfixSuggester(
      std::shared_ptr<Directory> dir,
      std::shared_ptr<Analyzer> analyzer) ;

  /** Create a new instance, loading from a previously built
   *  AnalyzingInfixSuggester directory, if it exists.  This directory must be
   *  private to the infix suggester (i.e., not an external
   *  Lucene index).  Note that {@link #close}
   *  will also close the provided directory.
   *
   *  @param minPrefixChars Minimum number of leading characters
   *     before PrefixQuery is used (default 4).
   *     Prefixes shorter than this are indexed as character
   *     ngrams (increasing index size but making lookups
   *     faster).
   *
   *  @param commitOnBuild Call commit after the index has finished building.
   * This would persist the suggester index to disk and future instances of this
   * suggester can use this pre-built dictionary.
   */
  AnalyzingInfixSuggester(std::shared_ptr<Directory> dir,
                          std::shared_ptr<Analyzer> indexAnalyzer,
                          std::shared_ptr<Analyzer> queryAnalyzer,
                          int minPrefixChars,
                          bool commitOnBuild) ;

  /** Create a new instance, loading from a previously built
   *  AnalyzingInfixSuggester directory, if it exists.  This directory must be
   *  private to the infix suggester (i.e., not an external
   *  Lucene index).  Note that {@link #close}
   *  will also close the provided directory.
   *
   *  @param minPrefixChars Minimum number of leading characters
   *     before PrefixQuery is used (default 4).
   *     Prefixes shorter than this are indexed as character
   *     ngrams (increasing index size but making lookups
   *     faster).
   *
   *  @param commitOnBuild Call commit after the index has finished building.
   * This would persist the suggester index to disk and future instances of this
   * suggester can use this pre-built dictionary.
   *
   *  @param allTermsRequired All terms in the suggest query must be matched.
   *  @param highlight Highlight suggest query in suggestions.
   *
   */
  AnalyzingInfixSuggester(std::shared_ptr<Directory> dir,
                          std::shared_ptr<Analyzer> indexAnalyzer,
                          std::shared_ptr<Analyzer> queryAnalyzer,
                          int minPrefixChars, bool commitOnBuild,
                          bool allTermsRequired,
                          bool highlight) ;

  /** Create a new instance, loading from a previously built
   *  AnalyzingInfixSuggester directory, if it exists.  This directory must be
   *  private to the infix suggester (i.e., not an external
   *  Lucene index).  Note that {@link #close}
   *  will also close the provided directory.
   *
   *  @param minPrefixChars Minimum number of leading characters
   *     before PrefixQuery is used (default 4).
   *     Prefixes shorter than this are indexed as character
   *     ngrams (increasing index size but making lookups
   *     faster).
   *
   *  @param commitOnBuild Call commit after the index has finished building.
   * This would persist the suggester index to disk and future instances of this
   * suggester can use this pre-built dictionary.
   *
   *  @param allTermsRequired All terms in the suggest query must be matched.
   *  @param highlight Highlight suggest query in suggestions.
   *  @param closeIndexWriterOnBuild If true, the IndexWriter will be closed
   * after the index has finished building.
   */
  AnalyzingInfixSuggester(std::shared_ptr<Directory> dir,
                          std::shared_ptr<Analyzer> indexAnalyzer,
                          std::shared_ptr<Analyzer> queryAnalyzer,
                          int minPrefixChars, bool commitOnBuild,
                          bool allTermsRequired, bool highlight,
                          bool closeIndexWriterOnBuild) ;

  /** Override this to customize index settings, e.g. which
   *  codec to use. */
protected:
  virtual std::shared_ptr<IndexWriterConfig>
  getIndexWriterConfig(std::shared_ptr<Analyzer> indexAnalyzer,
                       IndexWriterConfig::OpenMode openMode);

  /** Subclass can override to choose a specific {@link
   *  Directory} implementation. */
  virtual std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Path> path) ;

public:
  void build(std::shared_ptr<InputIterator> iter)  override;

  /** Commits all pending changes made to this suggester to disk.
   *
   *  @see IndexWriter#commit */
  virtual void commit() ;

private:
  std::shared_ptr<Analyzer> getGramAnalyzer();

private:
  class AnalyzerWrapperAnonymousInnerClass : public AnalyzerWrapper
  {
    GET_CLASS_NAME(AnalyzerWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<AnalyzingInfixSuggester> outerInstance;

  public:
    AnalyzerWrapperAnonymousInnerClass(
        std::shared_ptr<AnalyzingInfixSuggester> outerInstance,
        std::shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY);

  protected:
    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

    std::shared_ptr<Analyzer::TokenStreamComponents> wrapComponents(
        const std::wstring &fieldName,
        std::shared_ptr<Analyzer::TokenStreamComponents> components) override;

  protected:
    std::shared_ptr<AnalyzerWrapperAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerWrapperAnonymousInnerClass>(
          org.apache.lucene.analysis.AnalyzerWrapper::shared_from_this());
    }
  };

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void ensureOpen() ;

  /** Adds a new suggestion.  Be sure to use {@link #update}
   *  instead if you want to replace a previous suggestion.
   *  After adding or updating a batch of new suggestions,
   *  you must call {@link #refresh} in the end in order to
   *  see the suggestions in {@link #lookup} */
public:
  virtual void add(std::shared_ptr<BytesRef> text,
                   std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                   int64_t weight,
                   std::shared_ptr<BytesRef> payload) ;

  /** Updates a previous suggestion, matching the exact same
   *  text as before.  Use this to change the weight or
   *  payload of an already added suggestion.  If you know
   *  this text is not already present you can use {@link
   *  #add} instead.  After adding or updating a batch of
   *  new suggestions, you must call {@link #refresh} in the
   *  end in order to see the suggestions in {@link #lookup} */
  virtual void update(std::shared_ptr<BytesRef> text,
                      std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                      int64_t weight,
                      std::shared_ptr<BytesRef> payload) ;

private:
  std::shared_ptr<Document>
  buildDocument(std::shared_ptr<BytesRef> text,
                std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                int64_t weight,
                std::shared_ptr<BytesRef> payload) ;

  /** Reopens the underlying searcher; it's best to "batch
   *  up" many additions/updates, and then call refresh
   *  once in the end. */
public:
  virtual void refresh() ;

  /**
   * Subclass can override this method to change the field type of the text
   * field e.g. to change the index options
   */
protected:
  virtual std::shared_ptr<FieldType> getTextFieldType();

public:
  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         bool onlyMorePopular, int num)  override;

  /** Lookup, without any context. */
  virtual std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key, int num, bool allTermsRequired,
         bool doHighlight) ;

  /** Lookup, with context but without booleans. Context booleans default to
   * SHOULD, so each suggestion must have at least one of the contexts. */
  virtual std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts, int num,
         bool allTermsRequired, bool doHighlight) ;

  /** This is called if the last token isn't ended
   *  (e.g. user did not type a space after it).  Return an
   *  appropriate Query clause to add to the BooleanQuery. */
protected:
  virtual std::shared_ptr<Query>
  getLastTokenQuery(const std::wstring &token) ;

  /** Retrieve suggestions, specifying whether all terms
   *  must match ({@code allTermsRequired}) and whether the hits
   *  should be highlighted ({@code doHighlight}). */
public:
  virtual std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::unordered_map<std::shared_ptr<BytesRef>, Occur> &contextInfo,
         int num, bool allTermsRequired, bool doHighlight) ;

private:
  std::shared_ptr<BooleanQuery>
  toQuery(std::unordered_map<std::shared_ptr<BytesRef>, Occur> &contextInfo);

  std::shared_ptr<BooleanQuery>
  toQuery(std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contextInfo);

  /**
   * This method is handy as we do not need access to internal fields such as
   * CONTEXTS_FIELD_NAME in order to build queries However, here may not be its
   * best location.
   *
   * @param query an instance of @See {@link BooleanQuery}
   * @param context the context
   * @param clause one of {@link Occur}
   */
public:
  virtual void addContextToQuery(std::shared_ptr<BooleanQuery::Builder> query,
                                 std::shared_ptr<BytesRef> context,
                                 Occur clause);

  /**
   * This is an advanced method providing the capability to send down to the
   * suggester any arbitrary lucene query to be used to filter the result of the
   * suggester
   *
   * @param key the keyword being looked for
   * @param contextQuery an arbitrary Lucene query to be used to filter the
   * result of the suggester. {@link #addContextToQuery} could be used to build
   * this contextQuery.
   * @param num number of items to return
   * @param allTermsRequired all searched terms must match or not
   * @param doHighlight if true, the matching term will be highlighted in the
   * search result
   * @return the result of the suggester
   * @throws IOException f the is IO exception while reading data from the index
   */
  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<BooleanQuery> contextQuery, int num,
         bool allTermsRequired, bool doHighlight)  override;

  /**
   * Create the results based on the search hits.
   * Can be overridden by subclass to add particular behavior (e.g. weight
transformation).
   * Note that there is no prefix toke (the {@code prefixToken} argument will
GET_CLASS_NAME(to)
   * be null) whenever the final token in the incoming request was in fact
finished
   * (had trailing characters, such as white-space).
   *
   * @throws IOException If there are problems reading fields from the
underlying Lucene index.
   */
protected:
  virtual std::deque<std::shared_ptr<Lookup::LookupResult>>
  createResults(std::shared_ptr<IndexSearcher> searcher,
                std::shared_ptr<TopFieldDocs> hits, int num,
                std::shared_ptr<std::wstring> charSequence, bool doHighlight,
                std::shared_ptr<Set<std::wstring>> matchedTokens,
                const std::wstring &prefixToken) ;

  /** Subclass can override this to tweak the Query before
   *  searching. */
  virtual std::shared_ptr<Query>
  finishQuery(std::shared_ptr<BooleanQuery::Builder> in_,
              bool allTermsRequired);

  /** Override this method to customize the Object
   *  representing a single highlighted suggestions; the
   *  result is set on each {@link
   *  org.apache.lucene.search.suggest.Lookup.LookupResult#highlightKey} member.
   */
  virtual std::any
  highlight(const std::wstring &text,
            std::shared_ptr<Set<std::wstring>> matchedTokens,
            const std::wstring &prefixToken) ;

  /** Called while highlighting a single result, to append a
   *  non-matching chunk of text from the suggestion to the
   *  provided fragments deque.
   *  @param sb The {@code StringBuilder} to append to
   *  @param text The text chunk to add
   */
  virtual void addNonMatch(std::shared_ptr<StringBuilder> sb,
                           const std::wstring &text);

  /** Called while highlighting a single result, to append
   *  the whole matched token to the provided fragments deque.
   *  @param sb The {@code StringBuilder} to append to
   *  @param surface The surface form (original) text
   *  @param analyzed The analyzed token corresponding to the surface form text
   */
  virtual void addWholeMatch(std::shared_ptr<StringBuilder> sb,
                             const std::wstring &surface,
                             const std::wstring &analyzed);

  /** Called while highlighting a single result, to append a
   *  matched prefix token, to the provided fragments deque.
   *  @param sb The {@code StringBuilder} to append to
   *  @param surface The fragment of the surface form
   *        (indexed during {@link #build}, corresponding to
   *        this match
   *  @param analyzed The analyzed token that matched
   *  @param prefixToken The prefix of the token that matched
   */
  virtual void addPrefixMatch(std::shared_ptr<StringBuilder> sb,
                              const std::wstring &surface,
                              const std::wstring &analyzed,
                              const std::wstring &prefixToken);

public:
  bool store(std::shared_ptr<DataOutput> in_)  override;

  bool load(std::shared_ptr<DataInput> out)  override;

  virtual ~AnalyzingInfixSuggester();

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  int64_t getCount()  override;

protected:
  std::shared_ptr<AnalyzingInfixSuggester> shared_from_this()
  {
    return std::static_pointer_cast<AnalyzingInfixSuggester>(
        org.apache.lucene.search.suggest.Lookup::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/analyzing/
