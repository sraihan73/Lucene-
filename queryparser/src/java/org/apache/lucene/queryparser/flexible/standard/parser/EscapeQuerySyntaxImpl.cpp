using namespace std;

#include "EscapeQuerySyntaxImpl.h"

namespace org::apache::lucene::queryparser::flexible::standard::parser
{
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using UnescapedCharSequence = org::apache::lucene::queryparser::flexible::core::
    util::UnescapedCharSequence;
std::deque<wchar_t> const EscapeQuerySyntaxImpl::wildcardChars = {L'*', L'?'};
std::deque<wstring> const EscapeQuerySyntaxImpl::escapableTermExtraFirstChars =
    {L"+", L"-", L"@"};
std::deque<wstring> const EscapeQuerySyntaxImpl::escapableTermChars = {
    L"\"", L"<", L">", L"=", L"!", L"(", L")", L"^",
    L"[",  L"{", L":", L"]", L"}", L"~", L"/"};
std::deque<wstring> const EscapeQuerySyntaxImpl::escapableQuotedChars = {
    L"\""};
std::deque<wstring> const EscapeQuerySyntaxImpl::escapableWhiteChars = {
    L" ", L"\t", L"\n", L"\r", L"\f", L"\b", L"\u3000"};
std::deque<wstring> const EscapeQuerySyntaxImpl::escapableWordTokens = {
    L"AND",    L"OR",       L"NOT",       L"TO",
    L"WITHIN", L"SENTENCE", L"PARAGRAPH", L"INORDER"};

shared_ptr<std::wstring>
EscapeQuerySyntaxImpl::escapeChar(shared_ptr<std::wstring> str,
                                  shared_ptr<Locale> locale)
{
  if (str == nullptr || str->length() == 0) {
    return str;
  }

  shared_ptr<std::wstring> buffer = str;

  // regular escapable Char for terms
  for (int i = 0; i < escapableTermChars.size(); i++) {
    buffer = replaceIgnoreCase(
        buffer, escapableTermChars[i].toLowerCase(locale), L"\\", locale);
  }

  // First Character of a term as more escaping chars
  for (int i = 0; i < escapableTermExtraFirstChars.size(); i++) {
    if (buffer->charAt(0) == escapableTermExtraFirstChars[i][0]) {
      buffer = L"\\" + StringHelper::toString(buffer->charAt(0)) +
               buffer->substr(1, buffer->length() - 1);
      break;
    }
  }

  return buffer;
}

shared_ptr<std::wstring>
EscapeQuerySyntaxImpl::escapeQuoted(shared_ptr<std::wstring> str,
                                    shared_ptr<Locale> locale)
{
  if (str == nullptr || str->length() == 0) {
    return str;
  }

  shared_ptr<std::wstring> buffer = str;

  for (int i = 0; i < escapableQuotedChars.size(); i++) {
    buffer = replaceIgnoreCase(
        buffer, escapableTermChars[i].toLowerCase(locale), L"\\", locale);
  }
  return buffer;
}

shared_ptr<std::wstring>
EscapeQuerySyntaxImpl::escapeTerm(shared_ptr<std::wstring> term,
                                  shared_ptr<Locale> locale)
{
  if (term == nullptr) {
    return term;
  }

  // Escape single Chars
  term = escapeChar(term, locale);
  term = escapeWhiteChar(term, locale);

  // Escape Parser Words
  for (int i = 0; i < escapableWordTokens.size(); i++) {
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted: C++ TODO: There is no native C++ equivalent to 'toString':
    if (escapableWordTokens[i].equalsIgnoreCase(term->toString())) {
      return L"\\" + term;
    }
  }
  return term;
}

shared_ptr<std::wstring> EscapeQuerySyntaxImpl::replaceIgnoreCase(
    shared_ptr<std::wstring> string, shared_ptr<std::wstring> sequence1,
    shared_ptr<std::wstring> escapeChar, shared_ptr<Locale> locale)
{
  if (escapeChar == nullptr || sequence1 == nullptr || string == nullptr) {
    throw make_shared<NullPointerException>();
  }

  // empty string case
  int count = string->length();
  int sequence1Length = sequence1->length();
  if (sequence1Length == 0) {
    shared_ptr<StringBuilder> result =
        make_shared<StringBuilder>((count + 1) * escapeChar->length());
    result->append(escapeChar);
    for (int i = 0; i < count; i++) {
      result->append(string->charAt(i));
      result->append(escapeChar);
    }
    return result->toString();
  }

  // normal case
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  wchar_t first = sequence1->charAt(0);
  int start = 0, copyStart = 0, firstIndex;
  while (start < count) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    if ((firstIndex = string->toString()->toLowerCase(locale)->find(
             first, start)) == -1) {
      break;
    }
    bool found = true;
    if (sequence1->length() > 1) {
      if (firstIndex + sequence1Length > count) {
        break;
      }
      for (int i = 1; i < sequence1Length; i++) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        if (string->toString()->toLowerCase(locale)->charAt(firstIndex + i) !=
            sequence1->charAt(i)) {
          found = false;
          break;
        }
      }
    }
    if (found) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      result->append(
          string->toString()->substr(copyStart, firstIndex - copyStart));
      result->append(escapeChar);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      result->append(string->toString()->substr(firstIndex, sequence1Length));
      copyStart = start = firstIndex + sequence1Length;
    } else {
      start = firstIndex + 1;
    }
  }
  if (result->length() == 0 && copyStart == 0) {
    return string;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  result->append(string->toString()->substr(copyStart));
  return result->toString();
}

shared_ptr<std::wstring>
EscapeQuerySyntaxImpl::escapeWhiteChar(shared_ptr<std::wstring> str,
                                       shared_ptr<Locale> locale)
{
  if (str == nullptr || str->length() == 0) {
    return str;
  }

  shared_ptr<std::wstring> buffer = str;

  for (int i = 0; i < escapableWhiteChars.size(); i++) {
    buffer = replaceIgnoreCase(
        buffer, escapableWhiteChars[i].toLowerCase(locale), L"\\", locale);
  }
  return buffer;
}

shared_ptr<std::wstring>
EscapeQuerySyntaxImpl::escape(shared_ptr<std::wstring> text,
                              shared_ptr<Locale> locale, Type type)
{
  if (text == nullptr || text->length() == 0) {
    return text;
  }

  // escape wildcards and the escape char (this has to be perform before
  // anything else)
  // since we need to preserve the UnescapedCharSequence and escape the
  // original escape chars
  if (std::dynamic_pointer_cast<UnescapedCharSequence>(text) != nullptr) {
    text = (std::static_pointer_cast<UnescapedCharSequence>(text))
               ->toStringEscaped(wildcardChars);
  } else {
    text = (make_shared<UnescapedCharSequence>(text))
               ->toStringEscaped(wildcardChars);
  }

  if (type == Type::STRING) {
    return escapeQuoted(text, locale);
  } else {
    return escapeTerm(text, locale);
  }
}

shared_ptr<UnescapedCharSequence> EscapeQuerySyntaxImpl::discardEscapeChar(
    shared_ptr<std::wstring> input) 
{
  // Create char array to hold unescaped char sequence
  std::deque<wchar_t> output(input->length());
  std::deque<bool> wasEscaped(input->length());

  // The length of the output can be less than the input
  // due to discarded escape chars. This variable holds
  // the actual length of the output
  int length = 0;

  // We remember whether the last processed character was
  // an escape character
  bool lastCharWasEscapeChar = false;

  // The multiplier the current unicode digit must be multiplied with.
  // E. g. the first digit must be multiplied with 16^3, the second with
  // 16^2...
  int codePointMultiplier = 0;

  // Used to calculate the codepoint of the escaped unicode character
  int codePoint = 0;

  for (int i = 0; i < input->length(); i++) {
    wchar_t curChar = input->charAt(i);
    if (codePointMultiplier > 0) {
      codePoint += hexToInt(curChar) * codePointMultiplier;
      codePointMultiplier =
          static_cast<int>(static_cast<unsigned int>(codePointMultiplier) >> 4);
      if (codePointMultiplier == 0) {
        output[length++] = static_cast<wchar_t>(codePoint);
        codePoint = 0;
      }
    } else if (lastCharWasEscapeChar) {
      if (curChar == L'u') {
        // found an escaped unicode character
        codePointMultiplier = 16 * 16 * 16;
      } else {
        // this character was escaped
        output[length] = curChar;
        wasEscaped[length] = true;
        length++;
      }
      lastCharWasEscapeChar = false;
    } else {
      if (curChar == L'\\') {
        lastCharWasEscapeChar = true;
      } else {
        output[length] = curChar;
        length++;
      }
    }
  }

  if (codePointMultiplier > 0) {
    throw make_shared<ParseException>(make_shared<MessageImpl>(
        QueryParserMessages::INVALID_SYNTAX_ESCAPE_UNICODE_TRUNCATION));
  }

  if (lastCharWasEscapeChar) {
    throw make_shared<ParseException>(make_shared<MessageImpl>(
        QueryParserMessages::INVALID_SYNTAX_ESCAPE_CHARACTER));
  }

  return make_shared<UnescapedCharSequence>(output, wasEscaped, 0, length);
}

int EscapeQuerySyntaxImpl::hexToInt(wchar_t c) 
{
  if (L'0' <= c && c <= L'9') {
    return c - L'0';
  } else if (L'a' <= c && c <= L'f') {
    return c - L'a' + 10;
  } else if (L'A' <= c && c <= L'F') {
    return c - L'A' + 10;
  } else {
    throw make_shared<ParseException>(make_shared<MessageImpl>(
        QueryParserMessages::INVALID_SYNTAX_ESCAPE_NONE_HEX_UNICODE, c));
  }
}
} // namespace org::apache::lucene::queryparser::flexible::standard::parser