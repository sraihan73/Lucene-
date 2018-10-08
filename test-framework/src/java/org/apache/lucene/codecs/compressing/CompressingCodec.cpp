using namespace std;

#include "CompressingCodec.h"

namespace org::apache::lucene::codecs::compressing
{
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using DummyCompressingCodec =
    org::apache::lucene::codecs::compressing::dummy::DummyCompressingCodec;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;

shared_ptr<CompressingCodec>
CompressingCodec::randomInstance(shared_ptr<Random> random, int chunkSize,
                                 int maxDocsPerChunk, bool withSegmentSuffix,
                                 int blockSize)
{
  switch (random->nextInt(4)) {
  case 0:
    return make_shared<FastCompressingCodec>(chunkSize, maxDocsPerChunk,
                                             withSegmentSuffix, blockSize);
  case 1:
    return make_shared<FastDecompressionCompressingCodec>(
        chunkSize, maxDocsPerChunk, withSegmentSuffix, blockSize);
  case 2:
    return make_shared<HighCompressionCompressingCodec>(
        chunkSize, maxDocsPerChunk, withSegmentSuffix, blockSize);
  case 3:
    return make_shared<DummyCompressingCodec>(chunkSize, maxDocsPerChunk,
                                              withSegmentSuffix, blockSize);
  default:
    throw make_shared<AssertionError>();
  }
}

shared_ptr<CompressingCodec>
CompressingCodec::randomInstance(shared_ptr<Random> random)
{
  constexpr int chunkSize =
      random->nextBoolean()
          ? RandomNumbers::randomIntBetween(random, 1, 10)
          : RandomNumbers::randomIntBetween(random, 1, 1 << 15);
  constexpr int chunkDocs =
      random->nextBoolean() ? RandomNumbers::randomIntBetween(random, 1, 10)
                            : RandomNumbers::randomIntBetween(random, 64, 1024);
  constexpr int blockSize =
      random->nextBoolean() ? RandomNumbers::randomIntBetween(random, 1, 10)
                            : RandomNumbers::randomIntBetween(random, 1, 1024);
  return randomInstance(random, chunkSize, chunkDocs, false, blockSize);
}

shared_ptr<CompressingCodec>
CompressingCodec::reasonableInstance(shared_ptr<Random> random)
{
  // e.g. defaults use 2^14 for FAST and ~ 2^16 for HIGH
  constexpr int chunkSize = TestUtil::nextInt(random, 1 << 13, 1 << 17);
  // e.g. defaults use 128 for FAST and 512 for HIGH
  constexpr int chunkDocs = TestUtil::nextInt(random, 1 << 6, 1 << 10);
  // e.g. defaults use 1024 for both cases
  constexpr int blockSize = TestUtil::nextInt(random, 1 << 9, 1 << 11);
  return randomInstance(random, chunkSize, chunkDocs, false, blockSize);
}

shared_ptr<CompressingCodec>
CompressingCodec::randomInstance(shared_ptr<Random> random,
                                 bool withSegmentSuffix)
{
  return randomInstance(
      random, RandomNumbers::randomIntBetween(random, 1, 1 << 15),
      RandomNumbers::randomIntBetween(random, 64, 1024), withSegmentSuffix,
      RandomNumbers::randomIntBetween(random, 1, 1024));
}

CompressingCodec::CompressingCodec(const wstring &name,
                                   const wstring &segmentSuffix,
                                   shared_ptr<CompressionMode> compressionMode,
                                   int chunkSize, int maxDocsPerChunk,
                                   int blockSize)
    : org::apache::lucene::codecs::FilterCodec(name,
                                               TestUtil::getDefaultCodec()),
      storedFieldsFormat(make_shared<CompressingStoredFieldsFormat>(
          name, segmentSuffix, compressionMode, chunkSize, maxDocsPerChunk,
          blockSize)),
      termVectorsFormat(make_shared<CompressingTermVectorsFormat>(
          name, segmentSuffix, compressionMode, chunkSize, blockSize))
{
}

CompressingCodec::CompressingCodec(const wstring &name,
                                   shared_ptr<CompressionMode> compressionMode,
                                   int chunkSize, int maxDocsPerChunk,
                                   int blockSize)
    : CompressingCodec(name, L"", compressionMode, chunkSize, maxDocsPerChunk,
                       blockSize)
{
}

shared_ptr<StoredFieldsFormat> CompressingCodec::storedFieldsFormat()
{
  return storedFieldsFormat_;
}

shared_ptr<TermVectorsFormat> CompressingCodec::termVectorsFormat()
{
  return termVectorsFormat_;
}

wstring CompressingCodec::toString()
{
  return getName() + L"(storedFieldsFormat=" + storedFieldsFormat_ +
         L", termVectorsFormat=" + termVectorsFormat_ + L")";
}
} // namespace org::apache::lucene::codecs::compressing