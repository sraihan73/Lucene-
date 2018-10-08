using namespace std;

#include "DirectReader.h"

namespace org::apache::lucene::util::packed
{
using RandomAccessInput = org::apache::lucene::store::RandomAccessInput;
using LongValues = org::apache::lucene::util::LongValues;

shared_ptr<LongValues>
DirectReader::getInstance(shared_ptr<RandomAccessInput> slice, int bitsPerValue)
{
  return getInstance(slice, bitsPerValue, 0);
}

shared_ptr<LongValues>
DirectReader::getInstance(shared_ptr<RandomAccessInput> slice, int bitsPerValue,
                          int64_t offset)
{
  switch (bitsPerValue) {
  case 1:
    return make_shared<DirectPackedReader1>(slice, offset);
  case 2:
    return make_shared<DirectPackedReader2>(slice, offset);
  case 4:
    return make_shared<DirectPackedReader4>(slice, offset);
  case 8:
    return make_shared<DirectPackedReader8>(slice, offset);
  case 12:
    return make_shared<DirectPackedReader12>(slice, offset);
  case 16:
    return make_shared<DirectPackedReader16>(slice, offset);
  case 20:
    return make_shared<DirectPackedReader20>(slice, offset);
  case 24:
    return make_shared<DirectPackedReader24>(slice, offset);
  case 28:
    return make_shared<DirectPackedReader28>(slice, offset);
  case 32:
    return make_shared<DirectPackedReader32>(slice, offset);
  case 40:
    return make_shared<DirectPackedReader40>(slice, offset);
  case 48:
    return make_shared<DirectPackedReader48>(slice, offset);
  case 56:
    return make_shared<DirectPackedReader56>(slice, offset);
  case 64:
    return make_shared<DirectPackedReader64>(slice, offset);
  default:
    throw invalid_argument(L"unsupported bitsPerValue: " +
                           to_wstring(bitsPerValue));
  }
}

DirectReader::DirectPackedReader1::DirectPackedReader1(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader1::get(int64_t index)
{
  try {
    int shift = 7 - static_cast<int>(index & 7);
    return (static_cast<int>(
               static_cast<unsigned int>(in_->readByte(
                   offset +
                   (static_cast<int64_t>(
                       static_cast<uint64_t>(index) >> 3)))) >>
               shift)) &
           0x1;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader2::DirectPackedReader2(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader2::get(int64_t index)
{
  try {
    int shift = (3 - static_cast<int>(index & 3)) << 1;
    return (static_cast<int>(
               static_cast<unsigned int>(in_->readByte(
                   offset +
                   (static_cast<int64_t>(
                       static_cast<uint64_t>(index) >> 2)))) >>
               shift)) &
           0x3;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader4::DirectPackedReader4(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader4::get(int64_t index)
{
  try {
    int shift = static_cast<int>((index + 1) & 1) << 2;
    return (static_cast<int>(
               static_cast<unsigned int>(in_->readByte(
                   offset +
                   (static_cast<int64_t>(
                       static_cast<uint64_t>(index) >> 1)))) >>
               shift)) &
           0xF;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader8::DirectPackedReader8(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader8::get(int64_t index)
{
  try {
    return in_->readByte(offset + index) & 0xFF;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader12::DirectPackedReader12(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader12::get(int64_t index)
{
  try {
    int64_t offset = static_cast<int64_t>(
        static_cast<uint64_t>((index * 12)) >> 3);
    int shift = static_cast<int>((index + 1) & 1) << 2;
    return (static_cast<short>(static_cast<unsigned short>(
                                   in_->readShort(this->offset + offset)) >>
                               shift)) &
           0xFFF;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader16::DirectPackedReader16(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader16::get(int64_t index)
{
  try {
    return in_->readShort(offset + (index << 1)) & 0xFFFF;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader20::DirectPackedReader20(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader20::get(int64_t index)
{
  try {
    int64_t offset = static_cast<int64_t>(
        static_cast<uint64_t>((index * 20)) >> 3);
    // TODO: clean this up...
    int v = static_cast<int>(
        static_cast<unsigned int>(in_->readInt(this->offset + offset)) >> 8);
    int shift = static_cast<int>((index + 1) & 1) << 2;
    return (static_cast<int>(static_cast<unsigned int>(v) >> shift)) & 0xFFFFF;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader24::DirectPackedReader24(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader24::get(int64_t index)
{
  try {
    return static_cast<int>(
        static_cast<unsigned int>(in_->readInt(offset + index * 3)) >> 8);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader28::DirectPackedReader28(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader28::get(int64_t index)
{
  try {
    int64_t offset = static_cast<int64_t>(
        static_cast<uint64_t>((index * 28)) >> 3);
    int shift = static_cast<int>((index + 1) & 1) << 2;
    return (static_cast<int>(
               static_cast<unsigned int>(in_->readInt(this->offset + offset)) >>
               shift)) &
           0xFFFFFFFLL;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader32::DirectPackedReader32(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader32::get(int64_t index)
{
  try {
    return in_->readInt(this->offset + (index << 2)) & 0xFFFFFFFFLL;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader40::DirectPackedReader40(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader40::get(int64_t index)
{
  try {
    return static_cast<int64_t>(static_cast<uint64_t>(in_->readLong(
                                      this->offset + index * 5)) >>
                                  24);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader48::DirectPackedReader48(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader48::get(int64_t index)
{
  try {
    return static_cast<int64_t>(static_cast<uint64_t>(in_->readLong(
                                      this->offset + index * 6)) >>
                                  16);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader56::DirectPackedReader56(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader56::get(int64_t index)
{
  try {
    return static_cast<int64_t>(static_cast<uint64_t>(in_->readLong(
                                      this->offset + index * 7)) >>
                                  8);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

DirectReader::DirectPackedReader64::DirectPackedReader64(
    shared_ptr<RandomAccessInput> in_, int64_t offset)
    : in_(in_), offset(offset)
{
}

int64_t DirectReader::DirectPackedReader64::get(int64_t index)
{
  try {
    return in_->readLong(offset + (index << 3));
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::util::packed