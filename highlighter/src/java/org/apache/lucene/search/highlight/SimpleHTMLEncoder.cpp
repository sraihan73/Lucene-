using namespace std;

#include "SimpleHTMLEncoder.h"

namespace org::apache::lucene::search::highlight
{

SimpleHTMLEncoder::SimpleHTMLEncoder() {}

wstring SimpleHTMLEncoder::encodeText(const wstring &originalText)
{
  return htmlEncode(originalText);
}

wstring SimpleHTMLEncoder::htmlEncode(const wstring &plainText)
{
  if (plainText == L"" || plainText.length() == 0) {
    return L"";
  }

  shared_ptr<StringBuilder> result =
      make_shared<StringBuilder>(plainText.length());

  for (int index = 0; index < plainText.length(); index++) {
    wchar_t ch = plainText[index];

    switch (ch) {
    case L'"':
      result->append(L"&quot;");
      break;
    case L'&':
      result->append(L"&amp;");
      break;
    case L'<':
      result->append(L"&lt;");
      break;
    case L'>':
      result->append(L"&gt;");
      break;
    case L'\'':
      result->append(L"&#x27;");
      break;
    case L'/':
      result->append(L"&#x2F;");
      break;
    default:
      result->append(ch);
    }
  }

  return result->toString();
}
} // namespace org::apache::lucene::search::highlight