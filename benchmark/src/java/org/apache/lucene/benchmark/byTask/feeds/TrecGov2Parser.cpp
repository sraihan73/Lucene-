using namespace std;

#include "TrecGov2Parser.h"
#include "DocData.h"
#include "TrecContentSource.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{

const wstring TrecGov2Parser::DATE = L"Date: ";
const wstring TrecGov2Parser::DATE_END = TrecContentSource::NEW_LINE;
const wstring TrecGov2Parser::DOCHDR = L"<DOCHDR>";
const wstring TrecGov2Parser::TERMINATING_DOCHDR = L"</DOCHDR>";

shared_ptr<DocData>
TrecGov2Parser::parse(shared_ptr<DocData> docData, const wstring &name,
                      shared_ptr<TrecContentSource> trecSrc,
                      shared_ptr<StringBuilder> docBuf,
                      ParsePathType pathType) 
{
  // skip some of the non-html text, optionally set date
  Date date = nullptr;
  int start = 0;
  constexpr int h1 = docBuf->find(DOCHDR);
  if (h1 >= 0) {
    constexpr int h2 = docBuf->find(TERMINATING_DOCHDR, h1);
    const wstring dateStr = extract(docBuf, DATE, DATE_END, h2, nullptr);
    if (dateStr != L"") {
      date = trecSrc->parseDate(dateStr);
    }
    start = h2 + TERMINATING_DOCHDR.length();
  }
  const wstring html = docBuf->substr(start);
  return trecSrc->getHtmlParser()->parse(
      docData, name, date, make_shared<StringReader>(html), trecSrc);
}
} // namespace org::apache::lucene::benchmark::byTask::feeds