using namespace std;

#include "FastDecompressionCompressingCodec.h"

namespace org::apache::lucene::codecs::compressing
{

FastDecompressionCompressingCodec::FastDecompressionCompressingCodec(
    int chunkSize, int maxDocsPerChunk, bool withSegmentSuffix, int blockSize)
    : CompressingCodec(
          L"FastDecompressionCompressingStoredFields",
          withSegmentSuffix ? L"FastDecompressionCompressingStoredFields" : L"",
          CompressionMode::FAST_DECOMPRESSION, chunkSize, maxDocsPerChunk,
          blockSize)
{
}

FastDecompressionCompressingCodec::FastDecompressionCompressingCodec()
    : FastDecompressionCompressingCodec(1 << 14, 256, false, 1024)
{
}
} // namespace org::apache::lucene::codecs::compressing