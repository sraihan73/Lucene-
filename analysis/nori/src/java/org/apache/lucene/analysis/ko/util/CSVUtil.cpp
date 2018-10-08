using namespace std;

#include "CSVUtil.h"

namespace org::apache::lucene::analysis::ko::util
{

const shared_ptr<java::util::regex::Pattern> CSVUtil::QUOTE_REPLACE_PATTERN =
    java::util::regex::Pattern::compile(L"^\"([^\"]+)\"$");
const wstring CSVUtil::ESCAPED_QUOTE = L"\"\"";

CSVUtil::CSVUtil() {} // no instance!!!

std::deque<wstring> CSVUtil::parse(const wstring &line)
{
  bool insideQuote = false;
  deque<wstring> result = deque<wstring>();
  int quoteCount = 0;
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < line.length(); i++) {
    wchar_t c = line[i];

    if (c == QUOTE) {
      insideQuote = !insideQuote;
      quoteCount++;
    }

    if (c == COMMA && !insideQuote) {
      wstring value = sb->toString();
      value = unQuoteUnEscape(value);
      result.push_back(value);
      sb->setLength(0);
      continue;
    }

    sb->append(c);
  }

  result.push_back(sb->toString());

  // Validate
  if (quoteCount % 2 != 0) {
    return std::deque<wstring>(0);
  }

  return result.toArray(std::deque<wstring>(result.size()));
}

wstring CSVUtil::unQuoteUnEscape(const wstring &original)
{
  wstring result = original;

  // Unquote
  if (result.find(L'\"') != wstring::npos) {
    shared_ptr<Matcher> m = QUOTE_REPLACE_PATTERN->matcher(original);
    if (m->matches()) {
      result = m->group(1);
    }

    // Unescape
    if (result.find(ESCAPED_QUOTE) != wstring::npos) {
      result = StringHelper::replace(result, ESCAPED_QUOTE, L"\"");
    }
  }

  return result;
}
} // namespace org::apache::lucene::analysis::ko::util