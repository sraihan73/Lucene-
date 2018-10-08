using namespace std;

#include "TrecDocParser.h"
#include "DocData.h"
#include "TrecContentSource.h"
#include "TrecFBISParser.h"
#include "TrecFR94Parser.h"
#include "TrecFTParser.h"
#include "TrecGov2Parser.h"
#include "TrecLATimesParser.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{

const unordered_map<ParsePathType, std::shared_ptr<TrecDocParser>>
    TrecDocParser::pathType2parser =
        unordered_map<ParsePathType, std::shared_ptr<TrecDocParser>>();

TrecDocParser::StaticConstructor::StaticConstructor()
{
  pathType2parser.emplace(ParsePathType::GOV2, make_shared<TrecGov2Parser>());
  pathType2parser.emplace(ParsePathType::FBIS, make_shared<TrecFBISParser>());
  pathType2parser.emplace(ParsePathType::FR94, make_shared<TrecFR94Parser>());
  pathType2parser.emplace(ParsePathType::FT, make_shared<TrecFTParser>());
  pathType2parser.emplace(ParsePathType::LATIMES,
                          make_shared<TrecLATimesParser>());
  for (ParsePathType ppt : ParsePathType::values()) {
    pathName2Type.emplace(ppt.name()->toUpperCase(Locale::ROOT), ppt);
  }
}

TrecDocParser::StaticConstructor TrecDocParser::staticConstructor;
const unordered_map<wstring, ParsePathType> TrecDocParser::pathName2Type =
    unordered_map<wstring, ParsePathType>();

TrecDocParser::ParsePathType TrecDocParser::pathType(shared_ptr<Path> f)
{
  int pathLength = 0;
  while (f != nullptr && f->getFileName() != nullptr &&
         ++pathLength < MAX_PATH_LENGTH) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    ParsePathType ppt =
        pathName2Type[f->getFileName()->toString()->toUpperCase(Locale::ROOT)];
    if (ppt != nullptr) {
      return ppt;
    }
    f = f->getParent();
  }
  return DEFAULT_PATH_TYPE;
}

wstring TrecDocParser::stripTags(shared_ptr<StringBuilder> buf, int start)
{
  return stripTags(buf->substr(start), 0);
}

wstring TrecDocParser::stripTags(const wstring &buf, int start)
{
  if (start > 0) {
    buf = buf.substr(0);
  }
  return buf.replaceAll(L"<[^>]*>", L" ");
}

wstring TrecDocParser::extract(shared_ptr<StringBuilder> buf,
                               const wstring &startTag, const wstring &endTag,
                               int maxPos, std::deque<wstring> &noisePrefixes)
{
  int k1 = buf->find(startTag);
  if (k1 >= 0 && (maxPos < 0 || k1 < maxPos)) {
    k1 += startTag.length();
    int k2 = buf->find(endTag, k1);
    if (k2 >= 0 &&
        (maxPos < 0 || k2 < maxPos)) { // found end tag with allowed range
      if (noisePrefixes.size() > 0) {
        for (auto noise : noisePrefixes) {
          int k1a = buf->find(noise, k1);
          if (k1a >= 0 && k1a < k2) {
            k1 = k1a + noise.length();
          }
        }
      }
      return buf->substr(k1, k2 - k1)->trim();
    }
  }
  return L"";
}
} // namespace org::apache::lucene::benchmark::byTask::feeds