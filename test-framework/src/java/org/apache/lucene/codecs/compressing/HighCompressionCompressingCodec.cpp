using namespace std;

#include "HighCompressionCompressingCodec.h"

namespace org::apache::lucene::codecs::compressing
{

HighCompressionCompressingCodec::HighCompressionCompressingCodec(
    int chunkSize, int maxDocsPerChunk, bool withSegmentSuffix, int blockSize)
    : CompressingCodec(
          L"HighCompressionCompressingStoredFields",
          withSegmentSuffix ? L"HighCompressionCompressingStoredFields" : L"",
          CompressionMode::HIGH_COMPRESSION, chunkSize, maxDocsPerChunk,
          blockSize)
{
}

HighCompressionCompressingCodec::HighCompressionCompressingCodec()
    : HighCompressionCompressingCodec(61440, 512, false, 1024)
{
  // we don't worry about zlib block overhead as it's
  // not bad and try to save space instead:
}
} // namespace org::apache::lucene::codecs::compressing