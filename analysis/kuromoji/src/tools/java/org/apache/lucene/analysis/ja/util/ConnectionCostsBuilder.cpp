using namespace std;

#include "ConnectionCostsBuilder.h"
#include "ConnectionCostsWriter.h"

namespace org::apache::lucene::analysis::ja::util
{

ConnectionCostsBuilder::ConnectionCostsBuilder() {}

shared_ptr<ConnectionCostsWriter>
ConnectionCostsBuilder::build(const wstring &filename) 
{
  shared_ptr<FileInputStream> inputStream =
      make_shared<FileInputStream>(filename);
  shared_ptr<Charset> cs = StandardCharsets::US_ASCII;
  shared_ptr<CharsetDecoder> decoder =
      cs->newDecoder()
          .onMalformedInput(CodingErrorAction::REPORT)
          .onUnmappableCharacter(CodingErrorAction::REPORT);
  shared_ptr<InputStreamReader> streamReader =
      make_shared<InputStreamReader>(inputStream, decoder);
  shared_ptr<LineNumberReader> lineReader =
      make_shared<LineNumberReader>(streamReader);

  wstring line = lineReader->readLine();
  std::deque<wstring> dimensions = line.split(L"\\s+");

  assert(dimensions.size() == 2);

  int forwardSize = stoi(dimensions[0]);
  int backwardSize = stoi(dimensions[1]);

  assert(forwardSize > 0 && backwardSize > 0);

  shared_ptr<ConnectionCostsWriter> costs =
      make_shared<ConnectionCostsWriter>(forwardSize, backwardSize);

  while ((line = lineReader->readLine()) != L"") {
    std::deque<wstring> fields = line.split(L"\\s+");

    assert(fields.size() == 3);

    int forwardId = stoi(fields[0]);
    int backwardId = stoi(fields[1]);
    int cost = stoi(fields[2]);

    costs->add(forwardId, backwardId, cost);
  }
  return costs;
}
} // namespace org::apache::lucene::analysis::ja::util