#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::queryparser::classic
{
class ParseException;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class BooleanClause;
}
namespace org::apache::lucene::queryparser::classic
{
class QueryParserTokenManager;
}
namespace org::apache::lucene::queryparser::classic
{
class Token;
}
namespace org::apache::lucene::queryparser::classic
{
class JJCalls;
}
namespace org::apache::lucene::queryparser::classic
{
class CharStream;
}

/* Generated By:JavaCC: Do not edit this line. QueryParser.java */
namespace org::apache::lucene::queryparser::classic
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using Query = org::apache::lucene::search::Query;

/**
 * This class is generated by JavaCC.  The most important method is
 * {@link #parse(std::wstring)}.
GET_CLASS_NAME(is)
 *
 * The syntax for query strings is as follows:
 * A Query is a series of clauses.
 * A clause may be prefixed by:
 * <ul>
 * <li> a plus (<code>+</code>) or a minus (<code>-</code>) sign, indicating
 * that the clause is required or prohibited respectively; or
 * <li> a term followed by a colon, indicating the field to be searched.
 * This enables one to construct queries which search multiple fields.
 * </ul>
 *
 * A clause may be either:
 * <ul>
 * <li> a term, indicating all the documents that contain this term; or
 * <li> a nested query, enclosed in parentheses.  Note that this may be used
 * with a <code>+</code>/<code>-</code> prefix to require any of a set of
 * terms.
 * </ul>
 *
 * Thus, in BNF, the query grammar is:
 * <pre>
 *   Query  ::= ( Clause )*
 *   Clause ::= ["+", "-"] [&lt;TERM&gt; ":"] ( &lt;TERM&gt; | "(" Query ")" )
 * </pre>
 *
 * <p>
 * Examples of appropriately formatted queries can be found in the <a
 *
href="{@docRoot}/org/apache/lucene/queryparser/classic/package-summary.html#package.description">query
syntax
 * documentation</a>.
 * </p>
 *
 * <p>
 * In {@link TermRangeQuery}s, QueryParser tries to detect date values, e.g.
 * <tt>date:[6/1/2005 TO 6/4/2005]</tt> produces a range query that searches
 * for "date" fields between 2005-06-01 and 2005-06-04. Note that the format
 * of the accepted input depends on {@link #setLocale(Locale) the locale}.
 * A {@link org.apache.lucene.document.DateTools.Resolution} has to be set,
 * if you want to use {@link DateTools} for date conversion.
 * </p>
 * <p>
 * The date resolution that shall be used for RangeQueries can be set
 * using {@link #setDateResolution(DateTools.Resolution)}
 * or {@link #setDateResolution(std::wstring, DateTools.Resolution)}. The former
 * sets the default date resolution for all fields, whereas the latter can
 * be used to set field specific date resolutions. Field specific date
 * resolutions take, if set, precedence over the default date resolution.
 * </p>
 * <p>
 * If you don't use {@link DateTools} in your index, you can create your own
 * query parser that inherits QueryParser and overwrites
 * {@link #getRangeQuery(std::wstring, std::wstring, std::wstring, bool, bool)} to
 * use a different method for date conversion.
 * </p>
 *
 * <p>Note that QueryParser is <em>not</em> thread-safe.</p>
 *
 * <p><b>NOTE</b>: there is a new QueryParser in contrib, which matches
 * the same syntax as this class, but is more modular,
 * enabling substantial customization to how a query is created.
 */
class QueryParser : public QueryParserBase, public QueryParserConstants
{
  GET_CLASS_NAME(QueryParser)
  /** The default operator for parsing queries.
   * Use {@link QueryParserBase#setDefaultOperator} to change it.
   */
public:
  enum class Operator { GET_CLASS_NAME(Operator) OR, AND };

  /** default split on whitespace behavior */
public:
  static constexpr bool DEFAULT_SPLIT_ON_WHITESPACE = false;

  /** Create a query parser.
   *  @param f  the default field for query terms.
   *  @param a   used to find terms in the query text.
   */
  QueryParser(const std::wstring &f, std::shared_ptr<Analyzer> a);

  /**
   * Set to true if phrase queries will be automatically generated
   * when the analyzer returns more than one term from whitespace
   * delimited text.
   * NOTE: this behavior may not be suitable for all languages.
   * <p>
   * Set to false if phrase queries should only be generated when
   * surrounded by double quotes.
   * <p>
   * The combination splitOnWhitespace=false and autoGeneratePhraseQueries=true
   * is disallowed.  See <a
   * href="https://issues.apache.org/jira/browse/LUCENE-7533">LUCENE-7533</a>.
   */
  void setAutoGeneratePhraseQueries(bool value) override;

  /**
   * @see #setSplitOnWhitespace(bool)
   */
  virtual bool getSplitOnWhitespace();

