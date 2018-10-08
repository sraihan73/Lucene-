using namespace std;

#include "CompressionMode.h"
#include "../../index/CorruptIndexException.h"
#include "../../store/DataInput.h"
#include "../../store/DataOutput.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "LZ4.h"

namespace org::apache::lucene::codecs::compressing
{
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<CompressionMode> CompressionMode::FAST =
    make_shared<CompressionModeAnonymousInnerClass>();

CompressionMode::CompressionModeAnonymousInnerClass::
    CompressionModeAnonymousInnerClass()
{
}

shared_ptr<Compressor>
CompressionMode::CompressionModeAnonymousInnerClass::newCompressor()
{
  return make_shared<LZ4FastCompressor>();
}

shared_ptr<Decompressor>
CompressionMode::CompressionModeAnonymousInnerClass::newDecompressor()
{
  return LZ4_DECOMPRESSOR;
}

wstring CompressionMode::CompressionModeAnonymousInnerClass::toString()
{
  return L"FAST";
}

const shared_ptr<CompressionMode> CompressionMode::HIGH_COMPRESSION =
    make_shared<CompressionModeAnonymousInnerClass2>();

CompressionMode::CompressionModeAnonymousInnerClass2::
    CompressionModeAnonymousInnerClass2()
{
}

shared_ptr<Compressor>
CompressionMode::CompressionModeAnonymousInnerClass2::newCompressor()
{
  // notes:
  // 3 is the highest level that doesn't have lazy match evaluation
  // 6 is the default, higher than that is just a waste of cpu
  return make_shared<DeflateCompressor>(6);
}

shared_ptr<Decompressor>
CompressionMode::CompressionModeAnonymousInnerClass2::newDecompressor()
{
  return make_shared<DeflateDecompressor>();
}

wstring CompressionMode::CompressionModeAnonymousInnerClass2::toString()
{
  return L"HIGH_COMPRESSION";
}

const shared_ptr<CompressionMode> CompressionMode::FAST_DECOMPRESSION =
    make_shared<CompressionModeAnonymousInnerClass3>();

CompressionMode::CompressionModeAnonymousInnerClass3::
    CompressionModeAnonymousInnerClass3()
{
}

shared_ptr<Compressor>
CompressionMode::CompressionModeAnonymousInnerClass3::newCompressor()
{
  return make_shared<LZ4HighCompressor>();
}

shared_ptr<Decompressor>
CompressionMode::CompressionModeAnonymousInnerClass3::newDecompressor()
{
  return LZ4_DECOMPRESSOR;
}

wstring CompressionMode::CompressionModeAnonymousInnerClass3::toString()
{
  return L"FAST_DECOMPRESSION";
}

CompressionMode::CompressionMode() {}

const shared_ptr<Decompressor> CompressionMode::LZ4_DECOMPRESSOR =
    make_shared<DecompressorAnonymousInnerClass>();

CompressionMode::DecompressorAnonymousInnerClass::
    DecompressorAnonymousInnerClass()
{
}

void CompressionMode::DecompressorAnonymousInnerClass::decompress(
    shared_ptr<DataInput> in_, int originalLength, int offset, int length,
    shared_ptr<BytesRef> bytes) 
{
  assert(offset + length <= originalLength);
  // add 7 padding bytes, this is not necessary but can help decompression run
  // faster
  if (bytes->bytes.size() < originalLength + 7) {
    bytes->bytes =
        std::deque<char>(ArrayUtil::oversize(originalLength + 7, 1));
  }
  constexpr int decompressedLength =
      LZ4::decompress(in_, offset + length, bytes->bytes, 0);
  if (decompressedLength > originalLength) {
    throw make_shared<CorruptIndexException>(
        L"Corrupted: lengths mismatch: " + to_wstring(decompressedLength) +
            L" > " + to_wstring(originalLength),
        in_);
  }
  bytes->offset = offset;
  bytes->length = length;
}

shared_ptr<Decompressor>
CompressionMode::DecompressorAnonymousInnerClass::clone()
{
  return shared_from_this();
}

CompressionMode::LZ4FastCompressor::LZ4FastCompressor()
    : ht(make_shared<LZ4::HashTable>())
{
}

void CompressionMode::LZ4FastCompressor::compress(
    std::deque<char> &bytes, int off, int len,
    shared_ptr<DataOutput> out) 
{
  LZ4::compress(bytes, off, len, out, ht);
}

CompressionMode::LZ4FastCompressor::~LZ4FastCompressor()
{
  // no-op
}

CompressionMode::LZ4HighCompressor::LZ4HighCompressor()
    : ht(make_shared<LZ4::HCHashTable>())
{
}

void CompressionMode::LZ4HighCompressor::compress(
    std::deque<char> &bytes, int off, int len,
    shared_ptr<DataOutput> out) 
{
  LZ4::compressHC(bytes, off, len, out, ht);
}

CompressionMode::LZ4HighCompressor::~LZ4HighCompressor()
{
  // no-op
}

CompressionMode::DeflateDecompressor::DeflateDecompressor()
{
  compressed = std::deque<char>(0);
}

void CompressionMode::DeflateDecompressor::decompress(
    shared_ptr<DataInput> in_, int originalLength, int offset, int length,
    shared_ptr<BytesRef> bytes) 
{
  assert(offset + length <= originalLength);
  if (length == 0) {
    bytes->length = 0;
    return;
  }
  constexpr int compressedLength = in_->readVInt();
  // pad with extra "dummy byte": see javadocs for using Inflater(true)
  // we do it for compliance, but it's unnecessary for years in zlib.
  constexpr int paddedLength = compressedLength + 1;
  compressed = ArrayUtil::grow(compressed, paddedLength);
  in_->readBytes(compressed, 0, compressedLength);
  compressed[compressedLength] = 0; // explicitly set dummy byte to 0

  shared_ptr<Inflater> *const decompressor = make_shared<Inflater>(true);
  try {
    // extra "dummy byte"
    decompressor->setInput(compressed, 0, paddedLength);

    bytes->offset = bytes->length = 0;
    bytes->bytes = ArrayUtil::grow(bytes->bytes, originalLength);
    try {
      bytes->length =
          decompressor->inflate(bytes->bytes, bytes->length, originalLength);
    } catch (const DataFormatException &e) {
      throw make_shared<IOException>(e);
    }
    if (!decompressor->finished()) {
      throw make_shared<CorruptIndexException>(
          L"Invalid decoder state: needsInput=" + decompressor->needsInput() +
              L", needsDict=" + decompressor->needsDictionary(),
          in_);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    decompressor->end();
  }
  if (bytes->length != originalLength) {
    throw make_shared<CorruptIndexException>(
        L"Lengths mismatch: " + to_wstring(bytes->length) + L" != " +
            to_wstring(originalLength),
        in_);
  }
  bytes->offset = offset;
  bytes->length = length;
}

shared_ptr<Decompressor> CompressionMode::DeflateDecompressor::clone()
{
  return make_shared<DeflateDecompressor>();
}

CompressionMode::DeflateCompressor::DeflateCompressor(int level)
    : compressor(make_shared<Deflater>(level, true))
{
  compressed = std::deque<char>(64);
}

void CompressionMode::DeflateCompressor::compress(
    std::deque<char> &bytes, int off, int len,
    shared_ptr<DataOutput> out) 
{
  compressor->reset();
  compressor->setInput(bytes, off, len);
  compressor->finish();

  if (compressor->needsInput()) {
    // no output
    assert((len == 0, len));
    out->writeVInt(0);
    return;
  }

  int totalCount = 0;
  for (;;) {
    constexpr int count = compressor->deflate(compressed, totalCount,
                                              compressed.size() - totalCount);
    totalCount += count;
    assert(totalCount <= compressed.size());
    if (compressor->finished()) {
      break;
    } else {
      compressed = ArrayUtil::grow(compressed);
    }
  }

  out->writeVInt(totalCount);
  out->writeBytes(compressed, totalCount);
}

CompressionMode::DeflateCompressor::~DeflateCompressor()
{
  if (closed == false) {
    compressor->end();
    closed = true;
  }
}
} // namespace org::apache::lucene::codecs::compressing