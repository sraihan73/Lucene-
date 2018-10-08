using namespace std;

#include "ParseException.h"

namespace org::apache::lucene::queryparser::classic
{

ParseException::ParseException(
    shared_ptr<Token> currentTokenVal,
    std::deque<std::deque<int>> &expectedTokenSequencesVal,
    std::deque<wstring> &tokenImageVal)
    : Exception(
          initialise(currentTokenVal, expectedTokenSequencesVal, tokenImageVal))
{
  currentToken = currentTokenVal;
  expectedTokenSequences = expectedTokenSequencesVal;
  tokenImage = tokenImageVal;
}

ParseException::ParseException() : Exception() {}

ParseException::ParseException(const wstring &message) : Exception(message) {}

wstring ParseException::initialise(
    shared_ptr<Token> currentToken,
    std::deque<std::deque<int>> &expectedTokenSequences,
    std::deque<wstring> &tokenImage)
{
  wstring eol = System::getProperty(L"line.separator", L"\n");
  shared_ptr<StringBuilder> expected = make_shared<StringBuilder>();
  int maxSize = 0;
  for (int i = 0; i < expectedTokenSequences.size(); i++) {
    if (maxSize < expectedTokenSequences[i].size()) {
      maxSize = expectedTokenSequences[i].size();
    }
    for (int j = 0; j < expectedTokenSequences[i].size(); j++) {
      expected->append(tokenImage[expectedTokenSequences[i][j]])->append(L' ');
    }
    if (expectedTokenSequences[i][expectedTokenSequences[i].size() - 1] != 0) {
      expected->append(L"...");
    }
    expected->append(eol)->append(L"    ");
  }
  wstring retval = L"Encountered \"";
  shared_ptr<Token> tok = currentToken->next;
  for (int i = 0; i < maxSize; i++) {
    if (i != 0) {
      retval += L" ";
    }
    if (tok->kind == 0) {
      retval += tokenImage[0];
      break;
    }
    retval += L" " + tokenImage[tok->kind];
    retval += L" \"";
    retval += add_escapes(tok->image);
    retval += L" \"";
    tok = tok->next;
  }
  retval += L"\" at line " + to_wstring(currentToken->next->beginLine) +
            L", column " + to_wstring(currentToken->next->beginColumn);
  retval += L"." + eol;
  if (expectedTokenSequences.size() == 1) {
    retval += L"Was expecting:" + eol + L"    ";
  } else {
    retval += L"Was expecting one of:" + eol + L"    ";
  }
  retval += expected->toString();
  return retval;
}

wstring ParseException::add_escapes(const wstring &str)
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
} // namespace org::apache::lucene::queryparser::classic