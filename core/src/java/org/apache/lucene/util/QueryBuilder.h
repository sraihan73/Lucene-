#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/BooleanClause.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/Builder.h"
#include  "core/src/java/org/apache/lucene/search/MultiPhraseQuery.h"

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
namespace org::apache::lucene::util
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

/**
 * Creates queries from the {@link Analyzer} chain.
 * <p>
 * Example usage:
 * <pre class="prettyprint">
 *   QueryBuilder builder = new QueryBuilder(analyzer);
 *   Query a = builder.createBooleanQuery("body", "just a test");
 *   Query b = builder.createPhraseQuery("body", "another test");
 *   Query c = builder.createMinShouldMatchQuery("body", "another test", 0.5f);
 * </pre>
 * <p>
 * This can also be used as a subclass for query parsers to make it easier
 * to interact with the analysis chain. Factory methods such as {@code
newTermQuery} GET_CLASS_NAME(for)
 * are provided so that the generated queries can be customized.
 */
class QueryBuilder : public std::enable_shared_from_this<QueryBuilder>
{
  GET_CLASS_NAME(QueryBuilder)
protected:
  std::shared_ptr<Analyzer> analyzer;
  bool enablePositionIncrements = true;
  bool enableGraphQueries = true;
  bool autoGenerateMultiTermSynonymsPhraseQuery = false;

  /** Creates a new QueryBuilder using the given analyzer. */
public:
  QueryBuilder(std::shared_ptr<Analyzer> analyzer);

  /**
   * Creates a bool query from the query text.
   * <p>
   * This is equivalent to {@code createBooleanQuery(field, queryText,
   * Occur.SHOULD)}
   * @param field field name
   * @param queryText text to be passed to the analyzer
   * @return {@code TermQuery} or {@code BooleanQuery}, based on the analysis
   *         of {@code queryText}
   */
  virtual std::shared_ptr<Query>
  createBooleanQuery(const std::wstring &field, const std::wstring &queryText);

  /**
   * Creates a bool query from the query text.
   * <p>
   * @param field field name
   * @param queryText text to be passed to the analyzer
   * @param operator operator used for clauses between analyzer tokens.
   * @return {@code TermQuery} or {@code BooleanQuery}, based on the analysis
   *         of {@code queryText}
   */
  virtual std::shared_ptr<Query>
  createBooleanQuery(const std::wstring &field, const std::wstring &queryText,
                     BooleanClause::Occur operator_);

  /**
   * Creates a phrase query from the query text.
   * <p>
   * This is equivalent to {@code createPhraseQuery(field, queryText, 0)}
   * @param field field name
   * @param queryText text to be passed to the analyzer
   * @return {@code TermQuery}, {@code BooleanQuery}, {@code PhraseQuery}, or
   *         {@code MultiPhraseQuery}, based on the analysis of {@code
   * queryText}
   */
  virtual std::shared_ptr<Query>
  createPhraseQuery(const std::wstring &field, const std::wstring &queryText);

  /**
   * Creates a phrase query from the query text.
   * <p>
   * @param field field name
   * @param queryText text to be passed to the analyzer
   * @param phraseSlop number of other words permitted between words in query
   * phrase
   * @return {@code TermQuery}, {@code BooleanQuery}, {@code PhraseQuery}, or
   *         {@code MultiPhraseQuery}, based on the analysis of {@code
   * queryText}
   */
  virtual std::shared_ptr<Query>
  createPhraseQuery(const std::wstring &field, const std::wstring &queryText,
                    int phraseSlop);

  /**
   * Creates a minimum-should-match query from the query text.
   * <p>
   * @param field field name
   * @param queryText text to be passed to the analyzer
   * @param fraction of query terms {@code [0..1]} that should match
   * @return {@code TermQuery} or {@code BooleanQuery}, based on the analysis
   *         of {@code queryText}
   */
  virtual std::shared_ptr<Query>
  createMinShouldMatchQuery(const std::wstring &field,
                            const std::wstring &queryText, float fraction);

