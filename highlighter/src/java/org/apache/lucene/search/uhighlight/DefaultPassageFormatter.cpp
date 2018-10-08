using namespace std;

#include "DefaultPassageFormatter.h"

namespace org::apache::lucene::search::uhighlight
{

DefaultPassageFormatter::DefaultPassageFormatter()
    : DefaultPassageFormatter(L"<b>", L"</b>", L"... ", false)
{
}

DefaultPassageFormatter::DefaultPassageFormatter(const wstring &preTag,
                                                 const wstring &postTag,
                                                 const wstring &ellipsis,
                                                 bool escape)
    : preTag(preTag), postTag(postTag), ellipsis(ellipsis), escape(escape)
{
  if (preTag == L"" || postTag == L"" || ellipsis == L"") {
    throw make_shared<NullPointerException>();
  }
}

wstring
DefaultPassageFormatter::format(std::deque<std::shared_ptr<Passage>> &passages,
                                const wstring &content)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int pos = 0;
  for (auto passage : passages) {
    // don't add ellipsis if its the first one, or if its connected.
    if (passage->getStartOffset() > pos && pos > 0) {
      sb->append(ellipsis);
    }
    pos = passage->getStartOffset();
    for (int i = 0; i < passage->getNumMatches(); i++) {
      int start = passage->getMatchStarts()[i];
      int end = passage->getMatchEnds()[i];
      // its possible to have overlapping terms
      if (start > pos) {
        append(sb, content, pos, start);
      }
      if (end > pos) {
        sb->append(preTag);
        append(sb, content, max(pos, start), end);
        sb->append(postTag);
        pos = end;
      }
    }
    // its possible a "term" from the analyzer could span a sentence boundary.
    append(sb, content, pos, max(pos, passage->getEndOffset()));
    pos = passage->getEndOffset();
  }
  return sb->toString();
}

void DefaultPassageFormatter::append(shared_ptr<StringBuilder> dest,
                                     const wstring &content, int start, int end)
{
  if (escape) {
    // note: these are the rules from owasp.org
    for (int i = start; i < end; i++) {
      wchar_t ch = content[i];
      switch (ch) {
      case L'&':
        dest->append(L"&amp;");
        break;
      case L'<':
        dest->append(L"&lt;");
        break;
      case L'>':
        dest->append(L"&gt;");
        break;
      case L'"':
        dest->append(L"&quot;");
        break;
      case L'\'':
        dest->append(L"&#x27;");
        break;
      case L'/':
        dest->append(L"&#x2F;");
        break;
      default:
        if (ch >= 0x30 && ch <= 0x39 || ch >= 0x41 && ch <= 0x5A ||
            ch >= 0x61 && ch <= 0x7A) {
          dest->append(ch);
        } else if (ch < 0xff) {
          dest->append(L"&#");
          dest->append(static_cast<int>(ch));
          dest->append(L";");
        } else {
          dest->append(ch);
        }
      }
    }
  } else {
    dest->append(content, start, end);
  }
}
} // namespace org::apache::lucene::search::uhighlight