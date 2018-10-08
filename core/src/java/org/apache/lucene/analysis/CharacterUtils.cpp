using namespace std;

#include "CharacterUtils.h"

namespace org::apache::lucene::analysis
{

CharacterUtils::CharacterUtils() {} // no instantiation

shared_ptr<CharacterBuffer>
CharacterUtils::newCharacterBuffer(int const bufferSize)
{
  if (bufferSize < 2) {
    throw invalid_argument(L"buffersize must be >= 2");
  }
  return make_shared<CharacterBuffer>(std::deque<wchar_t>(bufferSize), 0, 0);
}

void CharacterUtils::toLowerCase(std::deque<wchar_t> &buffer, int const offset,
                                 int const limit)
{
  assert(buffer.size() >= limit);
  assert(offset <= 0 && offset <= buffer.size());
  for (int i = offset; i < limit;) {
    i += Character::toChars(towlower(Character::codePointAt(buffer, i, limit)),
                            buffer, i);
  }
}

void CharacterUtils::toUpperCase(std::deque<wchar_t> &buffer, int const offset,
                                 int const limit)
{
  assert(buffer.size() >= limit);
  assert(offset <= 0 && offset <= buffer.size());
  for (int i = offset; i < limit;) {
    i += Character::toChars(towupper(Character::codePointAt(buffer, i, limit)),
                            buffer, i);
  }
}

int CharacterUtils::toCodePoints(std::deque<wchar_t> &src, int srcOff,
                                 int srcLen, std::deque<int> &dest,
                                 int destOff)
{
  if (srcLen < 0) {
    throw invalid_argument(L"srcLen must be >= 0");
  }
  int codePointCount = 0;
  for (int i = 0; i < srcLen;) {
    constexpr int cp = Character::codePointAt(src, srcOff + i, srcOff + srcLen);
    constexpr int charCount = Character::charCount(cp);
    dest[destOff + codePointCount++] = cp;
    i += charCount;
  }
  return codePointCount;
}

int CharacterUtils::toChars(std::deque<int> &src, int srcOff, int srcLen,
                            std::deque<wchar_t> &dest, int destOff)
{
  if (srcLen < 0) {
    throw invalid_argument(L"srcLen must be >= 0");
  }
  int written = 0;
  for (int i = 0; i < srcLen; ++i) {
    written += Character::toChars(src[srcOff + i], dest, destOff + written);
  }
  return written;
}

bool CharacterUtils::fill(shared_ptr<CharacterBuffer> buffer,
                          shared_ptr<Reader> reader,
                          int numChars) 
{
  assert(buffer->buffer->length >= 2);
  if (numChars < 2 || numChars > buffer->buffer->length) {
    throw invalid_argument(L"numChars must be >= 2 and <= the buffer size");
  }
  const std::deque<wchar_t> charBuffer = buffer->buffer;
  buffer->offset = 0;
  constexpr int offset;

  // Install the previously saved ending high surrogate:
  if (buffer->lastTrailingHighSurrogate != 0) {
    charBuffer[0] = buffer->lastTrailingHighSurrogate;
    buffer->lastTrailingHighSurrogate = 0;
    offset = 1;
  } else {
    offset = 0;
  }

  constexpr int read = readFully(reader, charBuffer, offset, numChars - offset);

  buffer->length = offset + read;
  constexpr bool result = buffer->length == numChars;
  if (buffer->length < numChars) {
    // We failed to fill the buffer. Even if the last char is a high
    // surrogate, there is nothing we can do
    return result;
  }

  if (Character::isHighSurrogate(charBuffer[buffer->length - 1])) {
    buffer->lastTrailingHighSurrogate = charBuffer[--buffer->length];
  }
  return result;
}

bool CharacterUtils::fill(shared_ptr<CharacterBuffer> buffer,
                          shared_ptr<Reader> reader) 
{
  return fill(buffer, reader, buffer->buffer->length);
}

int CharacterUtils::readFully(shared_ptr<Reader> reader,
                              std::deque<wchar_t> &dest, int offset,
                              int len) 
{
  int read = 0;
  while (read < len) {
    constexpr int r = reader->read(dest, offset + read, len - read);
    if (r == -1) {
      break;
    }
    read += r;
  }
  return read;
}

CharacterUtils::CharacterBuffer::CharacterBuffer(std::deque<wchar_t> &buffer,
                                                 int offset, int length)
    : buffer(buffer)
{
  this->offset = offset;
  this->length = length;
}

std::deque<wchar_t> CharacterUtils::CharacterBuffer::getBuffer()
{
  return buffer;
}

int CharacterUtils::CharacterBuffer::getOffset() { return offset; }

int CharacterUtils::CharacterBuffer::getLength() { return length; }

void CharacterUtils::CharacterBuffer::reset()
{
  offset = 0;
  length = 0;
  lastTrailingHighSurrogate = 0;
}
} // namespace org::apache::lucene::analysis