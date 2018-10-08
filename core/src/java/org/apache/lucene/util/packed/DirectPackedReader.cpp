using namespace std;

#include "DirectPackedReader.h"

namespace org::apache::lucene::util::packed
{
using IndexInput = org::apache::lucene::store::IndexInput;

DirectPackedReader::DirectPackedReader(int bitsPerValue, int valueCount,
                                       shared_ptr<IndexInput> in_)
    : PackedInts::ReaderImpl(valueCount), in_(in_), bitsPerValue(bitsPerValue),
      startPointer(in_->getFilePointer())
{

  if (bitsPerValue == 64) {
    valueMask = -1LL;
  } else {
    valueMask = (1LL << bitsPerValue) - 1;
  }
}

int64_t DirectPackedReader::get(int index)
{
  constexpr int64_t majorBitPos =
      static_cast<int64_t>(index) * bitsPerValue;
  constexpr int64_t elementPos =
      static_cast<int64_t>(static_cast<uint64_t>(majorBitPos) >> 3);
  try {
    in_->seek(startPointer + elementPos);

    constexpr int bitPos = static_cast<int>(majorBitPos & 7);
    // round up bits to a multiple of 8 to find total bytes needed to read
    constexpr int roundedBits = ((bitPos + bitsPerValue + 7) & ~7);
    // the number of extra bits read at the end to shift out
    int shiftRightBits = roundedBits - bitPos - bitsPerValue;

    int64_t rawValue;
    switch (static_cast<int>(static_cast<unsigned int>(roundedBits) >> 3)) {
    case 1:
      rawValue = in_->readByte();
      break;
    case 2:
      rawValue = in_->readShort();
      break;
    case 3:
      rawValue = (static_cast<int64_t>(in_->readShort()) << 8) |
                 (in_->readByte() & 0xFFLL);
      break;
    case 4:
      rawValue = in_->readInt();
      break;
    case 5:
      rawValue = (static_cast<int64_t>(in_->readInt()) << 8) |
                 (in_->readByte() & 0xFFLL);
      break;
    case 6:
      rawValue = (static_cast<int64_t>(in_->readInt()) << 16) |
                 (in_->readShort() & 0xFFFFLL);
      break;
    case 7:
      rawValue = (static_cast<int64_t>(in_->readInt()) << 24) |
                 ((in_->readShort() & 0xFFFFLL) << 8) |
                 (in_->readByte() & 0xFFLL);
      break;
    case 8:
      rawValue = in_->readLong();
      break;
    case 9:
      // We must be very careful not to shift out relevant bits. So we account
      // for right shift we would normally do on return here, and reset it.
      rawValue = (in_->readLong() << (8 - shiftRightBits)) |
                 (static_cast<int64_t>(static_cast<uint64_t>(
                                             (in_->readByte() & 0xFFLL)) >>
                                         shiftRightBits));
      shiftRightBits = 0;
      break;
    default:
      throw make_shared<AssertionError>(L"bitsPerValue too large: " +
                                        to_wstring(bitsPerValue));
    }
    return (static_cast<int64_t>(static_cast<uint64_t>(rawValue) >>
                                   shiftRightBits)) &
           valueMask;

  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

int64_t DirectPackedReader::ramBytesUsed() { return 0; }
} // namespace org::apache::lucene::util::packed