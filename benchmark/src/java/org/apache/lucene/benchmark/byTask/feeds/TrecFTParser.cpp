using namespace std;

#include "TrecFTParser.h"
#include "DocData.h"
#include "TrecContentSource.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{

const wstring TrecFTParser::DATE = L"<DATE>";
const wstring TrecFTParser::DATE_END = L"</DATE>";
const wstring TrecFTParser::HEADLINE = L"<HEADLINE>";
const wstring TrecFTParser::HEADLINE_END = L"</HEADLINE>";

shared_ptr<DocData>
TrecFTParser::parse(shared_ptr<DocData> docData, const wstring &name,
                    shared_ptr<TrecContentSource> trecSrc,
                    shared_ptr<StringBuilder> docBuf,
                    ParsePathType pathType) 
{
  int mark = 0; // that much is skipped

  // date...
  Date date = nullptr;
  wstring dateStr = extract(docBuf, DATE, DATE_END, -1, nullptr);
  if (dateStr != L"") {
    date = trecSrc->parseDate(dateStr);
  }

  // title...
  wstring title = extract(docBuf, HEADLINE, HEADLINE_END, -1, nullptr);

  docData->clear();
  docData->setName(name);
  docData->setDate(date);
  docData->setTitle(title);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  docData->setBody(stripTags(docBuf, mark).toString());
  return docData;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds