#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/queryparser/simple/State.h"
#include  "core/src/java/org/apache/lucene/search/BooleanClause.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"

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
namespace org::apache::lucene::queryparser::simple
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using QueryBuilder = org::apache::lucene::util::QueryBuilder;

/**
 * SimpleQueryParser is used to parse human readable query syntax.
 * <p>
 * The main idea behind this parser is that a person should be able to type
 * whatever they want to represent a query, and this parser will do its best
 * to interpret what to search for no matter how poorly composed the request
 * may be. Tokens are considered to be any of a term, phrase, or subquery for
 * the operations described below.  Whitespace including ' ' '\n' '\r' and '\t'
 * and certain operators may be used to delimit tokens ( ) + | " .
 * <p>
 * Any errors in query syntax will be ignored and the parser will attempt
 * to decipher what it can; however, this may mean odd or unexpected results.
 * <p>
 * <b>Query Operators</b>
 * <ul>
 *  <li>'{@code +}' specifies {@code AND} operation: <tt>token1+token2</tt>
 *  <li>'{@code |}' specifies {@code OR} operation: <tt>token1|token2</tt>
 *  <li>'{@code -}' negates a single token: <tt>-token0</tt>
 *  <li>'{@code "}' creates phrases of terms: <tt>"term1 term2 ..."</tt>
 *  <li>'{@code *}' at the end of terms specifies prefix query: <tt>term*</tt>
 *  <li>'{@code ~}N' at the end of terms specifies fuzzy query: <tt>term~1</tt>
 *  <li>'{@code ~}N' at the end of phrases specifies near query: <tt>"term1
 * term2"~5</tt> <li>'{@code (}' and '{@code )}' specifies precedence:
 * <tt>token1 + (token2 | token3)</tt>
 * </ul>
 * <p>
 * The {@link #setDefaultOperator default operator} is {@code OR} if no other
 * operator is specified. For example, the following will {@code OR} {@code
 * token1} and {@code token2} together: <tt>token1 token2</tt> <p> Normal
 * operator precedence will be simple order from right to left. For example, the
 * following will evaluate {@code token1 OR token2} first, then {@code AND} with
 * {@code token3}: <blockquote>token1 | token2 + token3</blockquote>
 * <b>Escaping</b>
 * <p>
 * An individual term may contain any possible character with certain characters
 * requiring escaping using a '{@code \}'.  The following characters will need
 * to be escaped in terms and phrases:
 * {@code + | " ( ) ' \}
 * <p>
 * The '{@code -}' operator is a special case.  On individual terms (not
 * phrases) the first character of a term that is {@code -} must be escaped;
 * however, any '{@code -}' characters beyond the first character do not need to
 * be escaped. For example: <ul> <li>{@code -term1}   -- Specifies {@code NOT}
 * operation against {@code term1} <li>{@code \-term1}  -- Searches for the term
 * {@code -term1}. <li>{@code term-1}   -- Searches for the term {@code term-1}.
 *   <li>{@code term\-1}  -- Searches for the term {@code term-1}.
 * </ul>
 * <p>
 * The '{@code *}' operator is a special case. On individual terms (not phrases)
 * the last character of a term that is '{@code *}' must be escaped; however,
 * any '{@code *}' characters before the last character do not need to be
 * escaped: <ul> <li>{@code term1*}  --  Searches for the prefix {@code term1}
 *   <li>{@code term1\*} --  Searches for the term {@code term1*}
 *   <li>{@code term*1}  --  Searches for the term {@code term*1}
 *   <li>{@code term\*1} --  Searches for the term {@code term*1}
 * </ul>
 * <p>
 * Note that above examples consider the terms before text processing.
 */
class SimpleQueryParser : public QueryBuilder
{
  GET_CLASS_NAME(SimpleQueryParser)
  /** Map of fields to query against with their weights */
protected:
  const std::unordered_map<std::wstring, float> weights;
  /** flags to the parser (to turn features on/off) */
  const int flags;

