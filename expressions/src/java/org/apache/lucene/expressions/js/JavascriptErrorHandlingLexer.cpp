using namespace std;

#include "JavascriptErrorHandlingLexer.h"

namespace org::apache::lucene::expressions::js
{
using org::antlr::v4::runtime::CharStream;
using org::antlr::v4::runtime::LexerNoViableAltException;
using org::antlr::v4::runtime::misc::Interval;

JavascriptErrorHandlingLexer::JavascriptErrorHandlingLexer(
    shared_ptr<CharStream> charStream)
    : JavascriptLexer(charStream)
{
}

void JavascriptErrorHandlingLexer::recover(
    shared_ptr<LexerNoViableAltException> lnvae)
{
  shared_ptr<CharStream> charStream = lnvae->getInputStream();
  int startIndex = lnvae->getStartIndex();
  wstring text =
      charStream->getText(Interval::of(startIndex, charStream->index()));

  shared_ptr<ParseException> parseException = make_shared<ParseException>(
      L"unexpected character '" + getErrorDisplay(text) + L"'" + L" on line (" +
          _tokenStartLine + L") position (" + _tokenStartCharPositionInLine +
          L")",
      _tokenStartCharIndex);
  parseException->initCause(lnvae);
  throw runtime_error(parseException);
}
} // namespace org::apache::lucene::expressions::js