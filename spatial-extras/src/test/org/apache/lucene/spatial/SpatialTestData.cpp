using namespace std;

#include "SpatialTestData.h"

namespace org::apache::lucene::spatial
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;

shared_ptr<Iterator<std::shared_ptr<SpatialTestData>>>
SpatialTestData::getTestData(shared_ptr<InputStream> in_,
                             shared_ptr<SpatialContext> ctx) 
{
  deque<std::shared_ptr<SpatialTestData>> results =
      deque<std::shared_ptr<SpatialTestData>>();
  shared_ptr<BufferedReader> bufInput = make_shared<BufferedReader>(
      make_shared<InputStreamReader>(in_, StandardCharsets::UTF_8));
  try {
    wstring line;
    while ((line = bufInput->readLine()) != L"") {
      if (line.length() == 0 || line[0] == L'#') {
        continue;
      }

      shared_ptr<SpatialTestData> data = make_shared<SpatialTestData>();
      std::deque<wstring> vals = line.split(L"\t");
      if (vals.size() != 3) {
        throw runtime_error(
            L"bad format; expecting 3 tab-separated values for line: " + line);
      }
      data->id = vals[0];
      data->name = vals[1];
      try {
        data->shape = ctx->readShapeFromWkt(vals[2]);
      } catch (const ParseException &e) {
        throw runtime_error(e);
      }
      results.push_back(data);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    bufInput->close();
  }
  return results.begin();
}
} // namespace org::apache::lucene::spatial