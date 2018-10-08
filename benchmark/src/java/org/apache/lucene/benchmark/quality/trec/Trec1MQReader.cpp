using namespace std;

#include "Trec1MQReader.h"
#include "../QualityQuery.h"

namespace org::apache::lucene::benchmark::quality::trec
{
using QualityQuery = org::apache::lucene::benchmark::quality::QualityQuery;

// C++ TODO: No base class can be determined:
Trec1MQReader::Trec1MQReader(const wstring &name) { this->name = name; }

std::deque<std::shared_ptr<QualityQuery>>
Trec1MQReader::readQueries(shared_ptr<BufferedReader> reader) 
{
  deque<std::shared_ptr<QualityQuery>> res =
      deque<std::shared_ptr<QualityQuery>>();
  wstring line;
  try {
    while (L"" != (line = reader->readLine())) {
      line = StringHelper::trim(line);
      if (StringHelper::startsWith(line, L"#")) {
        continue;
      }
      // id
      int k = (int)line.find(L":");
      wstring id = line.substr(0, k)->trim();
      // qtext
      wstring qtext = line.substr(k + 1)->trim();
      // we got a topic!
      unordered_map<wstring, wstring> fields =
          unordered_map<wstring, wstring>();
      fields.emplace(name, qtext);
      // System.out.println("id: "+id+" qtext: "+qtext+"  line: "+line);
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
} // namespace org::apache::lucene::benchmark::quality::trec