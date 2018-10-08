#pragma once
#include "stringhelper.h"
/* Generated By:JavaCC: Do not edit this line. QueryParserConstants.java */
namespace org::apache::lucene::queryparser::classic
{

/**
 * Token literal values and constants.
 * Generated by org.javacc.parser.OtherFilesGen#start()
 */
class QueryParserConstants
{
  GET_CLASS_NAME(QueryParserConstants)

  /** End of File. */
public:
  static constexpr int EOF = 0;
  /** RegularExpression Id. */
  static constexpr int _NUM_CHAR = 1;
  /** RegularExpression Id. */
  static constexpr int _ESCAPED_CHAR = 2;
  /** RegularExpression Id. */
  static constexpr int _TERM_START_CHAR = 3;
  /** RegularExpression Id. */
  static constexpr int _TERM_CHAR = 4;
  /** RegularExpression Id. */
  static constexpr int _WHITESPACE = 5;
  /** RegularExpression Id. */
  static constexpr int _QUOTED_CHAR = 6;
  /** RegularExpression Id. */
  static constexpr int AND = 8;
  /** RegularExpression Id. */
  static constexpr int OR = 9;
  /** RegularExpression Id. */
  static constexpr int NOT = 10;
  /** RegularExpression Id. */
  static constexpr int PLUS = 11;
  /** RegularExpression Id. */
  static constexpr int MINUS = 12;
  /** RegularExpression Id. */
  static constexpr int BAREOPER = 13;
  /** RegularExpression Id. */
  static constexpr int LPAREN = 14;
  /** RegularExpression Id. */
  static constexpr int RPAREN = 15;
  /** RegularExpression Id. */
  static constexpr int COLON = 16;
  /** RegularExpression Id. */
  static constexpr int STAR = 17;
  /** RegularExpression Id. */
  static constexpr int CARAT = 18;
  /** RegularExpression Id. */
  static constexpr int QUOTED = 19;
  /** RegularExpression Id. */
  static constexpr int TERM = 20;
  /** RegularExpression Id. */
  static constexpr int FUZZY_SLOP = 21;
  /** RegularExpression Id. */
  static constexpr int PREFIXTERM = 22;
  /** RegularExpression Id. */
  static constexpr int WILDTERM = 23;
  /** RegularExpression Id. */
  static constexpr int REGEXPTERM = 24;
  /** RegularExpression Id. */
  static constexpr int RANGEIN_START = 25;
  /** RegularExpression Id. */
  static constexpr int RANGEEX_START = 26;
  /** RegularExpression Id. */
  static constexpr int NUMBER = 27;
  /** RegularExpression Id. */
  static constexpr int RANGE_TO = 28;
  /** RegularExpression Id. */
  static constexpr int RANGEIN_END = 29;
  /** RegularExpression Id. */
  static constexpr int RANGEEX_END = 30;
  /** RegularExpression Id. */
  static constexpr int RANGE_QUOTED = 31;
  /** RegularExpression Id. */
  static constexpr int RANGE_GOOP = 32;

  /** Lexical state. */
  static constexpr int Boost = 0;
  /** Lexical state. */
  static constexpr int Range = 1;
  /** Lexical state. */
  static constexpr int DEFAULT = 2;

  /** Literal token values. */
  static std::deque<std::wstring> const tokenImage;
};

} // namespace org::apache::lucene::queryparser::classic
