using namespace std;

#include "TrecLATimesParser.h"
#include "DocData.h"
#include "TrecContentSource.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{

const wstring TrecLATimesParser::DATE = L"<DATE>";
const wstring TrecLATimesParser::DATE_END = L"</DATE>";
const wstring TrecLATimesParser::DATE_NOISE = L"day,";
const wstring TrecLATimesParser::SUBJECT = L"<SUBJECT>";
const wstring TrecLATimesParser::SUBJECT_END = L"</SUBJECT>";
const wstring TrecLATimesParser::HEADLINE = L"<HEADLINE>";
const wstring TrecLATimesParser::HEADLINE_END = L"</HEADLINE>";

shared_ptr<DocData>
TrecLATimesParser::parse(shared_ptr<DocData> docData, const wstring &name,
                         shared_ptr<TrecContentSource> trecSrc,
                         shared_ptr<StringBuilder> docBuf,
                         ParsePathType pathType) 
{
  int mark = 0; // that much is skipped

  // date...
  Date date = nullptr;
  wstring dateStr = extract(docBuf, DATE, DATE_END, -1, nullptr);
  if (dateStr != L"") {
    int d2a = (int)dateStr.find(DATE_NOISE);
    if (d2a > 0) {
      dateStr = dateStr.substr(0, d2a + 3); // we need the "day" part
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    dateStr = stripTags(dateStr, 0).toString();
    date = trecSrc->parseDate(StringHelper::trim(dateStr));
  }

  // title... first try with SUBJECT, them with HEADLINE
  wstring title = extract(docBuf, SUBJECT, SUBJECT_END, -1, nullptr);
  if (title == L"") {
    title = extract(docBuf, HEADLINE, HEADLINE_END, -1, nullptr);
  }
  if (title != L"") {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    title = stripTags(title, 0).toString()->trim();
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