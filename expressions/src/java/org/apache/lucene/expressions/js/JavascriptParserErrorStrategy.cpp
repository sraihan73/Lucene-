using namespace std;

#include "JavascriptParserErrorStrategy.h"

namespace org::apache::lucene::expressions::js
{
using org::antlr::v4::runtime::DefaultErrorStrategy;
using org::antlr::v4::runtime::InputMismatchException;
using org::antlr::v4::runtime::NoViableAltException;
using org::antlr::v4::runtime::Parser;
using org::antlr::v4::runtime::RecognitionException;
using org::antlr::v4::runtime::Token;

void JavascriptParserErrorStrategy::recover(shared_ptr<Parser> recognizer,
                                            shared_ptr<RecognitionException> re)
{
  shared_ptr<Token> token = re->getOffendingToken();
  wstring message;

  if (token == nullptr) {
    message = L"error " + getTokenErrorDisplay(token);
  } else if (std::dynamic_pointer_cast<InputMismatchException>(re) != nullptr) {
    message = L"unexpected token " + getTokenErrorDisplay(token) +
              L" on line (" + token->getLine() + L") position (" +
              token->getCharPositionInLine() + L")" +
              // C++ TODO: There is no native C++ equivalent to 'toString':
              L" was expecting one of " +
              re->getExpectedTokens()->toString(recognizer->getVocabulary());
  } else if (std::dynamic_pointer_cast<NoViableAltException>(re) != nullptr) {
    if (token->getType() == JavascriptParser::EOF) {
      message = L"unexpected end of expression";
    } else {
      message = L"invalid sequence of tokens near " +
                getTokenErrorDisplay(token) + L" on line (" + token->getLine() +
                L") position (" + token->getCharPositionInLine() + L")";
    }
  } else {
    message = L" unexpected token near " + getTokenErrorDisplay(token) +
              L" on line (" + token->getLine() + L") position (" +
              token->getCharPositionInLine() + L")";
  }

  shared_ptr<ParseException> parseException =
      make_shared<ParseException>(message, token->getStartIndex());
  parseException->initCause(re);
  throw runtime_error(parseException);
}

shared_ptr<Token> JavascriptParserErrorStrategy::recoverInline(
    shared_ptr<Parser> recognizer) 
{
  shared_ptr<Token> token = recognizer->getCurrentToken();
  wstring message =
      L"unexpected token " + getTokenErrorDisplay(token) + L" on line (" +
      token->getLine() + L") position (" + token->getCharPositionInLine() +
      L")" +
      // C++ TODO: There is no native C++ equivalent to 'toString':
      L" was expecting one of " +
      recognizer->getExpectedTokens()->toString(recognizer->getVocabulary());
  shared_ptr<ParseException> parseException =
      make_shared<ParseException>(message, token->getStartIndex());
  throw runtime_error(parseException);
}

void JavascriptParserErrorStrategy::sync(shared_ptr<Parser> recognizer) {}
} // namespace org::apache::lucene::expressions::js