using namespace std;

#include "AbstractTestLZ4CompressionMode.h"

namespace org::apache::lucene::codecs::compressing
{
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;

std::deque<char> AbstractTestLZ4CompressionMode::test(
    std::deque<char> &decompressed) 
{
  const std::deque<char> compressed =
      AbstractTestCompressionMode::test(decompressed);
  int off = 0;
  int decompressedOff = 0;
  for (;;) {
    constexpr int token = compressed[off++] & 0xFF;
    int literalLen = static_cast<int>(static_cast<unsigned int>(token) >> 4);
    if (literalLen == 0x0F) {
      while (compressed[off] == static_cast<char>(0xFF)) {
        literalLen += 0xFF;
        ++off;
      }
      literalLen += compressed[off++] & 0xFF;
    }
    // skip literals
    off += literalLen;
    decompressedOff += literalLen;

    // check that the stream ends with literals and that there are at least
    // 5 of them
    if (off == compressed.size()) {
      assertEquals(decompressed.size(), decompressedOff);
      assertTrue(L"lastLiterals=" + to_wstring(literalLen) + L", bytes=" +
                     decompressed.size(),
                 literalLen >= LZ4::LAST_LITERALS ||
                     literalLen == decompressed.size());
      break;
    }

    constexpr int matchDec =
        (compressed[off++] & 0xFF) | ((compressed[off++] & 0xFF) << 8);
    // check that match dec is not 0
    assertTrue(to_wstring(matchDec) + L" " + to_wstring(decompressedOff),
               matchDec > 0 && matchDec <= decompressedOff);

    int matchLen = token & 0x0F;
    if (matchLen == 0x0F) {
      while (compressed[off] == static_cast<char>(0xFF)) {
        matchLen += 0xFF;
        ++off;
      }
      matchLen += compressed[off++] & 0xFF;
    }
    matchLen += LZ4::MIN_MATCH;

    // if the match ends prematurely, the next sequence should not have
    // literals or this means we are wasting space
    if (decompressedOff + matchLen < decompressed.size() - LZ4::LAST_LITERALS) {
      constexpr bool moreCommonBytes =
          decompressed[decompressedOff + matchLen] ==
          decompressed[decompressedOff - matchDec + matchLen];
      constexpr bool nextSequenceHasLiterals = static_cast<static_cast<int>(
          static_cast<unsigned int>((compressed[off] & 0xFF)) >> 4) *>(!) = 0;
      assertTrue(!moreCommonBytes || !nextSequenceHasLiterals);
    }

    decompressedOff += matchLen;
  }
  assertEquals(decompressed.size(), decompressedOff);
  return compressed;
}

void AbstractTestLZ4CompressionMode::testShortLiteralsAndMatchs() throw(
    IOException)
{
  // literals and matchs lengths <= 15
  const std::deque<char> decompressed =
      (wstring(L"1234562345673456745678910123"))
          .getBytes(StandardCharsets::UTF_8);
  test(decompressed);
}

void AbstractTestLZ4CompressionMode::testLongMatchs() 
{
  // match length >= 20
  const std::deque<char> decompressed =
      std::deque<char>(RandomNumbers::randomIntBetween(random(), 300, 1024));
  for (int i = 0; i < decompressed.size(); ++i) {
    decompressed[i] = static_cast<char>(i);
  }
  test(decompressed);
}

void AbstractTestLZ4CompressionMode::testLongLiterals() 
{
  // long literals (length >= 16) which are not the last literals
  const std::deque<char> decompressed =
      randomArray(RandomNumbers::randomIntBetween(random(), 400, 1024), 256);
  constexpr int matchRef = random()->nextInt(30);
  constexpr int matchOff = RandomNumbers::randomIntBetween(
      random(), decompressed.size() - 40, decompressed.size() - 20);
  constexpr int matchLength = RandomNumbers::randomIntBetween(random(), 4, 10);
  System::arraycopy(decompressed, matchRef, decompressed, matchOff,
                    matchLength);
  test(decompressed);
}

void AbstractTestLZ4CompressionMode::testMatchRightBeforeLastLiterals() throw(
    IOException)
{
  test(std::deque<char>{1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 5});
}
} // namespace org::apache::lucene::codecs::compressing