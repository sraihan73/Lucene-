using namespace std;

#include "TokenMgrError.h"

namespace org::apache::lucene::queryparser::classic
{

wstring TokenMgrError::addEscapes(const wstring &str)
{
  shared_ptr<StringBuilder> retval = make_shared<StringBuilder>();
  wchar_t ch;
  for (int i = 0; i < str.length(); i++) {
    switch (str[i]) {
    case 0:
      continue;
    case L'\b':
      retval->append(L"\\b");
      continue;
    case L'\t':
      retval->append(L"\\t");
      continue;
    case L'\n':
      retval->append(L"\\n");
      continue;
    case L'\f':
      retval->append(L"\\f");
      continue;
    case L'\r':
      retval->append(L"\\r");
      continue;
    case L'\"':
      retval->append(L"\\\"");
      continue;
    case L'\'':
      retval->append(L"\\\'");
      continue;
    case L'\\':
      retval->append(L"\\\\");
      continue;
    default:
      if ((ch = str[i]) < 0x20 || ch > 0x7e) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wstring s = L"0000" + Integer::toString(ch, 16);
        retval->append(L"\\u" +
                       s.substr(s.length() - 4, s.length() - (s.length() - 4)));
      } else {
        retval->append(ch);
      }
      continue;
    }
  }
  return retval->toString();
}

wstring TokenMgrError::LexicalError(bool EOFSeen, int lexState, int errorLine,
                                    int errorColumn, const wstring &errorAfter,
                                    wchar_t curChar)
{
  return (L"Lexical error at line " + to_wstring(errorLine) + L", column " +
          to_wstring(errorColumn) + L".  Encountered: " +
          (EOFSeen
               ? L"<EOF> "
               : (L"\"" + addEscapes(StringHelper::toString(curChar)) + L"\"") +
                     L" (" + to_wstring(static_cast<int>(curChar)) + L"), ") +
          L"after : \"" + addEscapes(errorAfter) + L"\"");
}

wstring TokenMgrError::getMessage() { return Error::getMessage(); }

TokenMgrError::TokenMgrError() {}

TokenMgrError::TokenMgrError(const wstring &message, int reason)
    : Error(message)
{
  errorCode = reason;
}

TokenMgrError::TokenMgrError(bool EOFSeen, int lexState, int errorLine,
                             int errorColumn, const wstring &errorAfter,
                             wchar_t curChar, int reason)
    : TokenMgrError(LexicalError(EOFSeen, lexState, errorLine, errorColumn,
                                 errorAfter, curChar),
                    reason)
{
}
} // namespace org::apache::lucene::queryparser::classic