  /**
   * Rebuilds a bool query and sets a new minimum number should match value.
   */
private:
  std::shared_ptr<BooleanQuery>
  addMinShouldMatchToBoolean(std::shared_ptr<BooleanQuery> query,
                             float fraction);

  /**
   * Returns the analyzer.
   * @see #setAnalyzer(Analyzer)
   */
public:
  virtual std::shared_ptr<Analyzer> getAnalyzer();

  /**
   * Sets the analyzer used to tokenize text.
   */
  virtual void setAnalyzer(std::shared_ptr<Analyzer> analyzer);

  /**
   * Returns true if position increments are enabled.
   * @see #setEnablePositionIncrements(bool)
   */
  virtual bool getEnablePositionIncrements();

  /**
   * Set to <code>true</code> to enable position increments in result query.
   * <p>
   * When set, result phrase and multi-phrase queries will
   * be aware of position increments.
   * Useful when e.g. a StopFilter increases the position increment of
   * the token that follows an omitted token.
   * <p>
   * Default: true.
   */
  virtual void setEnablePositionIncrements(bool enable);

  /**
   * Returns true if phrase query should be automatically generated for multi
   * terms synonyms.
   * @see #setAutoGenerateMultiTermSynonymsPhraseQuery(bool)
   */
  virtual bool getAutoGenerateMultiTermSynonymsPhraseQuery();

  /**
   * Set to <code>true</code> if phrase queries should be automatically
   * generated for multi terms synonyms. Default: false.
   */
  virtual void setAutoGenerateMultiTermSynonymsPhraseQuery(bool enable);

  /**
   * Creates a query from the analysis chain.
   * <p>
   * Expert: this is more useful for subclasses such as queryparsers.
   * If using this class directly, just use {@link #createBooleanQuery(std::wstring,
std::wstring)} GET_CLASS_NAME(es)
   * and {@link #createPhraseQuery(std::wstring, std::wstring)}.  This is a complex method
and
   * it is usually not necessary to override it in a subclass; instead, override
   * methods like {@link #newBooleanQuery}, etc., if possible.
GET_CLASS_NAME(;)
   *
   * @param analyzer   analyzer used for this query
   * @param operator   default bool operator used for this query
   * @param field      field to create queries against
   * @param queryText  text to be passed to the analysis chain
   * @param quoted     true if phrases should be generated when terms occur at
more than one position
   * @param phraseSlop slop factor for phrase/multiphrase queries
   */
protected:
  virtual std::shared_ptr<Query>
  createFieldQuery(std::shared_ptr<Analyzer> analyzer,
                   BooleanClause::Occur operator_, const std::wstring &field,
                   const std::wstring &queryText, bool quoted, int phraseSlop);

  /** Enable or disable graph TokenStream processing (enabled by default).
   *
   * @lucene.experimental */
public:
  virtual void setEnableGraphQueries(bool v);

  /** Returns true if graph TokenStream processing is enabled (default).
   *
   * @lucene.experimental */
  virtual bool getEnableGraphQueries();

  /**
   * Creates a query from a token stream.
   *
   * @param source     the token stream to create the query from
   * @param operator   default bool operator used for this query
   * @param field      field to create queries against
   * @param quoted     true if phrases should be generated when terms occur at
   * more than one position
   * @param phraseSlop slop factor for phrase/multiphrase queries
   */
protected:
  virtual std::shared_ptr<Query>
  createFieldQuery(std::shared_ptr<TokenStream> source,
                   BooleanClause::Occur operator_, const std::wstring &field,
                   bool quoted, int phraseSlop);

  /**
   * Creates a span query from the tokenstream.  In the case of a single token,
   * a simple <code>SpanTermQuery</code> is returned.  When multiple tokens, an
   * ordered <code>SpanNearQuery</code> with slop 0 is returned.
   */
  virtual std::shared_ptr<SpanQuery>
  createSpanQuery(std::shared_ptr<TokenStream> in_,
                  const std::wstring &field) ;

  /**
   * Creates simple term query from the cached tokenstream contents
   */
  virtual std::shared_ptr<Query>
  analyzeTerm(const std::wstring &field,
              std::shared_ptr<TokenStream> stream) ;

  /**
   * Creates simple bool query from the cached tokenstream contents
   */
  virtual std::shared_ptr<Query>
  analyzeBoolean(const std::wstring &field,
                 std::shared_ptr<TokenStream> stream) ;

  virtual void add(std::shared_ptr<BooleanQuery::Builder> q,
                   std::deque<std::shared_ptr<Term>> &current,
                   BooleanClause::Occur operator_);

  /**
   * Creates complex bool query from the cached tokenstream contents
   */
  virtual std::shared_ptr<Query>
  analyzeMultiBoolean(const std::wstring &field,
                      std::shared_ptr<TokenStream> stream,
                      BooleanClause::Occur operator_) ;

  /**
   * Creates simple phrase query from the cached tokenstream contents
   */
  virtual std::shared_ptr<Query>
  analyzePhrase(const std::wstring &field, std::shared_ptr<TokenStream> stream,
                int slop) ;

  /**
   * Creates complex phrase query from the cached tokenstream contents
   */
  virtual std::shared_ptr<Query>
  analyzeMultiPhrase(const std::wstring &field,
                     std::shared_ptr<TokenStream> stream,
                     int slop) ;

  /**
   * Creates a bool query from a graph token stream. The articulation points
   * of the graph are visited in order and the queries created at each point are
   * merged in the returned bool query.
   */
  virtual std::shared_ptr<Query>
  analyzeGraphBoolean(const std::wstring &field,
                      std::shared_ptr<TokenStream> source,
                      BooleanClause::Occur operator_) ;

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<std::shared_ptr<Query>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<QueryBuilder> outerInstance;

    std::wstring field;
    std::shared_ptr<Iterator<std::shared_ptr<TokenStream>>> it;

  public:
    IteratorAnonymousInnerClass(
        std::shared_ptr<QueryBuilder> outerInstance, const std::wstring &field,
        std::shared_ptr<Iterator<std::shared_ptr<TokenStream>>> it);

    bool hasNext();

    std::shared_ptr<Query> next();
  };

  /**
   * Creates a span near (phrase) query from a graph token stream. The
   * articulation points of the graph are visited in order and the queries
   * created at each point are merged in the returned near query.
   */
protected:
  virtual std::shared_ptr<SpanQuery>
  analyzeGraphPhrase(std::shared_ptr<TokenStream> source,
                     const std::wstring &field,
                     int phraseSlop) ;

  /**
   * Builds a new BooleanQuery instance.
   * <p>
   * This is intended for subclasses that wish to customize the generated
   * queries.
   * @return new BooleanQuery instance
   */
  virtual std::shared_ptr<BooleanQuery::Builder> newBooleanQuery();

  /**
   * Builds a new SynonymQuery instance.
   * <p>
   * This is intended for subclasses that wish to customize the generated
   * queries.
   * @return new Query instance
   */
  virtual std::shared_ptr<Query>
  newSynonymQuery(std::deque<std::shared_ptr<Term>> &terms);

  /**
   * Builds a new GraphQuery for multi-terms synonyms.
   * <p>
   * This is intended for subclasses that wish to customize the generated
   * queries.
   * @return new Query instance
   */
  virtual std::shared_ptr<Query> newGraphSynonymQuery(
      std::shared_ptr<Iterator<std::shared_ptr<Query>>> queries);

  /**
   * Builds a new TermQuery instance.
   * <p>
   * This is intended for subclasses that wish to customize the generated
   * queries.
   * @param term term
   * @return new TermQuery instance
   */
  virtual std::shared_ptr<Query> newTermQuery(std::shared_ptr<Term> term);

  /**
   * Builds a new MultiPhraseQuery instance.
   * <p>
   * This is intended for subclasses that wish to customize the generated
   * queries.
   * @return new MultiPhraseQuery instance
   */
  virtual std::shared_ptr<MultiPhraseQuery::Builder>
  newMultiPhraseQueryBuilder();
};

} // #include  "core/src/java/org/apache/lucene/util/
