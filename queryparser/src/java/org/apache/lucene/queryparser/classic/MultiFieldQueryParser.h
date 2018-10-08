#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/queryparser/classic/ParseException.h"
#include  "core/src/java/org/apache/lucene/search/BooleanClause.h"

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
namespace org::apache::lucene::queryparser::classic
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using Query = org::apache::lucene::search::Query;

/**
 * A QueryParser which constructs queries to search multiple fields.
 *
 */
class MultiFieldQueryParser : public QueryParser
{
  GET_CLASS_NAME(MultiFieldQueryParser)
protected:
  std::deque<std::wstring> fields;
  std::unordered_map<std::wstring, float> boosts;

  /**
   * Creates a MultiFieldQueryParser.
   * Allows passing of a map_obj with term to Boost, and the boost to apply to each
   * term.
   *
   * <p>It will, when parse(std::wstring query)
   * is called, construct a query like this (assuming the query consists of
   * two terms and you specify the two fields <code>title</code> and
   * <code>body</code>):</p>
   *
   * <code>
   * (title:term1 body:term1) (title:term2 body:term2)
   * </code>
   *
   * <p>When setDefaultOperator(AND_OPERATOR) is set, the result will be:</p>
   *
   * <code>
   * +(title:term1 body:term1) +(title:term2 body:term2)
   * </code>
   *
   * <p>When you pass a boost (title=&gt;5 body=&gt;10) you can get </p>
   *
   * <code>
   * +(title:term1^5.0 body:term1^10.0) +(title:term2^5.0 body:term2^10.0)
   * </code>
   *
   * <p>In other words, all the query's terms must appear, but it doesn't matter
   * in what fields they appear.</p>
   */
public:
  MultiFieldQueryParser(std::deque<std::wstring> &fields,
                        std::shared_ptr<Analyzer> analyzer,
                        std::unordered_map<std::wstring, float> &boosts);

  /**
   * Creates a MultiFieldQueryParser.
   *
   * <p>It will, when parse(std::wstring query)
   * is called, construct a query like this (assuming the query consists of
   * two terms and you specify the two fields <code>title</code> and
   * <code>body</code>):</p>
   *
   * <code>
   * (title:term1 body:term1) (title:term2 body:term2)
   * </code>
   *
   * <p>When setDefaultOperator(AND_OPERATOR) is set, the result will be:</p>
   *
   * <code>
   * +(title:term1 body:term1) +(title:term2 body:term2)
   * </code>
   *
   * <p>In other words, all the query's terms must appear, but it doesn't matter
   * in what fields they appear.</p>
   */
  MultiFieldQueryParser(std::deque<std::wstring> &fields,
                        std::shared_ptr<Analyzer> analyzer);

protected:
  std::shared_ptr<Query> getFieldQuery(const std::wstring &field,
                                       const std::wstring &queryText,
                                       int slop)  override;

private:
  std::shared_ptr<Query> applySlop(std::shared_ptr<Query> q, int slop);

protected:
  std::shared_ptr<Query>
  getFieldQuery(const std::wstring &field, const std::wstring &queryText,
                bool quoted)  override;

  std::shared_ptr<Query>
  getFuzzyQuery(const std::wstring &field, const std::wstring &termStr,
                float minSimilarity)  override;

  std::shared_ptr<Query>
  getPrefixQuery(const std::wstring &field,
                 const std::wstring &termStr)  override;

  std::shared_ptr<Query>
  getWildcardQuery(const std::wstring &field,
                   const std::wstring &termStr)  override;

  std::shared_ptr<Query>
  getRangeQuery(const std::wstring &field, const std::wstring &part1,
                const std::wstring &part2, bool startInclusive,
                bool endInclusive)  override;

  std::shared_ptr<Query>
  getRegexpQuery(const std::wstring &field,
                 const std::wstring &termStr)  override;

  /** Creates a multifield query */
  // TODO: investigate more general approach by default, e.g.
  // DisjunctionMaxQuery?
  virtual std::shared_ptr<Query> getMultiFieldQuery(
      std::deque<std::shared_ptr<Query>> &queries) ;

  /**
   * Parses a query which searches on the fields specified.
   * <p>
   * If x fields are specified, this effectively constructs:
   * <pre>
   * <code>
   * (field1:query1) (field2:query2) (field3:query3)...(fieldx:queryx)
   * </code>
   * </pre>
   * @param queries Queries strings to parse
   * @param fields Fields to search on
   * @param analyzer Analyzer to use
   * @throws ParseException if query parsing fails
   * @throws IllegalArgumentException if the length of the queries array differs
   *  from the length of the fields array
   */
public:
  static std::shared_ptr<Query>
  parse(std::deque<std::wstring> &queries, std::deque<std::wstring> &fields,
        std::shared_ptr<Analyzer> analyzer) ;

  /**
   * Parses a query, searching on the fields specified.
   * Use this if you need to specify certain fields as required,
   * and others as prohibited.
   * <p>
   * Usage:
   * <pre class="prettyprint">
   * <code>
   * std::wstring[] fields = {"filename", "contents", "description"};
   * BooleanClause.Occur[] flags = {BooleanClause.Occur.SHOULD,
   *                BooleanClause.Occur.MUST,
   *                BooleanClause.Occur.MUST_NOT};
   * MultiFieldQueryParser.parse("query", fields, flags, analyzer);
   * </code>
   * </pre>
   *<p>
   * The code above would construct a query:
   * <pre>
   * <code>
   * (filename:query) +(contents:query) -(description:query)
   * </code>
   * </pre>
   *
   * @param query Query string to parse
   * @param fields Fields to search on
   * @param flags Flags describing the fields
   * @param analyzer Analyzer to use
   * @throws ParseException if query parsing fails
   * @throws IllegalArgumentException if the length of the fields array differs
   *  from the length of the flags array
   */
  static std::shared_ptr<Query>
  parse(const std::wstring &query, std::deque<std::wstring> &fields,
        std::deque<BooleanClause::Occur> &flags,
        std::shared_ptr<Analyzer> analyzer) ;

  /**
   * Parses a query, searching on the fields specified.
   * Use this if you need to specify certain fields as required,
   * and others as prohibited.
   * <p>
   * Usage:
   * <pre class="prettyprint">
   * <code>
   * std::wstring[] query = {"query1", "query2", "query3"};
   * std::wstring[] fields = {"filename", "contents", "description"};
   * BooleanClause.Occur[] flags = {BooleanClause.Occur.SHOULD,
   *                BooleanClause.Occur.MUST,
   *                BooleanClause.Occur.MUST_NOT};
   * MultiFieldQueryParser.parse(query, fields, flags, analyzer);
   * </code>
   * </pre>
   *<p>
   * The code above would construct a query:
   * <pre>
   * <code>
   * (filename:query1) +(contents:query2) -(description:query3)
   * </code>
   * </pre>
   *
   * @param queries Queries string to parse
   * @param fields Fields to search on
   * @param flags Flags describing the fields
   * @param analyzer Analyzer to use
   * @throws ParseException if query parsing fails
   * @throws IllegalArgumentException if the length of the queries, fields,
   *  and flags array differ
   */
  static std::shared_ptr<Query>
  parse(std::deque<std::wstring> &queries, std::deque<std::wstring> &fields,
        std::deque<BooleanClause::Occur> &flags,
        std::shared_ptr<Analyzer> analyzer) ;

protected:
  std::shared_ptr<MultiFieldQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<MultiFieldQueryParser>(
        QueryParser::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/classic/
