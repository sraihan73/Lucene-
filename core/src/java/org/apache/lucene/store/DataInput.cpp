using namespace std;

#include "DataInput.h"

namespace org::apache::lucene::store
{
using BitUtil = org::apache::lucene::util::BitUtil;

void DataInput::readBytes(std::deque<char> &b, int offset, int len,
                          bool useBuffer) 
{
  // Default to ignoring useBuffer entirely
  readBytes(b, offset, len);
}

short DataInput::readShort() 
{
  return static_cast<short>(((readByte() & 0xFF) << 8) | (readByte() & 0xFF));
}

int DataInput::readInt() 
{
  return ((readByte() & 0xFF) << 24) | ((readByte() & 0xFF) << 16) |
         ((readByte() & 0xFF) << 8) | (readByte() & 0xFF);
}

int DataInput::readVInt() 
{
  /* This is the original code of this method,
   * but a Hotspot bug (see LUCENE-2975) corrupts the for-loop if
   * readByte() is inlined. So the loop was unwinded!
  byte b = readByte();
  int i = b & 0x7F;
  for (int shift = 7; (b & 0x80) != 0; shift += 7) {
    b = readByte();
    i |= (b & 0x7F) << shift;
  }
  return i;
  */
  char b = readByte();
  if (b >= 0) {
    return b;
  }
  int i = b & 0x7F;
  b = readByte();
  i |= (b & 0x7F) << 7;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  i |= (b & 0x7F) << 14;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  i |= (b & 0x7F) << 21;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  // Warning: the next ands use 0x0F / 0xF0 - beware copy/paste errors:
  i |= (b & 0x0F) << 28;
  if ((b & 0xF0) == 0) {
    return i;
  }
  // C++ TODO: The following line could not be converted:
  throw java.io.IOException(L"Invalid vInt detected (too many bits)");
}

int DataInput::readZInt() 
{
  return BitUtil::zigZagDecode(readVInt());
}

int64_t DataInput::readLong() 
{
  return ((static_cast<int64_t>(readInt())) << 32) |
         (readInt() & 0xFFFFFFFFLL);
}

int64_t DataInput::readVLong()  { return readVLong(false); }

int64_t DataInput::readVLong(bool allowNegative) 
{
  /* This is the original code of this method,
   * but a Hotspot bug (see LUCENE-2975) corrupts the for-loop if
   * readByte() is inlined. So the loop was unwinded!
  byte b = readByte();
  long i = b & 0x7F;
  for (int shift = 7; (b & 0x80) != 0; shift += 7) {
    b = readByte();
    i |= (b & 0x7FL) << shift;
  }
  return i;
  */
  char b = readByte();
  if (b >= 0) {
    return b;
  }
  int64_t i = b & 0x7FLL;
  b = readByte();
  i |= (b & 0x7FLL) << 7;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  i |= (b & 0x7FLL) << 14;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  i |= (b & 0x7FLL) << 21;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  i |= (b & 0x7FLL) << 28;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  i |= (b & 0x7FLL) << 35;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  i |= (b & 0x7FLL) << 42;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  i |= (b & 0x7FLL) << 49;
  if (b >= 0) {
    return i;
  }
  b = readByte();
  i |= (b & 0x7FLL) << 56;
  if (b >= 0) {
    return i;
  }
  if (allowNegative) {
    b = readByte();
    i |= (b & 0x7FLL) << 63;
    if (b == 0 || b == 1) {
      return i;
    }
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Invalid vLong detected (more than 64 bits)");
  } else {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Invalid vLong detected (negative values disallowed)");
  }
}

int64_t DataInput::readZLong() 
{
  return BitUtil::zigZagDecode(readVLong(true));
}

wstring DataInput::readString() 
{
  int length = readVInt();
  const std::deque<char> bytes = std::deque<char>(length);
  readBytes(bytes, 0, length);
  return wstring(bytes, 0, length, StandardCharsets::UTF_8);
}

shared_ptr<DataInput> DataInput::clone()
{
  try {
    return std::static_pointer_cast<DataInput>(__super::clone());
  } catch (const CloneNotSupportedException &e) {
    throw make_shared<Error>(L"This cannot happen: Failing to clone DataInput");
  }
}

unordered_map<wstring, wstring> DataInput::readMapOfStrings() 
{
  int count = readVInt();
  if (count == 0) {
    return Collections::emptyMap();
  } else if (count == 1) {
    return Collections::singletonMap(readString(), readString());
  } else {
    unordered_map<wstring, wstring> map_obj =
        count > 10 ? unordered_map<wstring, wstring>() : map_obj<>();
    for (int i = 0; i < count; i++) {
      const wstring key = readString();
      const wstring val = readString();
      map_obj.emplace(key, val);
    }
    return map_obj;
  }
}

shared_ptr<Set<wstring>> DataInput::readSetOfStrings() 
{
  int count = readVInt();
  if (count == 0) {
    return Collections::emptySet();
  } else if (count == 1) {
    return Collections::singleton(readString());
  } else {
    shared_ptr<Set<wstring>> set =
        count > 10 ? unordered_set<wstring>() : set<>();
    for (int i = 0; i < count; i++) {
      set->add(readString());
    }
    return Collections::unmodifiableSet(set);
  }
}

void DataInput::skipBytes(int64_t const numBytes) 
{
  if (numBytes < 0) {
    throw invalid_argument(L"numBytes must be >= 0, got " +
                           to_wstring(numBytes));
  }
  if (skipBuffer.empty()) {
    skipBuffer = std::deque<char>(SKIP_BUFFER_SIZE);
  }
  assert(skipBuffer.size() == SKIP_BUFFER_SIZE);
  for (int64_t skipped = 0; skipped < numBytes;) {
    constexpr int step =
        static_cast<int>(min(SKIP_BUFFER_SIZE, numBytes - skipped));
    readBytes(skipBuffer, 0, step, false);
    skipped += step;
  }
}
} // namespace org::apache::lucene::store