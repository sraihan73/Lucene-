#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

/* Generated By:JavaCC: Do not edit this line. TokenMgrError.java Version 5.0 */
/* JavaCCOptions: */
namespace org::apache::lucene::queryparser::surround::parser
{

/** Token Manager Error. */
class TokenMgrError : public Error
{
  GET_CLASS_NAME(TokenMgrError)

  /**
   * The version identifier for this Serializable class.
   * Increment only if the <i>serialized</i> form of the
   * class changes.
   */
private:
  static constexpr int64_t serialVersionUID = 1LL;

  /*
   * Ordinals for various reasons why an Error of this type can be thrown.
   */

  /**
   * Lexical error occurred.
   */
public:
  static constexpr int LEXICAL_ERROR = 0;

  /**
   * An attempt was made to create a second instance of a static token manager.
   */
  static constexpr int STATIC_LEXER_ERROR = 1;

  /**
   * Tried to change to an invalid lexical state.
   */
  static constexpr int INVALID_LEXICAL_STATE = 2;

  /**
   * Detected (and bailed out of) an infinite loop in the token manager.
   */
  static constexpr int LOOP_DETECTED = 3;

  /**
   * Indicates the reason why the exception is thrown. It will have
   * one of the above 4 values.
   */
  int errorCode = 0;

  /**
   * Replaces unprintable characters by their escaped (or unicode escaped)
   * equivalents in the given string
   */
protected:
  static std::wstring addEscapes(const std::wstring &str);

  /**
   * Returns a detailed message for the Error when it is thrown by the
   * token manager to indicate a lexical error.
   * Parameters :
   *    EOFSeen     : indicates if EOF caused the lexical error
   *    curLexState : lexical state in which this error occurred
   *    errorLine   : line number when the error occurred
   *    errorColumn : column number when the error occurred
   *    errorAfter  : prefix that was seen before this error occurred
   *    curchar     : the offending character
   * Note: You can customize the lexical error message by modifying this method.
   */
  static std::wstring LexicalError(bool EOFSeen, int lexState, int errorLine,
                                   int errorColumn,
                                   const std::wstring &errorAfter,
                                   wchar_t curChar);

  /**
   * You can also modify the body of this method to customize your error
   * messages. For example, cases like LOOP_DETECTED and INVALID_LEXICAL_STATE
   * are not of end-users concern, so you can return something like :
   *
   *     "Internal Error : Please file a bug report .... "
   *
   * from this method for such cases in the release version of your parser.
   */
public:
  virtual std::wstring getMessage();

  /*
   * Constructors of various flavors follow.
   */

  /** No arg constructor. */
  TokenMgrError();

  /** Constructor with message and reason. */
  TokenMgrError(const std::wstring &message, int reason);

  /** Full Constructor. */
  TokenMgrError(bool EOFSeen, int lexState, int errorLine, int errorColumn,
                const std::wstring &errorAfter, wchar_t curChar, int reason);

protected:
  std::shared_ptr<TokenMgrError> shared_from_this()
  {
    return std::static_pointer_cast<TokenMgrError>(Error::shared_from_this());
  }
};
/* JavaCC - OriginalChecksum=dcdd5ccde13b91bcd8f76a86ca618852 (do not edit this
 * line) */

} // #include  "core/src/java/org/apache/lucene/queryparser/surround/parser/
