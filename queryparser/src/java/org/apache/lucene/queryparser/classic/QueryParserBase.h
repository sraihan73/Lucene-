#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/RewriteMethod.h"

#include  "core/src/java/org/apache/lucene/document/DateTools.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/queryparser/classic/CharStream.h"
#include  "core/src/java/org/apache/lucene/queryparser/classic/ParseException.h"
#include  "core/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/queryparser/classic/Token.h"

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
using DateTools = org::apache::lucene::document::DateTools;
using Term = org::apache::lucene::index::Term;
using Operator =
    org::apache::lucene::queryparser::classic::QueryParser::Operator;
using CommonQueryParserConfiguration = org::apache::lucene::queryparser::
    flexible::standard::CommonQueryParserConfiguration;
using namespace org::apache::lucene::search;
using BytesRef = org::apache::lucene::util::BytesRef;
using QueryBuilder = org::apache::lucene::util::QueryBuilder;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

/** This class is overridden by QueryParser in QueryParser.jj
 * and acts to separate the majority of the Java code from the .jj grammar file.
 */
class QueryParserBase : public QueryBuilder,
                        public CommonQueryParserConfiguration
{
  GET_CLASS_NAME(QueryParserBase)

public:
  static constexpr int CONJ_NONE = 0;
  static constexpr int CONJ_AND = 1;
  static constexpr int CONJ_OR = 2;

  static constexpr int MOD_NONE = 0;
  static constexpr int MOD_NOT = 10;
  static constexpr int MOD_REQ = 11;

  // make it possible to call setDefaultOperator() without accessing
  // the nested class:
  /** Alternative form of QueryParser.Operator.AND */
  static constexpr Operator AND_OPERATOR = Operator::AND;
  /** Alternative form of QueryParser.Operator.OR */
  static constexpr Operator OR_OPERATOR = Operator::OR;

  /** The actual operator that parser uses to combine query terms */
  Operator operator_ = OR_OPERATOR;

  std::shared_ptr<MultiTermQuery::RewriteMethod> multiTermRewriteMethod =
      MultiTermQuery::CONSTANT_SCORE_REWRITE;
  bool allowLeadingWildcard = false;

protected:
  std::wstring field;

public:
  int phraseSlop = 0;
  float fuzzyMinSim = FuzzyQuery::defaultMinSimilarity;
  int fuzzyPrefixLength = FuzzyQuery::defaultPrefixLength;
  std::shared_ptr<Locale> locale = Locale::getDefault();
  std::shared_ptr<TimeZone> timeZone = TimeZone::getDefault();

  // the default date resolution
  DateTools::Resolution dateResolution = nullptr;
  // maps field names to date resolutions
  std::unordered_map<std::wstring, DateTools::Resolution> fieldToDateResolution;

  bool autoGeneratePhraseQueries = false;
  int maxDeterminizedStates = DEFAULT_MAX_DETERMINIZED_STATES;

  // So the generated QueryParser(CharStream) won't error out
protected:
  QueryParserBase();

  /** Initializes a query parser.  Called by the QueryParser constructor
   *  @param f  the default field for query terms.
   *  @param a   used to find terms in the query text.
   */
public:
  virtual void init(const std::wstring &f, std::shared_ptr<Analyzer> a);

  // the generated parser will create these in QueryParser
  virtual void ReInit(std::shared_ptr<CharStream> stream) = 0;
  virtual std::shared_ptr<Query> TopLevelQuery(const std::wstring &field) = 0;

  /** Parses a query string, returning a {@link org.apache.lucene.search.Query}.
   *  @param query  the query string to be parsed.
   *  @throws ParseException if the parsing fails
   */
  virtual std::shared_ptr<Query>
  parse(const std::wstring &query) ;

  /**
   * @return Returns the default field.
   */
  virtual std::wstring getField();

  /**
   * @see #setAutoGeneratePhraseQueries(bool)
   */
  bool getAutoGeneratePhraseQueries();

  /**
   * Set to true if phrase queries will be automatically generated
   * when the analyzer returns more than one term from whitespace
   * delimited text.
   * NOTE: this behavior may not be suitable for all languages.
   * <p>
   * Set to false if phrase queries should only be generated when
   * surrounded by double quotes.
   */
  virtual void setAutoGeneratePhraseQueries(bool value);

  /**
   * Get the minimal similarity for fuzzy queries.
   */
  float getFuzzyMinSim() override;

  /**
   * Set the minimum similarity for fuzzy queries.
   * Default is 2f.
   */
  void setFuzzyMinSim(float fuzzyMinSim) override;

  /**
   * Get the prefix length for fuzzy queries.
   * @return Returns the fuzzyPrefixLength.
   */
  int getFuzzyPrefixLength() override;

  /**
   * Set the prefix length for fuzzy queries. Default is 0.
   * @param fuzzyPrefixLength The fuzzyPrefixLength to set.
   */
  void setFuzzyPrefixLength(int fuzzyPrefixLength) override;

  /**
   * Sets the default slop for phrases.  If zero, then exact phrase matches
   * are required.  Default value is zero.
   */
  void setPhraseSlop(int phraseSlop) override;

  /**
   * Gets the default slop for phrases.
   */
  int getPhraseSlop() override;

  /**
   * Set to <code>true</code> to allow leading wildcard characters.
   * <p>
   * When set, <code>*</code> or <code>?</code> are allowed as
   * the first character of a PrefixQuery and WildcardQuery.
   * Note that this can produce very slow
   * queries on big indexes.
   * <p>
   * Default: false.
   */
  void setAllowLeadingWildcard(bool allowLeadingWildcard) override;

  /**
   * @see #setAllowLeadingWildcard(bool)
   */
  bool getAllowLeadingWildcard() override;

  /**
   * Sets the bool operator of the QueryParser.
   * In default mode (<code>OR_OPERATOR</code>) terms without any modifiers
   * are considered optional: for example <code>capital of Hungary</code> is
   * equal to <code>capital OR of OR Hungary</code>.<br> In
   * <code>AND_OPERATOR</code> mode terms are considered to be in conjunction:
   * the above mentioned query is parsed as <code>capital AND of AND
   * Hungary</code>
   */
  virtual void setDefaultOperator(Operator op);

  /**
   * Gets implicit operator setting, which will be either AND_OPERATOR
   * or OR_OPERATOR.
   */
  virtual Operator getDefaultOperator();

  /**
   * By default QueryParser uses {@link
   * org.apache.lucene.search.MultiTermQuery#CONSTANT_SCORE_REWRITE} when
   * creating a {@link PrefixQuery}, {@link WildcardQuery} or {@link
   * TermRangeQuery}. This implementation is generally preferable because it a)
   * Runs faster b) Does not have the scarcity of terms unduly influence score
   * c) avoids any {@link TooManyClauses} exception.
   * However, if your application really needs to use the
   * old-fashioned {@link BooleanQuery} expansion rewriting and the above
   * points are not relevant then use this to change
   * the rewrite method.
   */
  void setMultiTermRewriteMethod(
      std::shared_ptr<MultiTermQuery::RewriteMethod> method) override;

  /**
   * @see #setMultiTermRewriteMethod
   */
  std::shared_ptr<MultiTermQuery::RewriteMethod>
  getMultiTermRewriteMethod() override;

  /**
   * Set locale used by date range parsing, lowercasing, and other
   * locale-sensitive operations.
   */
  void setLocale(std::shared_ptr<Locale> locale) override;

  /**
   * Returns current locale, allowing access by subclasses.
   */
  std::shared_ptr<Locale> getLocale() override;

  void setTimeZone(std::shared_ptr<TimeZone> timeZone) override;

  std::shared_ptr<TimeZone> getTimeZone() override;

  /**
   * Sets the default date resolution used by RangeQueries for fields for which
   * no specific date resolutions has been set. Field specific resolutions can
   * be set with {@link #setDateResolution(std::wstring,
   * org.apache.lucene.document.DateTools.Resolution)}.
   *
   * @param dateResolution the default date resolution to set
   */
  void setDateResolution(DateTools::Resolution dateResolution) override;

  /**
   * Sets the date resolution used by RangeQueries for a specific field.
   *
   * @param fieldName field for which the date resolution is to be set
   * @param dateResolution date resolution to set
   */
  virtual void setDateResolution(const std::wstring &fieldName,
                                 DateTools::Resolution dateResolution);

  /**
   * Returns the date resolution that is used by RangeQueries for the given
   * field. Returns null, if no default or field specific date resolution has
   * been set for the given field.
   *
   */
  virtual DateTools::Resolution
  getDateResolution(const std::wstring &fieldName);

  /**
   * @param maxDeterminizedStates the maximum number of states that
   *   determinizing a regexp query can result in.  If the query results in any
   *   more states a TooComplexToDeterminizeException is thrown.
   */
  virtual void setMaxDeterminizedStates(int maxDeterminizedStates);

  /**
   * @return the maximum number of states that determinizing a regexp query
   *   can result in.  If the query results in any more states a
   *   TooComplexToDeterminizeException is thrown.
   */
  virtual int getMaxDeterminizedStates();

protected:
  virtual void addClause(std::deque<std::shared_ptr<BooleanClause>> &clauses,
                         int conj, int mods, std::shared_ptr<Query> q);

  /**
   * Adds clauses generated from analysis over text containing whitespace.
   * There are no operators, so the query's clauses can either be MUST (if the
   * default operator is AND) or SHOULD (default OR).
   *
   * If all of the clauses in the given Query are TermQuery-s, this method
   * flattens the result by adding the TermQuery-s individually to the output
   * clause deque; otherwise, the given Query is added as a single clause
   * including its nested clauses.
   */
  virtual void
  addMultiTermClauses(std::deque<std::shared_ptr<BooleanClause>> &clauses,
                      std::shared_ptr<Query> q);

  /**
   * @exception org.apache.lucene.queryparser.classic.ParseException throw in
   * overridden method to disallow
   */
  virtual std::shared_ptr<Query>
  getFieldQuery(const std::wstring &field, const std::wstring &queryText,
                bool quoted) ;

  /**
   * @exception org.apache.lucene.queryparser.classic.ParseException throw in
   * overridden method to disallow
   */
  virtual std::shared_ptr<Query>
  newFieldQuery(std::shared_ptr<Analyzer> analyzer, const std::wstring &field,
                const std::wstring &queryText,
                bool quoted) ;

  /**
   * Base implementation delegates to {@link
   * #getFieldQuery(std::wstring,std::wstring,bool)}. This method may be overridden, for
   * example, to return a SpanNearQuery instead of a PhraseQuery.
   *
   * @exception org.apache.lucene.queryparser.classic.ParseException throw in
   * overridden method to disallow
   */
  virtual std::shared_ptr<Query> getFieldQuery(const std::wstring &field,
                                               const std::wstring &queryText,
                                               int slop) ;

  /**
   * Rebuild a phrase query with a slop value
   */
private:
  std::shared_ptr<PhraseQuery>
  addSlopToPhrase(std::shared_ptr<PhraseQuery> query, int slop);

protected:
  virtual std::shared_ptr<Query>
  getRangeQuery(const std::wstring &field, const std::wstring &part1,
                const std::wstring &part2, bool startInclusive,
                bool endInclusive) ;

  /**
   * Builds a new BooleanClause instance
   * @param q sub query
   * @param occur how this clause should occur when matching documents
   * @return new BooleanClause instance
   */
  virtual std::shared_ptr<BooleanClause>
  newBooleanClause(std::shared_ptr<Query> q, BooleanClause::Occur occur);

  /**
   * Builds a new PrefixQuery instance
   * @param prefix Prefix term
   * @return new PrefixQuery instance
   */
  virtual std::shared_ptr<Query> newPrefixQuery(std::shared_ptr<Term> prefix);

  /**
   * Builds a new RegexpQuery instance
   * @param regexp Regexp term
   * @return new RegexpQuery instance
   */
  virtual std::shared_ptr<Query> newRegexpQuery(std::shared_ptr<Term> regexp);

  /**
   * Builds a new FuzzyQuery instance
   * @param term Term
   * @param minimumSimilarity minimum similarity
   * @param prefixLength prefix length
   * @return new FuzzyQuery Instance
   */
  virtual std::shared_ptr<Query> newFuzzyQuery(std::shared_ptr<Term> term,
                                               float minimumSimilarity,
                                               int prefixLength);

  /**
   * Builds a new {@link TermRangeQuery} instance
   * @param field Field
   * @param part1 min
   * @param part2 max
   * @param startInclusive true if the start of the range is inclusive
   * @param endInclusive true if the end of the range is inclusive
   * @return new {@link TermRangeQuery} instance
   */
  virtual std::shared_ptr<Query> newRangeQuery(const std::wstring &field,
                                               const std::wstring &part1,
                                               const std::wstring &part2,
                                               bool startInclusive,
                                               bool endInclusive);

  /**
   * Builds a new MatchAllDocsQuery instance
   * @return new MatchAllDocsQuery instance
   */
  virtual std::shared_ptr<Query> newMatchAllDocsQuery();

  /**
   * Builds a new WildcardQuery instance
   * @param t wildcard term
   * @return new WildcardQuery instance
   */
  virtual std::shared_ptr<Query> newWildcardQuery(std::shared_ptr<Term> t);

  /**
   * Factory method for generating query, given a set of clauses.
   * By default creates a bool query composed of clauses passed in.
   *
   * Can be overridden by extending classes, to modify query being
   * returned.
   *
   * @param clauses List that contains {@link
   * org.apache.lucene.search.BooleanClause} instances to join.
   *
   * @return Resulting {@link org.apache.lucene.search.Query} object.
   * @exception org.apache.lucene.queryparser.classic.ParseException throw in
   * overridden method to disallow
   */
  virtual std::shared_ptr<Query>
  getBooleanQuery(std::deque<std::shared_ptr<BooleanClause>> &clauses) throw(
      ParseException);

  /**
   * Factory method for generating a query. Called when parser
   * parses an input term token that contains one or more wildcard
   * characters (? and *), but is not a prefix term token (one
   * that has just a single * character at the end)
   *<p>
   * Depending on settings, prefix term may be lower-cased
   * automatically. It will not go through the default Analyzer,
   * however, since normal Analyzers are unlikely to work properly
   * with wildcard templates.
   *<p>
   * Can be overridden by extending classes, to provide custom handling for
   * wildcard queries, which may be necessary due to missing analyzer calls.
   *
   * @param field Name of the field query will use.
   * @param termStr Term token that contains one or more wild card
   *   characters (? or *), but is not simple prefix term
   *
   * @return Resulting {@link org.apache.lucene.search.Query} built for the term
   * @exception org.apache.lucene.queryparser.classic.ParseException throw in
   *overridden method to disallow
   */
  virtual std::shared_ptr<Query>
  getWildcardQuery(const std::wstring &field,
                   const std::wstring &termStr) ;

private:
  static const std::shared_ptr<Pattern> WILDCARD_PATTERN;

  std::shared_ptr<BytesRef> analyzeWildcard(const std::wstring &field,
                                            const std::wstring &termStr);

  /**
   * Factory method for generating a query. Called when parser
   * parses an input term token that contains a regular expression
   * query.
   *<p>
   * Depending on settings, pattern term may be lower-cased
   * automatically. It will not go through the default Analyzer,
   * however, since normal Analyzers are unlikely to work properly
   * with regular expression templates.
   *<p>
   * Can be overridden by extending classes, to provide custom handling for
   * regular expression queries, which may be necessary due to missing analyzer
   * calls.
   *
   * @param field Name of the field query will use.
   * @param termStr Term token that contains a regular expression
   *
   * @return Resulting {@link org.apache.lucene.search.Query} built for the term
   * @exception org.apache.lucene.queryparser.classic.ParseException throw in
   *overridden method to disallow
   */
protected:
  virtual std::shared_ptr<Query>
  getRegexpQuery(const std::wstring &field,
                 const std::wstring &termStr) ;

  /**
   * Factory method for generating a query (similar to
   * {@link #getWildcardQuery}). Called when parser parses an input term
   * token that uses prefix notation; that is, contains a single '*' wildcard
   * character as its last character. Since this is a special case
   * of generic wildcard term, and such a query can be optimized easily,
   * this usually results in a different query object.
   *<p>
   * Depending on settings, a prefix term may be lower-cased
   * automatically. It will not go through the default Analyzer,
   * however, since normal Analyzers are unlikely to work properly
   * with wildcard templates.
   *<p>
   * Can be overridden by extending classes, to provide custom handling for
   * wild card queries, which may be necessary due to missing analyzer calls.
   *
   * @param field Name of the field query will use.
   * @param termStr Term token to use for building term for the query
   *    (<b>without</b> trailing '*' character!)
   *
   * @return Resulting {@link org.apache.lucene.search.Query} built for the term
   * @exception org.apache.lucene.queryparser.classic.ParseException throw in
   *overridden method to disallow
   */
  virtual std::shared_ptr<Query>
  getPrefixQuery(const std::wstring &field,
                 const std::wstring &termStr) ;

  /**
   * Factory method for generating a query (similar to
   * {@link #getWildcardQuery}). Called when parser parses
   * an input term token that has the fuzzy suffix (~) appended.
   *
   * @param field Name of the field query will use.
   * @param termStr Term token to use for building term for the query
   *
   * @return Resulting {@link org.apache.lucene.search.Query} built for the term
   * @exception org.apache.lucene.queryparser.classic.ParseException throw in
   * overridden method to disallow
   */
  virtual std::shared_ptr<Query>
  getFuzzyQuery(const std::wstring &field, const std::wstring &termStr,
                float minSimilarity) ;

  // extracted from the .jj grammar
public:
  virtual std::shared_ptr<Query>
  handleBareTokenQuery(const std::wstring &qfield, std::shared_ptr<Token> term,
                       std::shared_ptr<Token> fuzzySlop, bool prefix,
                       bool wildcard, bool fuzzy,
                       bool regexp) ;

  virtual std::shared_ptr<Query>
  handleBareFuzzy(const std::wstring &qfield, std::shared_ptr<Token> fuzzySlop,
                  const std::wstring &termImage) ;

  // extracted from the .jj grammar
  virtual std::shared_ptr<Query>
  handleQuotedTerm(const std::wstring &qfield, std::shared_ptr<Token> term,
                   std::shared_ptr<Token> fuzzySlop) ;

  // extracted from the .jj grammar
  virtual std::shared_ptr<Query> handleBoost(std::shared_ptr<Query> q,
                                             std::shared_ptr<Token> boost);

  /**
   * Returns a std::wstring where the escape char has been
   * removed, or kept only once if there was a double escape.
   *
   * Supports escaped unicode characters, e. g. translates
   * <code>\\u0041</code> to <code>A</code>.
   *
   */
  virtual std::wstring
  discardEscapeChar(const std::wstring &input) ;

  /** Returns the numeric value of the hexadecimal character */
  static int hexToInt(wchar_t c) ;

  /**
   * Returns a std::wstring where those characters that QueryParser
   * expects to be escaped are escaped by a preceding <code>\</code>.
   */
  static std::wstring escape(const std::wstring &s);

protected:
  std::shared_ptr<QueryParserBase> shared_from_this()
  {
    return std::static_pointer_cast<QueryParserBase>(
        org.apache.lucene.util.QueryBuilder::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/classic/
