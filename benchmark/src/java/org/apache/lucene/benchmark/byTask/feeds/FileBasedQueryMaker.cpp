using namespace std;

#include "FileBasedQueryMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../queryparser/src/java/org/apache/lucene/queryparser/classic/ParseException.h"
#include "../../../../../../../../../queryparser/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"
#include "../tasks/NewAnalyzerTask.h"
#include "DocMaker.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using Query = org::apache::lucene::search::Query;
using NewAnalyzerTask =
    org::apache::lucene::benchmark::byTask::tasks::NewAnalyzerTask;
using IOUtils = org::apache::lucene::util::IOUtils;

std::deque<std::shared_ptr<Query>>
FileBasedQueryMaker::prepareQueries() 
{

  shared_ptr<Analyzer> anlzr = NewAnalyzerTask::createAnalyzer(config->get(
      L"analyzer", L"org.apache.lucene.analysis.standard.StandardAnalyzer"));
  wstring defaultField =
      config->get(L"file.query.maker.default.field", DocMaker::BODY_FIELD);
  shared_ptr<QueryParser> qp = make_shared<QueryParser>(defaultField, anlzr);
  qp->setAllowLeadingWildcard(true);

  deque<std::shared_ptr<Query>> qq = deque<std::shared_ptr<Query>>();
  wstring fileName = config->get(L"file.query.maker.file", L"");
  if (fileName != L"") {
    shared_ptr<Path> path = Paths->get(fileName);
    shared_ptr<Reader> reader = nullptr;
    // note: we use a decoding reader, so if your queries are screwed up you
    // know
    if (Files::exists(path)) {
      reader = Files::newBufferedReader(path, StandardCharsets::UTF_8);
    } else {
      // see if we can find it as a resource
      shared_ptr<InputStream> asStream =
          FileBasedQueryMaker::typeid->getClassLoader().getResourceAsStream(
              fileName);
      if (asStream != nullptr) {
        reader = IOUtils::getDecodingReader(asStream, StandardCharsets::UTF_8);
      }
    }
    if (reader != nullptr) {
      try {
        shared_ptr<BufferedReader> buffered =
            make_shared<BufferedReader>(reader);
        wstring line = L"";
        int lineNum = 0;
        while ((line = buffered->readLine()) != L"") {
          line = StringHelper::trim(line);
          if (line.length() != 0 && !StringHelper::startsWith(line, L"#")) {
            try {
              qq.push_back(qp->parse(line));
            } catch (const ParseException &e) {
              System::err::println(L"Exception: " + e->what() +
                                   L" occurred while parsing line: " +
                                   to_wstring(lineNum) + L" Text: " + line);
            }
          }
          lineNum++;
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        reader->close();
      }
    } else {
      System::err::println(L"No Reader available for: " + fileName);
    }
  }
  return qq.toArray(std::deque<std::shared_ptr<Query>>(qq.size()));
}
} // namespace org::apache::lucene::benchmark::byTask::feeds