  /**
   * Whether query text should be split on whitespace prior to analysis.
   * Default is <code>{@value #DEFAULT_SPLIT_ON_WHITESPACE}</code>.
   * <p>
   * The combination splitOnWhitespace=false and autoGeneratePhraseQueries=true
   * is disallowed.  See <a
   * href="https://issues.apache.org/jira/browse/LUCENE-7533">LUCENE-7533</a>.
   */
  virtual void setSplitOnWhitespace(bool splitOnWhitespace);

private:
  bool splitOnWhitespace = DEFAULT_SPLIT_ON_WHITESPACE;
  static std::shared_ptr<Set<int>> disallowedPostMultiTerm;
  static bool allowedPostMultiTerm(int tokenKind);

  // *   Query  ::= ( Clause )*
  // *   Clause ::= ["+", "-"] [<TERM> ":"] ( <TERM> | "(" Query ")" )
public:
  int Conjunction() ;

  int Modifiers() ;

  // This makes sure that there is no garbage after the query string
  std::shared_ptr<Query>
  TopLevelQuery(const std::wstring &field)  final override;

  std::shared_ptr<Query> Query(const std::wstring &field) ;

  std::shared_ptr<Query>
  Clause(const std::wstring &field) ;

  std::shared_ptr<Query> Term(const std::wstring &field) ;

  /** Returns the first query if splitOnWhitespace=true or otherwise the entire
   * produced query */
  std::shared_ptr<Query> MultiTerm(const std::wstring &field,
                                   std::deque<std::shared_ptr<BooleanClause>>
                                       &clauses) ;

private:
  bool jj_2_1(int xla);

  bool jj_2_2(int xla);

  bool jj_2_3(int xla);

  bool jj_3R_3();

  bool jj_3R_6();

  bool jj_3R_5();

  bool jj_3R_4();

  bool jj_3_2();

  bool jj_3_1();

  bool jj_3R_7();

  bool jj_3_3();

  /** Generated Token Manager. */
public:
  std::shared_ptr<QueryParserTokenManager> token_source;
  /** Current token. */
  std::shared_ptr<Token> token;
  /** Next token. */
  std::shared_ptr<Token> jj_nt;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int jj_ntk_ = 0;
  std::shared_ptr<Token> jj_scanpos, jj_lastpos;
  int jj_la = 0;
  /** Whether we are looking ahead. */
  bool jj_lookingAhead = false;
  bool jj_semLA = false;
  int jj_gen = 0;
  std::deque<int> const jj_la1 = std::deque<int>(25);
  static std::deque<int> jj_la1_0;
  static std::deque<int> jj_la1_1;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static QueryParser::StaticConstructor staticConstructor;

  static void jj_la1_init_0();
  static void jj_la1_init_1();
  std::deque<std::shared_ptr<JJCalls>> const jj_2_rtns =
      std::deque<std::shared_ptr<JJCalls>>(3);
  bool jj_rescan = false;
  int jj_gc = 0;

  /** Constructor with user supplied CharStream. */
protected:
  QueryParser(std::shared_ptr<CharStream> stream);

  /** Reinitialise. */
public:
  void ReInit(std::shared_ptr<CharStream> stream) override;

  /** Constructor with generated Token Manager. */
protected:
  QueryParser(std::shared_ptr<QueryParserTokenManager> tm);

  /** Reinitialise. */
public:
  virtual void ReInit(std::shared_ptr<QueryParserTokenManager> tm);

private:
  std::shared_ptr<Token> jj_consume_token(int kind) ;

private:
  class LookaheadSuccess final : public java::lang::Error
  {
    GET_CLASS_NAME(LookaheadSuccess)

  protected:
    std::shared_ptr<LookaheadSuccess> shared_from_this()
    {
      return std::static_pointer_cast<LookaheadSuccess>(
          java.lang.Error::shared_from_this());
    }
  };

private:
  const std::shared_ptr<LookaheadSuccess> jj_ls =
      std::make_shared<LookaheadSuccess>();
  bool jj_scan_token(int kind);

  /** Get the next Token. */
public:
  std::shared_ptr<Token> getNextToken();

  /** Get the specific Token. */
  std::shared_ptr<Token> getToken(int index);

private:
  int jj_ntk();

  std::deque<std::deque<int>> jj_expentries = std::deque<std::deque<int>>();
  std::deque<int> jj_expentry;
  int jj_kind = -1;
  std::deque<int> jj_lasttokens = std::deque<int>(100);
  int jj_endpos = 0;

  void jj_add_error_token(int kind, int pos);

  /** Generate ParseException. */
public:
  virtual std::shared_ptr<ParseException> generateParseException();

  /** Enable tracing. */
  void enable_tracing();

  /** Disable tracing. */
  void disable_tracing();

private:
  void jj_rescan_token();

  void jj_save(int index, int xla);

public:
  class JJCalls final : public std::enable_shared_from_this<JJCalls>
  {
    GET_CLASS_NAME(JJCalls)
  public:
    int gen = 0;
    std::shared_ptr<Token> first;
    int arg = 0;
    std::shared_ptr<JJCalls> next;
  };

protected:
  std::shared_ptr<QueryParser> shared_from_this()
  {
    return std::static_pointer_cast<QueryParser>(
        QueryParserBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::classic
