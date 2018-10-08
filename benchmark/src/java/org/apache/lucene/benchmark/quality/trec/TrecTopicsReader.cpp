using namespace std;

#include "TrecTopicsReader.h"
#include "../QualityQuery.h"

namespace org::apache::lucene::benchmark::quality::trec
{
using QualityQuery = org::apache::lucene::benchmark::quality::QualityQuery;
const wstring TrecTopicsReader::newline =
    System::getProperty(L"line.separator");

// C++ TODO: No base class can be determined:
TrecTopicsReader::TrecTopicsReader() {}

std::deque<std::shared_ptr<QualityQuery>> TrecTopicsReader::readQueries(
    shared_ptr<BufferedReader> reader) 
{
  deque<std::shared_ptr<QualityQuery>> res =
      deque<std::shared_ptr<QualityQuery>>();
  shared_ptr<StringBuilder> sb;
  try {
    while (nullptr != (sb = read(reader, L"<top>", nullptr, false, false))) {
      unordered_map<wstring, wstring> fields =
          unordered_map<wstring, wstring>();
      // id
      sb = read(reader, L"<num>", nullptr, true, false);
      int k = sb->find(L":");
      wstring id = sb->substr(k + 1)->trim();
      // title
      sb = read(reader, L"<title>", nullptr, true, false);
      k = sb->find(L">");
      wstring title = sb->substr(k + 1)->trim();
      // description
      read(reader, L"<desc>", nullptr, false, false);
      sb->setLength(0);
      wstring line = L"";
      while ((line = reader->readLine()) != L"") {
        if (StringHelper::startsWith(line, L"<narr>")) {
          break;
        }
        if (sb->length() > 0) {
          sb->append(L' ');
        }
        sb->append(line);
      }
      wstring description = sb->toString()->trim();
      // narrative
      sb->setLength(0);
      while ((line = reader->readLine()) != L"") {
        if (StringHelper::startsWith(line, L"</top>")) {
          break;
        }
        if (sb->length() > 0) {
          sb->append(L' ');
        }
        sb->append(line);
      }
      wstring narrative = sb->toString()->trim();
      // we got a topic!
      fields.emplace(L"title", title);
      fields.emplace(L"description", description);
      fields.emplace(L"narrative", narrative);
      shared_ptr<QualityQuery> topic = make_shared<QualityQuery>(id, fields);
      res.push_back(topic);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reader->close();
  }
  // sort result array (by ID)
  std::deque<std::shared_ptr<QualityQuery>> qq =
      res.toArray(std::deque<std::shared_ptr<QualityQuery>>(0));
  Arrays::sort(qq);
  return qq;
}

shared_ptr<StringBuilder>
TrecTopicsReader::read(shared_ptr<BufferedReader> reader, const wstring &prefix,
                       shared_ptr<StringBuilder> sb, bool collectMatchLine,
                       bool collectAll) 
{
  sb = (sb == nullptr ? make_shared<StringBuilder>() : sb);
  wstring sep = L"";
  while (true) {
    wstring line = reader->readLine();
    if (line == L"") {
      return nullptr;
    }
    if (StringHelper::startsWith(line, prefix)) {
      if (collectMatchLine) {
        sb->append(sep + line);
        sep = newline;
      }
      break;
    }
    if (collectAll) {
      sb->append(sep + line);
      sep = newline;
    }
  }
  // System.out.println("read: "+sb);
  return sb;
}
} // namespace org::apache::lucene::benchmark::quality::trec