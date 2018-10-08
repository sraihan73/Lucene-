using namespace std;

#include "DataOutput.h"

namespace org::apache::lucene::store
{
using BitUtil = org::apache::lucene::util::BitUtil;
using BytesRef = org::apache::lucene::util::BytesRef;

void DataOutput::writeBytes(std::deque<char> &b, int length) 
{
  writeBytes(b, 0, length);
}

void DataOutput::writeInt(int i) 
{
  writeByte(static_cast<char>(i >> 24));
  writeByte(static_cast<char>(i >> 16));
  writeByte(static_cast<char>(i >> 8));
  writeByte(static_cast<char>(i));
}

void DataOutput::writeShort(short i) 
{
  writeByte(static_cast<char>(i >> 8));
  writeByte(static_cast<char>(i));
}

void DataOutput::writeVInt(int i) 
{
  while ((i & ~0x7F) != 0) {
    writeByte(static_cast<char>((i & 0x7F) | 0x80));
    i = static_cast<int>(static_cast<unsigned int>(i) >> 7);
  }
  writeByte(static_cast<char>(i));
}

void DataOutput::writeZInt(int i) 
{
  writeVInt(BitUtil::zigZagEncode(i));
}

void DataOutput::writeLong(int64_t i) 
{
  writeInt(static_cast<int>(i >> 32));
  writeInt(static_cast<int>(i));
}

void DataOutput::writeVLong(int64_t i) 
{
  if (i < 0) {
    throw invalid_argument(L"cannot write negative vLong (got: " +
                           to_wstring(i) + L")");
  }
  writeSignedVLong(i);
}

void DataOutput::writeSignedVLong(int64_t i) 
{
  while ((i & ~0x7FLL) != 0LL) {
    writeByte(static_cast<char>((i & 0x7FLL) | 0x80LL));
    i = static_cast<int64_t>(static_cast<uint64_t>(i) >> 7);
  }
  writeByte(static_cast<char>(i));
}

void DataOutput::writeZLong(int64_t i) 
{
  writeSignedVLong(BitUtil::zigZagEncode(i));
}

void DataOutput::writeString(const wstring &s) 
{
  shared_ptr<BytesRef> *const utf8Result = make_shared<BytesRef>(s);
  writeVInt(utf8Result->length);
  writeBytes(utf8Result->bytes, utf8Result->offset, utf8Result->length);
}

int DataOutput::COPY_BUFFER_SIZE = 16384;

void DataOutput::copyBytes(shared_ptr<DataInput> input,
                           int64_t numBytes) 
{
  assert((numBytes >= 0, L"numBytes=" + to_wstring(numBytes)));
  int64_t left = numBytes;
  if (copyBuffer.empty()) {
    copyBuffer = std::deque<char>(COPY_BUFFER_SIZE);
  }
  while (left > 0) {
    constexpr int toCopy;
    if (left > COPY_BUFFER_SIZE) {
      toCopy = COPY_BUFFER_SIZE;
    } else {
      toCopy = static_cast<int>(left);
    }
    input->readBytes(copyBuffer, 0, toCopy);
    writeBytes(copyBuffer, 0, toCopy);
    left -= toCopy;
  }
}

void DataOutput::writeMapOfStrings(unordered_map<wstring, wstring> &map_obj) throw(
    IOException)
{
  writeVInt(map_obj.size());
  for (auto entry : map_obj) {
    writeString(entry.first);
    writeString(entry.second);
  }
}

void DataOutput::writeSetOfStrings(shared_ptr<Set<wstring>> set) throw(
    IOException)
{
  writeVInt(set->size());
  for (auto value : set) {
    writeString(value);
  }
}
} // namespace org::apache::lucene::store