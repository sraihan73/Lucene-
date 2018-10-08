using namespace std;

#include "TrecFR94Parser.h"
#include "DocData.h"
#include "TrecContentSource.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{

const wstring TrecFR94Parser::TEXT = L"<TEXT>";
const wstring TrecFR94Parser::TEXT_END = L"</TEXT>";
const wstring TrecFR94Parser::DATE = L"<DATE>";
std::deque<wstring> const TrecFR94Parser::DATE_NOISE_PREFIXES = {
    L"DATE:", L"date:", L"t.c."};
const wstring TrecFR94Parser::DATE_END = L"</DATE>";

shared_ptr<DocData>
TrecFR94Parser::parse(shared_ptr<DocData> docData, const wstring &name,
                      shared_ptr<TrecContentSource> trecSrc,
                      shared_ptr<StringBuilder> docBuf,
                      ParsePathType pathType) 
{
  int mark = 0; // that much is skipped
  // optionally skip some of the text, set date (no title?)
  Date date = nullptr;
  int h1 = docBuf->find(TEXT);
  if (h1 >= 0) {
    int h2 = docBuf->find(TEXT_END, h1);
    mark = h1 + TEXT_LENGTH;
    // date...
    wstring dateStr = extract(docBuf, DATE, DATE_END, h2, DATE_NOISE_PREFIXES);
    if (dateStr != L"") {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      dateStr = stripTags(dateStr, 0).toString();
      date = trecSrc->parseDate(StringHelper::trim(dateStr));
    }
  }
  docData->clear();
  docData->setName(name);
  docData->setDate(date);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  docData->setBody(stripTags(docBuf, mark).toString());
  return docData;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds