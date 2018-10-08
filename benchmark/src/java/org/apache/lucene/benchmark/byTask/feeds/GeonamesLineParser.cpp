using namespace std;

#include "GeonamesLineParser.h"
#include "DocData.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{

GeonamesLineParser::GeonamesLineParser(std::deque<wstring> &header)
    : LineDocSource::LineParser(header)
{
}

void GeonamesLineParser::parseLine(shared_ptr<DocData> docData,
                                   const wstring &line)
{
  std::deque<wstring> parts =
      line.split(L"\\t", 7); // no more than first 6 fields needed

  //    Sample data line:
  // 3578267, Morne du Vitet, Morne du Vitet, 17.88333, -62.8, ...
  // ID, Name, Alternate name (unused), Lat, Lon, ...

  docData->setID(
      stoi(parts[0])); // note: overwrites ID assigned by LineDocSource
  docData->setName(parts[1]);
  wstring latitude = parts[4];
  wstring longitude = parts[5];
  docData->setBody(L"POINT(" + longitude + L" " + latitude +
                   L")"); // WKT is x y order
}
} // namespace org::apache::lucene::benchmark::byTask::feeds