  /** Enables {@code AND} operator (+) */
public:
  static const int AND_OPERATOR = 1 << 0;
  /** Enables {@code NOT} operator (-) */
  static const int NOT_OPERATOR = 1 << 1;
  /** Enables {@code OR} operator (|) */
  static const int OR_OPERATOR = 1 << 2;
  /** Enables {@code PREFIX} operator (*) */
  static const int PREFIX_OPERATOR = 1 << 3;
  /** Enables {@code PHRASE} operator (") */
  static const int PHRASE_OPERATOR = 1 << 4;
  /** Enables {@code PRECEDENCE} operators: {@code (} and {@code )} */
  static const int PRECEDENCE_OPERATORS = 1 << 5;
  /** Enables {@code ESCAPE} operator (\) */
  static const int ESCAPE_OPERATOR = 1 << 6;
  /** Enables {@code WHITESPACE} operators: ' ' '\n' '\r' '\t' */
  static const int WHITESPACE_OPERATOR = 1 << 7;
  /** Enables {@code FUZZY} operators: (~) on single terms */
  static const int FUZZY_OPERATOR = 1 << 8;
  /** Enables {@code NEAR} operators: (~) on phrases */
  static const int NEAR_OPERATOR = 1 << 9;

private:
  BooleanClause::Occur defaultOperator = BooleanClause::Occur::SHOULD;

  /** Creates a new parser searching over a single field. */
public:
  SimpleQueryParser(std::shared_ptr<Analyzer> analyzer,
                    const std::wstring &field);

  /** Creates a new parser searching over multiple fields with different
   * weights. */
  SimpleQueryParser(std::shared_ptr<Analyzer> analyzer,
                    std::unordered_map<std::wstring, float> &weights);

  /** Creates a new parser with custom flags used to enable/disable certain
   * features. */
  SimpleQueryParser(std::shared_ptr<Analyzer> analyzer,
                    std::unordered_map<std::wstring, float> &weights,
                    int flags);

  /** Parses the query text and returns parsed query */
  virtual std::shared_ptr<Query> parse(const std::wstring &queryText);

private:
  void parseSubQuery(std::shared_ptr<State> state);

  void consumeSubQuery(std::shared_ptr<State> state);

  void consumePhrase(std::shared_ptr<State> state);

  void consumeToken(std::shared_ptr<State> state);

  static std::shared_ptr<BooleanQuery>
  addClause(std::shared_ptr<BooleanQuery> bq, std::shared_ptr<Query> query,
            BooleanClause::Occur occur);

  // buildQueryTree should be called after a term, phrase, or subquery
  // is consumed to be added to our existing query tree
  // this method will only add to the existing tree if the branch contained in
  // state is not null
  void buildQueryTree(std::shared_ptr<State> state,
                      std::shared_ptr<Query> branch);

  /**
   * Helper parsing fuzziness from parsing state
   * @return slop/edit distance, 0 in the case of non-parsing slop/edit string
   */
  int parseFuzziness(std::shared_ptr<State> state);

  /**
   * Helper returning true if the state has reached the end of token.
   */
  bool tokenFinished(std::shared_ptr<State> state);

  /**
   * Factory method to generate a standard query (no phrase or prefix
   * operators).
   */
protected:
  virtual std::shared_ptr<Query> newDefaultQuery(const std::wstring &text);

  /**
   * Factory method to generate a fuzzy query.
   */
  virtual std::shared_ptr<Query> newFuzzyQuery(const std::wstring &text,
                                               int fuzziness);

  /**
   * Factory method to generate a phrase query with slop.
   */
  virtual std::shared_ptr<Query> newPhraseQuery(const std::wstring &text,
                                                int slop);

  /**
   * Factory method to generate a prefix query.
   */
  virtual std::shared_ptr<Query> newPrefixQuery(const std::wstring &text);

  /**
   * Helper to simplify bool queries with 0 or 1 clause
   */
  virtual std::shared_ptr<Query> simplify(std::shared_ptr<BooleanQuery> bq);

  /**
   * Returns the implicit operator setting, which will be
   * either {@code SHOULD} or {@code MUST}.
   */
public:
  virtual BooleanClause::Occur getDefaultOperator();

  /**
   * Sets the implicit operator setting, which must be
   * either {@code SHOULD} or {@code MUST}.
   */
  virtual void setDefaultOperator(BooleanClause::Occur operator_);

public:
  class State : public std::enable_shared_from_this<State>
  {
    GET_CLASS_NAME(State)
  public:
    std::deque<wchar_t> const data; // the characters in the query string
    std::deque<wchar_t> const
        buffer; // a temporary buffer used to reduce necessary allocations
    int index = 0;
    int length = 0;

    BooleanClause::Occur currentOperation =
        static_cast<BooleanClause::Occur>(0);
    BooleanClause::Occur previousOperation =
        static_cast<BooleanClause::Occur>(0);
    int not = 0;

    std::shared_ptr<Query> top;

    State(std::deque<wchar_t> &data, std::deque<wchar_t> &buffer, int index,
          int length);
  };

protected:
  std::shared_ptr<SimpleQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<SimpleQueryParser>(
        org.apache.lucene.util.QueryBuilder::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/simple/
