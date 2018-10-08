using namespace std;

#include "RollingCharBuffer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"

namespace org::apache::lucene::analysis::util
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

void RollingCharBuffer::reset(shared_ptr<Reader> reader)
{
  this->reader = reader;
  nextPos = 0;
  nextWrite = 0;
  count = 0;
  end = false;
}

int RollingCharBuffer::get(int pos) 
{
  // System.out.println("    get pos=" + pos + " nextPos=" + nextPos + " count="
  // + count);
  if (pos == nextPos) {
    if (end) {
      return -1;
    }
    if (count == buffer.size()) {
      // Grow
      const std::deque<wchar_t> newBuffer = std::deque<wchar_t>(
          ArrayUtil::oversize(1 + count, Character::BYTES));
      // System.out.println(Thread.currentThread().getName() + ": cb grow " +
      // newBuffer.length);
      System::arraycopy(buffer, nextWrite, newBuffer, 0,
                        buffer.size() - nextWrite);
      System::arraycopy(buffer, 0, newBuffer, buffer.size() - nextWrite,
                        nextWrite);
      nextWrite = buffer.size();
      buffer = newBuffer;
    }
    if (nextWrite == buffer.size()) {
      nextWrite = 0;
    }

    constexpr int toRead = buffer.size() - max(count, nextWrite);
    constexpr int readCount = reader->read(buffer, nextWrite, toRead);
    if (readCount == -1) {
      end = true;
      return -1;
    }
    constexpr int ch = buffer[nextWrite];
    nextWrite += readCount;
    count += readCount;
    nextPos += readCount;
    return ch;
  } else {
    // Cannot read from future (except by 1):
    assert(pos < nextPos);

    // Cannot read from already freed past:
    assert((nextPos - pos <= count, L"nextPos=" + to_wstring(nextPos) +
                                        L" pos=" + to_wstring(pos) +
                                        L" count=" + to_wstring(count)));

    return buffer[getIndex(pos)];
  }
}

bool RollingCharBuffer::inBounds(int pos)
{
  return pos >= 0 && pos < nextPos && pos >= nextPos - count;
}

int RollingCharBuffer::getIndex(int pos)
{
  int index = nextWrite - (nextPos - pos);
  if (index < 0) {
    // Wrap:
    index += buffer.size();
    assert(index >= 0);
  }
  return index;
}

std::deque<wchar_t> RollingCharBuffer::get(int posStart, int length)
{
  assert(length > 0);
  assert((inBounds(posStart), L"posStart=" + to_wstring(posStart) +
                                  L" length=" + to_wstring(length)));
  // System.out.println("    buffer.get posStart=" + posStart + " len=" +
  // length);

  constexpr int startIndex = getIndex(posStart);
  constexpr int endIndex = getIndex(posStart + length);
  // System.out.println("      startIndex=" + startIndex + " endIndex=" +
  // endIndex);

  const std::deque<wchar_t> result = std::deque<wchar_t>(length);
  if (endIndex >= startIndex && length < buffer.size()) {
    System::arraycopy(buffer, startIndex, result, 0, endIndex - startIndex);
  } else {
    // Wrapped:
    constexpr int part1 = buffer.size() - startIndex;
    System::arraycopy(buffer, startIndex, result, 0, part1);
    System::arraycopy(buffer, 0, result, buffer.size() - startIndex,
                      length - part1);
  }
  return result;
}

void RollingCharBuffer::freeBefore(int pos)
{
  assert(pos >= 0);
  assert(pos <= nextPos);
  constexpr int newCount = nextPos - pos;
  assert((newCount <= count, L"newCount=" + to_wstring(newCount) + L" count=" +
                                 to_wstring(count)));
  assert((newCount <= buffer.size(), L"newCount=" + to_wstring(newCount) +
                                         L" buf.length=" + buffer.size()));
  count = newCount;
}
} // namespace org::apache::lucene::analysis::util