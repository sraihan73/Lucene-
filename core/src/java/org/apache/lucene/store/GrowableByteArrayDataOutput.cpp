using namespace std;

#include "GrowableByteArrayDataOutput.h"

namespace org::apache::lucene::store
{
using DataOutput = org::apache::lucene::store::DataOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

GrowableByteArrayDataOutput::GrowableByteArrayDataOutput(int cp)
{
  this->bytes = std::deque<char>(ArrayUtil::oversize(cp, 1));
  this->length = 0;
}

void GrowableByteArrayDataOutput::writeByte(char b)
{
  if (length >= bytes.size()) {
    bytes = ArrayUtil::grow(bytes);
  }
  bytes[length++] = b;
}

void GrowableByteArrayDataOutput::writeBytes(std::deque<char> &b, int off,
                                             int len)
{
  constexpr int newLength = length + len;
  if (newLength > bytes.size()) {
    bytes = ArrayUtil::grow(bytes, newLength);
  }
  System::arraycopy(b, off, bytes, length, len);
  length = newLength;
}

void GrowableByteArrayDataOutput::writeString(const wstring &string) throw(
    IOException)
{
  int maxLen = UnicodeUtil::maxUTF8Length(string.length());
  if (maxLen <= MIN_UTF8_SIZE_TO_ENABLE_DOUBLE_PASS_ENCODING) {
    // string is small enough that we don't need to save memory by falling back
    // to double-pass approach this is just an optimized writeString() that
    // re-uses scratchBytes.
    if (scratchBytes.empty()) {
      scratchBytes =
          std::deque<char>(ArrayUtil::oversize(maxLen, Character::BYTES));
    } else {
      scratchBytes = ArrayUtil::grow(scratchBytes, maxLen);
    }
    int len =
        UnicodeUtil::UTF16toUTF8(string, 0, string.length(), scratchBytes);
    writeVInt(len);
    writeBytes(scratchBytes, len);
  } else {
    // use a double pass approach to avoid allocating a large intermediate
    // buffer for string encoding
    int numBytes =
        UnicodeUtil::calcUTF16toUTF8Length(string, 0, string.length());
    writeVInt(numBytes);
    bytes = ArrayUtil::grow(bytes, length + numBytes);
    length =
        UnicodeUtil::UTF16toUTF8(string, 0, string.length(), bytes, length);
  }
}

std::deque<char> GrowableByteArrayDataOutput::getBytes() { return bytes; }

int GrowableByteArrayDataOutput::getPosition() { return length; }

void GrowableByteArrayDataOutput::reset() { length = 0; }
} // namespace org::apache::lucene::store