using namespace std;

#include "DummyCompressingCodec.h"

namespace org::apache::lucene::codecs::compressing::dummy
{
using CompressingCodec =
    org::apache::lucene::codecs::compressing::CompressingCodec;
using CompressionMode =
    org::apache::lucene::codecs::compressing::CompressionMode;
using Compressor = org::apache::lucene::codecs::compressing::Compressor;
using Decompressor = org::apache::lucene::codecs::compressing::Decompressor;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<org::apache::lucene::codecs::compressing::CompressionMode>
    DummyCompressingCodec::DUMMY =
        make_shared<CompressionModeAnonymousInnerClass>();

DummyCompressingCodec::CompressionModeAnonymousInnerClass::
    CompressionModeAnonymousInnerClass()
{
}

shared_ptr<Compressor>
DummyCompressingCodec::CompressionModeAnonymousInnerClass::newCompressor()
{
  return DUMMY_COMPRESSOR;
}

shared_ptr<Decompressor>
DummyCompressingCodec::CompressionModeAnonymousInnerClass::newDecompressor()
{
  return DUMMY_DECOMPRESSOR;
}

wstring DummyCompressingCodec::CompressionModeAnonymousInnerClass::toString()
{
  return L"DUMMY";
}

const shared_ptr<org::apache::lucene::codecs::compressing::Decompressor>
    DummyCompressingCodec::DUMMY_DECOMPRESSOR =
        make_shared<DecompressorAnonymousInnerClass>();

DummyCompressingCodec::DecompressorAnonymousInnerClass::
    DecompressorAnonymousInnerClass()
{
}

void DummyCompressingCodec::DecompressorAnonymousInnerClass::decompress(
    shared_ptr<DataInput> in_, int originalLength, int offset, int length,
    shared_ptr<BytesRef> bytes) 
{
  assert(offset + length <= originalLength);
  if (bytes->bytes.size() < originalLength) {
    bytes->bytes = std::deque<char>(ArrayUtil::oversize(originalLength, 1));
  }
  in_->readBytes(bytes->bytes, 0, offset + length);
  bytes->offset = offset;
  bytes->length = length;
}

shared_ptr<Decompressor>
DummyCompressingCodec::DecompressorAnonymousInnerClass::clone()
{
  return shared_from_this();
}

const shared_ptr<org::apache::lucene::codecs::compressing::Compressor>
    DummyCompressingCodec::DUMMY_COMPRESSOR =
        make_shared<CompressorAnonymousInnerClass>();

DummyCompressingCodec::CompressorAnonymousInnerClass::
    CompressorAnonymousInnerClass()
{
}

void DummyCompressingCodec::CompressorAnonymousInnerClass::compress(
    std::deque<char> &bytes, int off, int len,
    shared_ptr<DataOutput> out) 
{
  out->writeBytes(bytes, off, len);
}

DummyCompressingCodec::CompressorAnonymousInnerClass::
    ~CompressorAnonymousInnerClass(){};

DummyCompressingCodec::DummyCompressingCodec(int chunkSize, int maxDocsPerChunk,
                                             bool withSegmentSuffix,
                                             int blockSize)
    : org::apache::lucene::codecs::compressing::CompressingCodec(
          L"DummyCompressingStoredFields",
          withSegmentSuffix ? L"DummyCompressingStoredFields" : L"", DUMMY,
          chunkSize, maxDocsPerChunk, blockSize)
{
}

DummyCompressingCodec::DummyCompressingCodec()
    : DummyCompressingCodec(1 << 14, 128, false, 1024)
{
}
} // namespace org::apache::lucene::codecs::compressing::dummy