using namespace std;

#include "ForUtil.h"
#include "../../store/DataInput.h"
#include "../../store/DataOutput.h"
#include "../../store/IndexInput.h"
#include "../../store/IndexOutput.h"

namespace org::apache::lucene::codecs::lucene50
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Decoder = org::apache::lucene::util::packed::PackedInts::Decoder;
using FormatAndBits =
    org::apache::lucene::util::packed::PackedInts::FormatAndBits;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
//    import static
//    org.apache.lucene.codecs.lucene50.Lucene50PostingsFormat.BLOCK_SIZE;

ForUtil::StaticConstructor::StaticConstructor()
{
  int maxDataSize = 0;
  for (int version = PackedInts::VERSION_START;
       version <= PackedInts::VERSION_CURRENT; version++) {
    for (PackedInts::Format format : PackedInts::Format::values()) {
      for (int bpv = 1; bpv <= 32; ++bpv) {
        if (!format.isSupported(bpv)) {
          continue;
        }
        shared_ptr<Decoder> *const decoder =
            PackedInts::getDecoder(format, version, bpv);
        constexpr int iterations = computeIterations(decoder);
        maxDataSize = max(maxDataSize, iterations * decoder->byteValueCount());
      }
    }
  }
  MAX_DATA_SIZE = maxDataSize;
}

ForUtil::StaticConstructor ForUtil::staticConstructor;

int ForUtil::computeIterations(shared_ptr<Decoder> decoder)
{
  return static_cast<int>(
      ceil(static_cast<float>(BLOCK_SIZE) / decoder->byteValueCount()));
}

int ForUtil::encodedSize(PackedInts::Format format, int packedIntsVersion,
                         int bitsPerValue)
{
  constexpr int64_t byteCount =
      format.byteCount(packedIntsVersion, BLOCK_SIZE, bitsPerValue);
  assert((byteCount >= 0 && byteCount <= std,
          : numeric_limits<int>::max()
          : byteCount));
  return static_cast<int>(byteCount);
}

ForUtil::ForUtil(float acceptableOverheadRatio,
                 shared_ptr<DataOutput> out) 
    : encodedSizes(std::deque<int>(33)),
      encoders(std::deque<std::shared_ptr<PackedInts::Encoder>>(33)),
      decoders(std::deque<std::shared_ptr<PackedInts::Decoder>>(33)),
      iterations(std::deque<int>(33))
{
  out->writeVInt(PackedInts::VERSION_CURRENT);

  for (int bpv = 1; bpv <= 32; ++bpv) {
    shared_ptr<FormatAndBits> *const formatAndBits =
        PackedInts::fastestFormatAndBits(BLOCK_SIZE, bpv,
                                         acceptableOverheadRatio);
    assert(formatAndBits->format.isSupported(formatAndBits->bitsPerValue));
    assert(formatAndBits->bitsPerValue <= 32);
    encodedSizes[bpv] =
        encodedSize(formatAndBits->format, PackedInts::VERSION_CURRENT,
                    formatAndBits->bitsPerValue);
    encoders[bpv] = PackedInts::getEncoder(formatAndBits->format,
                                           PackedInts::VERSION_CURRENT,
                                           formatAndBits->bitsPerValue);
    decoders[bpv] = PackedInts::getDecoder(formatAndBits->format,
                                           PackedInts::VERSION_CURRENT,
                                           formatAndBits->bitsPerValue);
    iterations[bpv] = computeIterations(decoders[bpv]);

    out->writeVInt(formatAndBits->format.getId() << 5 |
                   (formatAndBits->bitsPerValue - 1));
  }
}

ForUtil::ForUtil(shared_ptr<DataInput> in_) 
    : encodedSizes(std::deque<int>(33)),
      encoders(std::deque<std::shared_ptr<PackedInts::Encoder>>(33)),
      decoders(std::deque<std::shared_ptr<PackedInts::Decoder>>(33)),
      iterations(std::deque<int>(33))
{
  int packedIntsVersion = in_->readVInt();
  PackedInts::checkVersion(packedIntsVersion);

  for (int bpv = 1; bpv <= 32; ++bpv) {
    constexpr int code = in_->readVInt();
    constexpr int formatId =
        static_cast<int>(static_cast<unsigned int>(code) >> 5);
    constexpr int bitsPerValue = (code & 31) + 1;

    constexpr PackedInts::Format format = PackedInts::Format::byId(formatId);
    assert(format.isSupported(bitsPerValue));
    encodedSizes[bpv] = encodedSize(format, packedIntsVersion, bitsPerValue);
    encoders[bpv] =
        PackedInts::getEncoder(format, packedIntsVersion, bitsPerValue);
    decoders[bpv] =
        PackedInts::getDecoder(format, packedIntsVersion, bitsPerValue);
    iterations[bpv] = computeIterations(decoders[bpv]);
  }
}

void ForUtil::writeBlock(std::deque<int> &data, std::deque<char> &encoded,
                         shared_ptr<IndexOutput> out) 
{
  if (isAllEqual(data)) {
    out->writeByte(static_cast<char>(ALL_VALUES_EQUAL));
    out->writeVInt(data[0]);
    return;
  }

  constexpr int numBits = bitsRequired(data);
  assert((numBits > 0 && numBits <= 32, numBits));
  shared_ptr<PackedInts::Encoder> *const encoder = encoders[numBits];
  constexpr int iters = iterations[numBits];
  assert(iters * encoder->byteValueCount() >= BLOCK_SIZE);
  constexpr int encodedSize = encodedSizes[numBits];
  assert(iters * encoder->byteBlockCount() >= encodedSize);

  out->writeByte(static_cast<char>(numBits));

  encoder->encode(data, 0, encoded, 0, iters);
  out->writeBytes(encoded, encodedSize);
}

void ForUtil::readBlock(shared_ptr<IndexInput> in_, std::deque<char> &encoded,
                        std::deque<int> &decoded) 
{
  constexpr int numBits = in_->readByte();
  assert((numBits <= 32, numBits));

  if (numBits == ALL_VALUES_EQUAL) {
    constexpr int value = in_->readVInt();
    Arrays::fill(decoded, 0, BLOCK_SIZE, value);
    return;
  }

  constexpr int encodedSize = encodedSizes[numBits];
  in_->readBytes(encoded, 0, encodedSize);

  shared_ptr<Decoder> *const decoder = decoders[numBits];
  constexpr int iters = iterations[numBits];
  assert(iters * decoder->byteValueCount() >= BLOCK_SIZE);

  decoder->decode(encoded, 0, decoded, 0, iters);
}

void ForUtil::skipBlock(shared_ptr<IndexInput> in_) 
{
  constexpr int numBits = in_->readByte();
  if (numBits == ALL_VALUES_EQUAL) {
    in_->readVInt();
    return;
  }
  assert((numBits > 0 && numBits <= 32, numBits));
  constexpr int encodedSize = encodedSizes[numBits];
  in_->seek(in_->getFilePointer() + encodedSize);
}

bool ForUtil::isAllEqual(std::deque<int> &data)
{
  constexpr int v = data[0];
  for (int i = 1; i < BLOCK_SIZE; ++i) {
    if (data[i] != v) {
      return false;
    }
  }
  return true;
}

int ForUtil::bitsRequired(std::deque<int> &data)
{
  int64_t or = 0;
  for (int i = 0; i < BLOCK_SIZE; ++i) {
    assert(data[i] >= 0);
    or |= data[i];
  }
  return PackedInts::bitsRequired(or);
}
} // namespace org::apache::lucene::codecs::lucene50