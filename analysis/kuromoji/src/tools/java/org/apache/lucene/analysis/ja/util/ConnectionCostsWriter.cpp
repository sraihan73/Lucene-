using namespace std;

#include "ConnectionCostsWriter.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/store/DataOutput.h"
#include "../../../../../../../../../../../core/src/java/org/apache/lucene/store/OutputStreamDataOutput.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ja/dict/ConnectionCosts.h"

namespace org::apache::lucene::analysis::ja::util
{
using ConnectionCosts =
    org::apache::lucene::analysis::ja::dict::ConnectionCosts;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataOutput = org::apache::lucene::store::DataOutput;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;

// C++ NOTE: The following call to the 'RectangularVectors' helper class
// reproduces the rectangular array initialization that is automatic in Java:
// ORIGINAL LINE: public ConnectionCostsWriter(int forwardSize, int
// backwardSize)
ConnectionCostsWriter::ConnectionCostsWriter(int forwardSize, int backwardSize)
    : costs(RectangularVectors::ReturnRectangularShortVector(backwardSize,
                                                             forwardSize)),
      forwardSize(forwardSize), backwardSize(backwardSize)
{
}

void ConnectionCostsWriter::add(int forwardId, int backwardId, int cost)
{
  this->costs[backwardId][forwardId] = static_cast<short>(cost);
}

void ConnectionCostsWriter::write(const wstring &baseDir) 
{
  wstring filename = baseDir + File::separator +
                     ConnectionCosts::typeid->getName()->replace(
                         L'.', FileSystem::preferredSeparator()) +
                     ConnectionCosts::FILENAME_SUFFIX;
  (make_shared<File>(filename))->getParentFile().mkdirs();
  shared_ptr<OutputStream> os = make_shared<FileOutputStream>(filename);
  try {
    os = make_shared<BufferedOutputStream>(os);
    shared_ptr<DataOutput> *const out = make_shared<OutputStreamDataOutput>(os);
    CodecUtil::writeHeader(out, ConnectionCosts::HEADER,
                           ConnectionCosts::VERSION);
    out->writeVInt(forwardSize);
    out->writeVInt(backwardSize);
    int last = 0;
    assert(costs.size() == backwardSize);
    for (auto a : costs) {
      assert(a.size() == forwardSize);
      for (int i = 0; i < a.size(); i++) {
        int delta = static_cast<int>(a[i]) - last;
        out->writeZInt(delta);
        last = a[i];
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    os->close();
  }
}
} // namespace org::apache::lucene::analysis::ja::util