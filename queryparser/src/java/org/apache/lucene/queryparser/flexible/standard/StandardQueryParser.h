#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/config/StandardQueryConfigHandler.h"
#include  "core/src/java/org/apache/lucene/search/MultiTermQuery.h"
#include  "core/src/java/org/apache/lucene/search/RewriteMethod.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/config/PointsConfig.h"
#include  "core/src/java/org/apache/lucene/document/DateTools.h"

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
namespace org::apache::lucene::queryparser::flexible::standard
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using DateTools = org::apache::lucene::document::DateTools;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryParserHelper =
    org::apache::lucene::queryparser::flexible::core::QueryParserHelper;
using PointsConfig =
    org::apache::lucene::queryparser::flexible::standard::config::PointsConfig;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using Operator = org::apache::lucene::queryparser::flexible::standard::config::
    StandardQueryConfigHandler::Operator;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using Query = org::apache::lucene::search::Query;

/**
 * This class is a helper that enables users to easily use the Lucene query
 * parser.
 * <p>
 * To construct a Query object from a query string, use the
 * {@link #parse(std::wstring, std::wstring)} method:
 * <pre class="prettyprint">
 * StandardQueryParser queryParserHelper = new StandardQueryParser();
 * Query query = queryParserHelper.parse("a AND b", "defaultField");
 * </pre>
 * <p>
 * To change any configuration before parsing the query string do, for example:
 * <br>
 * <pre class="prettyprint">
 * // the query config handler returned by {@link StandardQueryParser} is a
{@link StandardQueryConfigHandler} GET_CLASS_NAME(="prettyprint">)
 * queryParserHelper.getQueryConfigHandler().setAnalyzer(new
WhitespaceAnalyzer());
 * </pre>
 * <p>
 * The syntax for query strings is as follows (copied from the old QueryParser
 * javadoc):
 * A Query is a series of clauses. A clause may be prefixed by:
 * <ul>
 * <li>a plus (<code>+</code>) or a minus (<code>-</code>) sign, indicating that
 * the clause is required or prohibited respectively; or
 * <li>a term followed by a colon, indicating the field to be searched. This
 * enables one to construct queries which search multiple fields.
 * </ul>
 *
 * A clause may be either:
 * <ul>
 * <li>a term, indicating all the documents that contain this term; or
 * <li>a nested query, enclosed in parentheses. Note that this may be used with
 * a <code>+</code>/<code>-</code> prefix to require any of a set of terms.
 * </ul>
 *
 * Thus, in BNF, the query grammar is:
 *
 * <pre>
 *   Query  ::= ( Clause )*
 *   Clause ::= [&quot;+&quot;, &quot;-&quot;] [&lt;TERM&gt; &quot;:&quot;] (
&lt;TERM&gt; | &quot;(&quot; Query &quot;)&quot; )
 * </pre>
 *
 * <p>
 * Examples of appropriately formatted queries can be found in the <a
 *
href="{@docRoot}/org/apache/lucene/queryparser/classic/package-summary.html#package.description">
 * query syntax documentation</a>.
 * </p>
 * <p>
 * The text parser used by this helper is a {@link StandardSyntaxParser}.
 * <p>
 * The query node processor used by this helper is a
 * {@link StandardQueryNodeProcessorPipeline}.
 * <p>
 * The builder used by this helper is a {@link StandardQueryTreeBuilder}.
 *
 * @see StandardQueryParser
 * @see StandardQueryConfigHandler
 * @see StandardSyntaxParser
 * @see StandardQueryNodeProcessorPipeline
 * @see StandardQueryTreeBuilder
 */
