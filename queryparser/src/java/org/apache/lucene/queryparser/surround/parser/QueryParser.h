#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::surround::parser
{
class ParseException;
}

namespace org::apache::lucene::queryparser::surround::query
{
class SrndQuery;
}
namespace org::apache::lucene::queryparser::surround::parser
{
class Token;
}
namespace org::apache::lucene::queryparser::surround::query
{
class DistanceQuery;
}
namespace org::apache::lucene::queryparser::surround::query
{
class FieldsQuery;
}
namespace org::apache::lucene::queryparser::surround::query
{
class OrQuery;
}
namespace org::apache::lucene::queryparser::surround::query
{
class AndQuery;
}
namespace org::apache::lucene::queryparser::surround::query
{
class NotQuery;
}
namespace org::apache::lucene::queryparser::surround::parser
{
class QueryParserTokenManager;
}
namespace org::apache::lucene::queryparser::surround::parser
{
class JJCalls;
}
namespace org::apache::lucene::queryparser::surround::parser
{
class CharStream;
}

/* Generated By:JavaCC: Do not edit this line. QueryParser.java */
namespace org::apache::lucene::queryparser::surround::parser
{

using SrndQuery = org::apache::lucene::queryparser::surround::query::SrndQuery;
using FieldsQuery =
    org::apache::lucene::queryparser::surround::query::FieldsQuery;
using OrQuery = org::apache::lucene::queryparser::surround::query::OrQuery;
using AndQuery = org::apache::lucene::queryparser::surround::query::AndQuery;
using NotQuery = org::apache::lucene::queryparser::surround::query::NotQuery;
using DistanceQuery =
    org::apache::lucene::queryparser::surround::query::DistanceQuery;

/**
 * This class is generated by JavaCC.  The only method that clients should need
 * to call is {@link #parse parse()}.
GET_CLASS_NAME(is)
 *

 * <p>This parser generates queries that make use of position information
 *   (Span queries). It provides positional operators (<code>w</code> and
 *   <code>n</code>) that accept a numeric distance, as well as bool
 *   operators (<code>and</code>, <code>or</code>, and <code>not</code>,
 *   wildcards (<code>*</code> and <code>?</code>), quoting (with
 *   <code>"</code>), and boosting (via <code>^</code>).</p>

 *  <p>The operators (W, N, AND, OR, NOT) can be expressed lower-cased or
 *   upper-cased, and the non-unary operators (everything but NOT) support
 *   both infix <code>(a AND b AND c)</code> and prefix <code>AND(a, b,
 *   c)</code> notation. </p>
 
 *  <p>The W and N operators express a positional relationship among their
 *  operands.  W is ordered, and N is unordered.  The distance is 1 by
 *  default, meaning the operands are adjacent, or may be provided as a
 *  prefix from 2-99.  So, for example, 3W(a, b) means that terms a and b
 *  must appear within three positions of each other, or in other words, up
 *  to two terms may appear between a and b.  </p>
 */

class QueryParser : public std::enable_shared_from_this<QueryParser>,
                    public QueryParserConstants
{
  GET_CLASS_NAME(QueryParser)
public:
  static constexpr int MINIMUM_PREFIX_LENGTH = 3;
  static constexpr int MINIMUM_CHARS_IN_TRUNC = 3;
  static const std::wstring TRUNCATION_ERROR_MESSAGE;
  static const std::wstring BOOST_ERROR_MESSAGE;

  /* CHECKME: These should be the same as for the tokenizer. How? */
  static constexpr wchar_t TRUNCATOR = L'*';
  static constexpr wchar_t ANY_CHAR = L'?';
  static constexpr wchar_t FIELD_OPERATOR = L':';

  static std::shared_ptr<SrndQuery>
  parse(const std::wstring &query) ;

  QueryParser();

  virtual std::shared_ptr<SrndQuery>
  parse2(const std::wstring &query) ;

protected:
  virtual std::shared_ptr<SrndQuery>
  getFieldsQuery(std::shared_ptr<SrndQuery> q,
                 std::deque<std::wstring> &fieldNames);

  virtual std::shared_ptr<SrndQuery>
  getOrQuery(std::deque<std::shared_ptr<SrndQuery>> &queries, bool infix,
             std::shared_ptr<Token> orToken);

  virtual std::shared_ptr<SrndQuery>
  getAndQuery(std::deque<std::shared_ptr<SrndQuery>> &queries, bool infix,
              std::shared_ptr<Token> andToken);

  virtual std::shared_ptr<SrndQuery>
  getNotQuery(std::deque<std::shared_ptr<SrndQuery>> &queries,
              std::shared_ptr<Token> notToken);

  static int getOpDistance(const std::wstring &distanceOp);

  static void
  checkDistanceSubQueries(std::shared_ptr<DistanceQuery> distq,
                          const std::wstring &opName) ;

  virtual std::shared_ptr<SrndQuery>
  getDistanceQuery(std::deque<std::shared_ptr<SrndQuery>> &queries, bool infix,
                   std::shared_ptr<Token> dToken,
                   bool ordered) ;

  virtual std::shared_ptr<SrndQuery> getTermQuery(const std::wstring &term,
                                                  bool quoted);

  virtual bool allowedSuffix(const std::wstring &suffixed);

  virtual std::shared_ptr<SrndQuery> getPrefixQuery(const std::wstring &prefix,
                                                    bool quoted);

  virtual bool allowedTruncation(const std::wstring &truncated);

  virtual std::shared_ptr<SrndQuery>
  getTruncQuery(const std::wstring &truncated);

public:
  std::shared_ptr<SrndQuery> TopSrndQuery() ;

  std::shared_ptr<SrndQuery> FieldsQuery() ;

  std::deque<std::wstring> OptionalFields() ;

  std::shared_ptr<SrndQuery> OrQuery() ;

  std::shared_ptr<SrndQuery> AndQuery() ;

  std::shared_ptr<SrndQuery> NotQuery() ;

  std::shared_ptr<SrndQuery> NQuery() ;

  std::shared_ptr<SrndQuery> WQuery() ;

  std::shared_ptr<SrndQuery> PrimaryQuery() ;

  std::shared_ptr<SrndQuery> PrefixOperatorQuery() ;

  std::deque<std::shared_ptr<SrndQuery>>
  FieldsQueryList() ;

  std::shared_ptr<SrndQuery> SimpleTerm() ;

  void OptionalWeights(std::shared_ptr<SrndQuery> q) ;

private:
  bool jj_2_1(int xla);

  bool jj_3_1();

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
  int jj_gen = 0;
  std::deque<int> const jj_la1 = std::deque<int>(10);
  static std::deque<int> jj_la1_0;

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
  std::deque<std::shared_ptr<JJCalls>> const jj_2_rtns =
      std::deque<std::shared_ptr<JJCalls>>(1);
  bool jj_rescan = false;
  int jj_gc = 0;

  /** Constructor with user supplied CharStream. */
public:
  QueryParser(std::shared_ptr<CharStream> stream);

  /** Reinitialise. */
  virtual void ReInit(std::shared_ptr<CharStream> stream);

  /** Constructor with generated Token Manager. */
  QueryParser(std::shared_ptr<QueryParserTokenManager> tm);

  /** Reinitialise. */
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
};

} // namespace org::apache::lucene::queryparser::surround::parser