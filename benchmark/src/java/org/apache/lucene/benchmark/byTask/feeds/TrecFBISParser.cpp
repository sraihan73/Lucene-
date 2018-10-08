using namespace std;

#include "TrecFBISParser.h"
#include "DocData.h"
#include "TrecContentSource.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{

const wstring TrecFBISParser::HEADER = L"<HEADER>";
const wstring TrecFBISParser::HEADER_END = L"</HEADER>";
const wstring TrecFBISParser::DATE1 = L"<DATE1>";
const wstring TrecFBISParser::DATE1_END = L"</DATE1>";
const wstring TrecFBISParser::TI = L"<TI>";
const wstring TrecFBISParser::TI_END = L"</TI>";

shared_ptr<DocData>
TrecFBISParser::parse(shared_ptr<DocData> docData, const wstring &name,
                      shared_ptr<TrecContentSource> trecSrc,
                      shared_ptr<StringBuilder> docBuf,
                      ParsePathType pathType) 
{
  int mark = 0; // that much is skipped
  // optionally skip some of the text, set date, title
  Date date = nullptr;
  wstring title = L"";
  int h1 = docBuf->find(HEADER);
  if (h1 >= 0) {
    int h2 = docBuf->find(HEADER_END, h1);
    mark = h2 + HEADER_END_LENGTH;
    // date...
    wstring dateStr = extract(docBuf, DATE1, DATE1_END, h2, nullptr);
    if (dateStr != L"") {
      date = trecSrc->parseDate(dateStr);
    }
    // title...
    title = extract(docBuf, TI, TI_END, h2, nullptr);
  }
  docData->clear();
  docData->setName(name);
  docData->setDate(date);
  docData->setTitle(title);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  docData->setBody(stripTags(docBuf, mark).toString());
  return docData;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds