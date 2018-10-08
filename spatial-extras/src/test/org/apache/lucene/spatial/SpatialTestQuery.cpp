using namespace std;

#include "SpatialTestQuery.h"

namespace org::apache::lucene::spatial
{
using org::locationtech::spatial4j::context::SpatialContext;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialArgsParser =
    org::apache::lucene::spatial::query::SpatialArgsParser;

shared_ptr<Iterator<std::shared_ptr<SpatialTestQuery>>>
SpatialTestQuery::getTestQueries(shared_ptr<SpatialArgsParser> parser,
                                 shared_ptr<SpatialContext> ctx,
                                 const wstring &name,
                                 shared_ptr<InputStream> in_) 
{

  deque<std::shared_ptr<SpatialTestQuery>> results =
      deque<std::shared_ptr<SpatialTestQuery>>();

  shared_ptr<BufferedReader> bufInput = make_shared<BufferedReader>(
      make_shared<InputStreamReader>(in_, StandardCharsets::UTF_8));
  try {
    wstring line;
    for (int lineNumber = 1; (line = bufInput->readLine()) != nullptr;
         lineNumber++) {
      shared_ptr<SpatialTestQuery> test = make_shared<SpatialTestQuery>();
      test->line = line;
      test->lineNumber = lineNumber;

      try {
        // skip a comment
        if (StringHelper::startsWith(line, L"[")) {
          int idx = (int)line.find(L']');
          if (idx > 0) {
            line = line.substr(idx + 1);
          }
        }

        int idx = (int)line.find(L'@');
        shared_ptr<StringTokenizer> st =
            make_shared<StringTokenizer>(line.substr(0, idx));
        while (st->hasMoreTokens()) {
          test->ids.push_back(st->nextToken()->trim());
        }
        test->args = parser->parse(line.substr(idx + 1)->trim(), ctx);
        results.push_back(test);
      } catch (const runtime_error &ex) {
        // C++ TODO: This exception's constructor requires only one argument:
        // ORIGINAL LINE: throw new RuntimeException("invalid query line:
        // "+test.line, ex);
        throw runtime_error(L"invalid query line: " + test->line);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    bufInput->close();
  }
  return results.begin();
}

wstring SpatialTestQuery::toString()
{
  if (line != L"") {
    return line;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return args->toString() + L" " + ids;
}
} // namespace org::apache::lucene::spatial