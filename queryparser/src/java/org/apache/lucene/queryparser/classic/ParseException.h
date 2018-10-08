#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/classic/Token.h"

/* Generated By:JavaCC: Do not edit this line. ParseException.java Version 5.0
 */
/* JavaCCOptions:KEEP_LINE_COL=null */
namespace org::apache::lucene::queryparser::classic
{

/**
 * This exception is thrown when parse errors are encountered.
 * You can explicitly create objects of this exception type by
 * calling the method generateParseException in the generated
 * parser.
 *
 * You can modify this class to customize your error reporting
 * mechanisms so long as you retain the public fields.
 */
class ParseException : public std::runtime_error
{
  GET_CLASS_NAME(ParseException)

  /**
   * The version identifier for this Serializable class.
   * Increment only if the <i>serialized</i> form of the
   * class changes.
   */
private:
  static constexpr int64_t serialVersionUID = 1LL;

  /**
   * This constructor is used by the method "generateParseException"
   * in the generated parser.  Calling this constructor generates
   * a new object of this type with the fields "currentToken",
   * "expectedTokenSequences", and "tokenImage" set.
   */
public:
  ParseException(std::shared_ptr<Token> currentTokenVal,
                 std::deque<std::deque<int>> &expectedTokenSequencesVal,
                 std::deque<std::wstring> &tokenImageVal);

  /**
   * The following constructors are for use by you for whatever
   * purpose you can think of.  Constructing the exception in this
   * manner makes the exception behave in the normal way - i.e., as
   * documented in the class "Throwable".  The fields "errorToken",
   * "expectedTokenSequences", and "tokenImage" do not contain
   * relevant information.  The JavaCC generated code does not use
   * these constructors.
   */

  ParseException();

  /** Constructor with message. */
  ParseException(const std::wstring &message);

  /**
   * This is the last token that has been consumed successfully.  If
   * this object has been created due to a parse error, the token
   * followng this token will (therefore) be the first error token.
   */
  std::shared_ptr<Token> currentToken;

  /**
   * Each entry in this array is an array of integers.  Each array
   * of integers represents a sequence of tokens (by their ordinal
   * values) that is expected at this point of the parse.
   */
  std::deque<std::deque<int>> expectedTokenSequences;

  /**
   * This is a reference to the "tokenImage" array of the generated
   * parser within which the parse error occurred.  This array is
   * defined in the generated ...Constants interface.
   */
  std::deque<std::wstring> tokenImage;

  /**
   * It uses "currentToken" and "expectedTokenSequences" to generate a parse
   * error message and returns it.  If this object has been created
   * due to a parse error, and you do not catch it (it gets thrown
   * from the parser) the correct error message
   * gets displayed.
   */
private:
  static std::wstring
  initialise(std::shared_ptr<Token> currentToken,
             std::deque<std::deque<int>> &expectedTokenSequences,
             std::deque<std::wstring> &tokenImage);

  /**
   * The end of line string for this machine.
   */
protected:
  std::wstring eol = System::getProperty(L"line.separator", L"\n");

  /**
   * Used to convert raw characters to their escaped version
   * when these raw version cannot be used as part of an ASCII
   * string literal.
   */
public:
  static std::wstring add_escapes(const std::wstring &str);

protected:
  std::shared_ptr<ParseException> shared_from_this()
  {
    return std::static_pointer_cast<ParseException>(
        Exception::shared_from_this());
  }
};
/* JavaCC - OriginalChecksum=b187d97d5bb75c3fc63d642c1c26ac6e (do not edit this
 * line) */

} // #include  "core/src/java/org/apache/lucene/queryparser/classic/