class StandardQueryParser : public QueryParserHelper,
                            public CommonQueryParserConfiguration
{
  GET_CLASS_NAME(StandardQueryParser)

  /**
   * Constructs a {@link StandardQueryParser} object.
   */
public:
  StandardQueryParser();

  /**
   * Constructs a {@link StandardQueryParser} object and sets an
   * {@link Analyzer} to it. The same as:
   *
   * <pre class="prettyprint">
   * StandardQueryParser qp = new StandardQueryParser();
   * qp.getQueryConfigHandler().setAnalyzer(analyzer);
   * </pre>
   *
   * @param analyzer
   *          the analyzer to be used by this query parser helper
   */
  StandardQueryParser(std::shared_ptr<Analyzer> analyzer);

  virtual std::wstring toString();

  /**
   * Overrides {@link QueryParserHelper#parse(std::wstring, std::wstring)} so it casts the
   * return object to {@link Query}. For more reference about this method, check
   * {@link QueryParserHelper#parse(std::wstring, std::wstring)}.
   *
   * @param query
   *          the query string
   * @param defaultField
   *          the default field used by the text parser
   *
   * @return the object built from the query
   *
   * @throws QueryNodeException
   *           if something wrong happens along the three phases
   */
  std::shared_ptr<Query>
  parse(const std::wstring &query,
        const std::wstring &defaultField)  override;

  /**
   * Gets implicit operator setting, which will be either {@link Operator#AND}
   * or {@link Operator#OR}.
   */
  virtual StandardQueryConfigHandler::Operator getDefaultOperator();

  /**
   * Sets the bool operator of the QueryParser. In default mode (
   * {@link Operator#OR}) terms without any modifiers are considered optional:
   * for example <code>capital of Hungary</code> is equal to
   * <code>capital OR of OR Hungary</code>.<br>
   * In {@link Operator#AND} mode terms are considered to be in conjunction: the
   * above mentioned query is parsed as <code>capital AND of AND Hungary</code>
   */
  virtual void
  setDefaultOperator(StandardQueryConfigHandler::Operator operator_);

  /**
   * Set to <code>true</code> to allow leading wildcard characters.
   * <p>
   * When set, <code>*</code> or <code>?</code> are allowed as the first
   * character of a PrefixQuery and WildcardQuery. Note that this can produce
   * very slow queries on big indexes.
   * <p>
   * Default: false.
   */
  void setAllowLeadingWildcard(bool allowLeadingWildcard) override;

  /**
   * Set to <code>true</code> to enable position increments in result query.
   * <p>
   * When set, result phrase and multi-phrase queries will be aware of position
   * increments. Useful when e.g. a StopFilter increases the position increment
   * of the token that follows an omitted token.
   * <p>
   * Default: false.
   */
  void setEnablePositionIncrements(bool enabled) override;

  /**
   * @see #setEnablePositionIncrements(bool)
   */
  bool getEnablePositionIncrements() override;

  /**
   * By default, it uses
   * {@link MultiTermQuery#CONSTANT_SCORE_REWRITE} when creating a
   * prefix, wildcard and range queries. This implementation is generally
   * preferable because it a) Runs faster b) Does not have the scarcity of terms
   * unduly influence score c) avoids any {@link TooManyListenersException}
   * exception. However, if your application really needs to use the
   * old-fashioned bool queries expansion rewriting and the above points are
   * not relevant then use this change the rewrite method.
   */
  void setMultiTermRewriteMethod(
      std::shared_ptr<MultiTermQuery::RewriteMethod> method) override;

  /**
   * @see
   * #setMultiTermRewriteMethod(org.apache.lucene.search.MultiTermQuery.RewriteMethod)
   */
  std::shared_ptr<MultiTermQuery::RewriteMethod>
  getMultiTermRewriteMethod() override;

  /**
   * Set the fields a query should be expanded to when the field is
   * <code>null</code>
   *
   * @param fields the fields used to expand the query
   */
  virtual void
  setMultiFields(std::deque<std::shared_ptr<std::wstring>> &fields);

  /**
   * Returns the fields used to expand the query when the field for a
   * certain query is <code>null</code>
   *
   * @return the fields used to expand the query
   */
  virtual std::deque<std::shared_ptr<std::wstring>> getMultiFields();

  /**
   * Set the prefix length for fuzzy queries. Default is 0.
   *
   * @param fuzzyPrefixLength
   *          The fuzzyPrefixLength to set.
   */
  void setFuzzyPrefixLength(int fuzzyPrefixLength) override;

  virtual void setPointsConfigMap(
      std::unordered_map<std::wstring, std::shared_ptr<PointsConfig>>
          &pointsConfigMap);

  virtual std::unordered_map<std::wstring, std::shared_ptr<PointsConfig>>
  getPointsConfigMap();

  /**
   * Set locale used by date range parsing.
   */
  void setLocale(std::shared_ptr<Locale> locale) override;

  /**
   * Returns current locale, allowing access by subclasses.
   */
  std::shared_ptr<Locale> getLocale() override;

  void setTimeZone(std::shared_ptr<TimeZone> timeZone) override;

  std::shared_ptr<TimeZone> getTimeZone() override;

  /**
   * Sets the default slop for phrases. If zero, then exact phrase matches are
   * required. Default value is zero.
   */
  void setPhraseSlop(int defaultPhraseSlop) override;

  virtual void setAnalyzer(std::shared_ptr<Analyzer> analyzer);

  std::shared_ptr<Analyzer> getAnalyzer() override;

  /**
   * @see #setAllowLeadingWildcard(bool)
   */
  bool getAllowLeadingWildcard() override;

  /**
   * Get the minimal similarity for fuzzy queries.
   */
  float getFuzzyMinSim() override;

  /**
   * Get the prefix length for fuzzy queries.
   *
   * @return Returns the fuzzyPrefixLength.
   */
  int getFuzzyPrefixLength() override;

  /**
   * Gets the default slop for phrases.
   */
  int getPhraseSlop() override;

  /**
   * Set the minimum similarity for fuzzy queries. Default is defined on
   * {@link FuzzyQuery#defaultMinSimilarity}.
   */
  void setFuzzyMinSim(float fuzzyMinSim) override;

  /**
   * Sets the boost used for each field.
   *
   * @param boosts a collection that maps a field to its boost
   */
  virtual void setFieldsBoost(std::unordered_map<std::wstring, float> &boosts);

  /**
   * Returns the field to boost map_obj used to set boost for each field.
   *
   * @return the field to boost map_obj
   */
  virtual std::unordered_map<std::wstring, float> getFieldsBoost();

  /**
   * Sets the default {@link Resolution} used for certain field when
   * no {@link Resolution} is defined for this field.
   *
   * @param dateResolution the default {@link Resolution}
   */
  void setDateResolution(DateTools::Resolution dateResolution) override;

  /**
   * Returns the default {@link Resolution} used for certain field when
   * no {@link Resolution} is defined for this field.
   *
   * @return the default {@link Resolution}
   */
  virtual DateTools::Resolution getDateResolution();

  /**
   * Returns the field to {@link Resolution} map_obj used to normalize each date
   * field.
   *
   * @return the field to {@link Resolution} map_obj
   */
  virtual std::unordered_map<std::shared_ptr<std::wstring>,
                             DateTools::Resolution>
  getDateResolutionMap();

  /**
   * Sets the {@link Resolution} used for each field
   *
   * @param dateRes a collection that maps a field to its {@link Resolution}
   */
  virtual void setDateResolutionMap(
      std::unordered_map<std::shared_ptr<std::wstring>, DateTools::Resolution>
          &dateRes);

protected:
  std::shared_ptr<StandardQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<StandardQueryParser>(
        org.apache.lucene.queryparser.flexible.core
            .QueryParserHelper::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/
