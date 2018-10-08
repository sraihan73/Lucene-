using namespace std;

#include "FastCompressingCodec.h"

namespace org::apache::lucene::codecs::compressing
{

FastCompressingCodec::FastCompressingCodec(int chunkSize, int maxDocsPerChunk,
                                           bool withSegmentSuffix,
                                           int blockSize)
    : CompressingCodec(L"FastCompressingStoredFields",
                       withSegmentSuffix ? L"FastCompressingStoredFields" : L"",
                       CompressionMode::FAST, chunkSize, maxDocsPerChunk,
                       blockSize)
{
}

FastCompressingCodec::FastCompressingCodec()
    : FastCompressingCodec(1 << 14, 128, false, 1024)
{
}
} // namespace org::apache::lucene::codecs::compressing