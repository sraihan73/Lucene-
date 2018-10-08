using namespace std;

#include "AbstractTestCompressionMode.h"

namespace org::apache::lucene::codecs::compressing
{
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;

std::deque<char> AbstractTestCompressionMode::randomArray()
{
  constexpr int max =
      random()->nextBoolean() ? random()->nextInt(4) : random()->nextInt(255);
  constexpr int length = random()->nextBoolean()
                             ? random()->nextInt(20)
                             : random()->nextInt(192 * 1024);
  return randomArray(length, max);
}

std::deque<char> AbstractTestCompressionMode::randomArray(int length, int max)
{
  const std::deque<char> arr = std::deque<char>(length);
  for (int i = 0; i < arr.size(); ++i) {
    arr[i] =
        static_cast<char>(RandomNumbers::randomIntBetween(random(), 0, max));
  }
  return arr;
}

std::deque<char>
AbstractTestCompressionMode::compress(std::deque<char> &decompressed, int off,
                                      int len) 
{
  shared_ptr<Compressor> compressor = mode->newCompressor();
  return compress(compressor, decompressed, off, len);
}

std::deque<char>
AbstractTestCompressionMode::compress(shared_ptr<Compressor> compressor,
                                      std::deque<char> &decompressed, int off,
                                      int len) 
{
  std::deque<char> compressed(len * 2 + 16); // should be enough
  shared_ptr<ByteArrayDataOutput> out =
      make_shared<ByteArrayDataOutput>(compressed);
  compressor->compress(decompressed, off, len, out);
  constexpr int compressedLen = out->getPosition();
  return Arrays::copyOf(compressed, compressedLen);
}

std::deque<char>
AbstractTestCompressionMode::decompress(std::deque<char> &compressed,
                                        int originalLength) 
{
  shared_ptr<Decompressor> decompressor = mode->newDecompressor();
  return decompress(decompressor, compressed, originalLength);
}

std::deque<char>
AbstractTestCompressionMode::decompress(shared_ptr<Decompressor> decompressor,
                                        std::deque<char> &compressed,
                                        int originalLength) 
{
  shared_ptr<BytesRef> *const bytes = make_shared<BytesRef>();
  decompressor->decompress(make_shared<ByteArrayDataInput>(compressed),
                           originalLength, 0, originalLength, bytes);
  return Arrays::copyOfRange(bytes->bytes, bytes->offset,
                             bytes->offset + bytes->length);
}

std::deque<char>
AbstractTestCompressionMode::decompress(std::deque<char> &compressed,
                                        int originalLength, int offset,
                                        int length) 
{
  shared_ptr<Decompressor> decompressor = mode->newDecompressor();
  shared_ptr<BytesRef> *const bytes = make_shared<BytesRef>();
  decompressor->decompress(make_shared<ByteArrayDataInput>(compressed),
                           originalLength, offset, length, bytes);
  return Arrays::copyOfRange(bytes->bytes, bytes->offset,
                             bytes->offset + bytes->length);
}

void AbstractTestCompressionMode::testDecompress() 
{
  constexpr int iterations = atLeast(10);
  for (int i = 0; i < iterations; ++i) {
    const std::deque<char> decompressed = randomArray();
    constexpr int off =
        random()->nextBoolean()
            ? 0
            : TestUtil::nextInt(random(), 0, decompressed.size());
    constexpr int len =
        random()->nextBoolean()
            ? decompressed.size() - off
            : TestUtil::nextInt(random(), 0, decompressed.size() - off);
    const std::deque<char> compressed = compress(decompressed, off, len);
    const std::deque<char> restored = decompress(compressed, len);
    assertArrayEquals(Arrays::copyOfRange(decompressed, off, off + len),
                      restored);
  }
}

void AbstractTestCompressionMode::testPartialDecompress() 
{
  constexpr int iterations = atLeast(10);
  for (int i = 0; i < iterations; ++i) {
    const std::deque<char> decompressed = randomArray();
    const std::deque<char> compressed =
        compress(decompressed, 0, decompressed.size());
    constexpr int offset, length;
    if (decompressed.empty()) {
      offset = length = 0;
    } else {
      offset = random()->nextInt(decompressed.size());
      length = random()->nextInt(decompressed.size() - offset);
    }
    const std::deque<char> restored =
        decompress(compressed, decompressed.size(), offset, length);
    assertArrayEquals(
        Arrays::copyOfRange(decompressed, offset, offset + length), restored);
  }
}

std::deque<char> AbstractTestCompressionMode::test(
    std::deque<char> &decompressed) 
{
  return test(decompressed, 0, decompressed.size());
}

std::deque<char>
AbstractTestCompressionMode::test(std::deque<char> &decompressed, int off,
                                  int len) 
{
  const std::deque<char> compressed = compress(decompressed, off, len);
  const std::deque<char> restored = decompress(compressed, len);
  TestUtil::assertEquals(len, restored.size());
  return compressed;
}

void AbstractTestCompressionMode::testEmptySequence() 
{
  test(std::deque<char>(0));
}

void AbstractTestCompressionMode::testShortSequence() 
{
  test(std::deque<char>{static_cast<char>(random()->nextInt(256))});
}

void AbstractTestCompressionMode::testIncompressible() 
{
  const std::deque<char> decompressed =
      std::deque<char>(RandomNumbers::randomIntBetween(random(), 20, 256));
  for (int i = 0; i < decompressed.size(); ++i) {
    decompressed[i] = static_cast<char>(i);
  }
  test(decompressed);
}

void AbstractTestCompressionMode::testConstant() 
{
  const std::deque<char> decompressed =
      std::deque<char>(TestUtil::nextInt(random(), 1, 10000));
  Arrays::fill(decompressed, static_cast<char>(random()->nextInt()));
  test(decompressed);
}

void AbstractTestCompressionMode::testLUCENE5201() 
{
  std::deque<char> data = {
      14,   72,   14,   85,   3,    72,   14,   85,   3,    72,   14,   72,
      14,   72,   14,   85,   3,    72,   14,   72,   14,   72,   14,   72,
      14,   72,   14,   72,   14,   85,   3,    72,   14,   85,   3,    72,
      14,   85,   3,    72,   14,   85,   3,    72,   14,   85,   3,    72,
      14,   85,   3,    72,   14,   50,   64,   0,    46,   -1,   0,    0,
      0,    29,   3,    85,   8,    -113, 0,    68,   -97,  3,    0,    2,
      3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,
      68,   -113, 0,    2,    3,    85,   8,    -113, 0,    68,   -97,  3,
      0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,
      6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113,
      0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,
      6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113,
      0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,
      6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113,
      0,    2,    3,    -97,  6,    0,    68,   -113, 0,    50,   64,   0,
      47,   -105, 0,    0,    0,    30,   3,    -97,  6,    0,    68,   -113,
      0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    85,
      8,    -113, 0,    68,   -97,  3,    0,    2,    3,    85,   8,    -113,
      0,    68,   -97,  3,    0,    2,    3,    85,   8,    -113, 0,    68,
      -97,  3,    0,    2,    -97,  6,    0,    2,    3,    85,   8,    -113,
      0,    68,   -97,  3,    0,    2,    3,    -97,  6,    0,    68,   -113,
      0,    2,    3,    -97,  6,    0,    68,   -113, 0,    120,  64,   0,
      48,   4,    0,    0,    0,    31,   34,   72,   29,   72,   37,   72,
      35,   72,   45,   72,   23,   72,   46,   72,   20,   72,   40,   72,
      33,   72,   25,   72,   39,   72,   38,   72,   26,   72,   28,   72,
      42,   72,   24,   72,   27,   72,   36,   72,   41,   72,   32,   72,
      18,   72,   30,   72,   22,   72,   31,   72,   43,   72,   19,   72,
      34,   72,   29,   72,   37,   72,   35,   72,   45,   72,   23,   72,
      46,   72,   20,   72,   40,   72,   33,   72,   25,   72,   39,   72,
      38,   72,   26,   72,   28,   72,   42,   72,   24,   72,   27,   72,
      36,   72,   41,   72,   32,   72,   18,   72,   30,   72,   22,   72,
      31,   72,   43,   72,   19,   72,   34,   72,   29,   72,   37,   72,
      35,   72,   45,   72,   23,   72,   46,   72,   20,   72,   40,   72,
      33,   72,   25,   72,   39,   72,   38,   72,   26,   72,   28,   72,
      42,   72,   24,   72,   27,   72,   36,   72,   41,   72,   32,   72,
      18,   16,   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      0,    0,    0,    0,    0,    39,   24,   32,   34,   124,  0,    120,
      64,   0,    48,   80,   0,    0,    0,    31,   30,   72,   22,   72,
      31,   72,   43,   72,   19,   72,   34,   72,   29,   72,   37,   72,
      35,   72,   45,   72,   23,   72,   46,   72,   20,   72,   40,   72,
      33,   72,   25,   72,   39,   72,   38,   72,   26,   72,   28,   72,
      42,   72,   24,   72,   27,   72,   36,   72,   41,   72,   32,   72,
      18,   72,   30,   72,   22,   72,   31,   72,   43,   72,   19,   72,
      34,   72,   29,   72,   37,   72,   35,   72,   45,   72,   23,   72,
      46,   72,   20,   72,   40,   72,   33,   72,   25,   72,   39,   72,
      38,   72,   26,   72,   28,   72,   42,   72,   24,   72,   27,   72,
      36,   72,   41,   72,   32,   72,   18,   72,   30,   72,   22,   72,
      31,   72,   43,   72,   19,   72,   34,   72,   29,   72,   37,   72,
      35,   72,   45,   72,   23,   72,   46,   72,   20,   72,   40,   72,
      33,   72,   25,   72,   39,   72,   38,   72,   26,   72,   28,   72,
      42,   72,   24,   72,   27,   72,   36,   72,   41,   72,   32,   72,
      18,   72,   30,   72,   22,   72,   31,   72,   43,   72,   19,   72,
      34,   72,   29,   72,   37,   72,   35,   72,   45,   72,   23,   72,
      46,   72,   20,   72,   40,   72,   33,   72,   25,   72,   39,   72,
      38,   72,   26,   72,   28,   72,   42,   72,   24,   72,   27,   72,
      36,   72,   41,   72,   32,   72,   18,   72,   30,   72,   22,   72,
      31,   72,   43,   72,   19,   72,   34,   72,   29,   72,   37,   72,
      35,   72,   45,   72,   23,   72,   46,   72,   20,   72,   40,   72,
      33,   72,   25,   72,   39,   72,   38,   72,   26,   72,   28,   72,
      42,   72,   24,   72,   27,   72,   36,   72,   41,   72,   32,   72,
      18,   72,   30,   72,   22,   72,   31,   72,   43,   72,   19,   72,
      34,   72,   29,   72,   37,   72,   35,   72,   45,   72,   23,   72,
      46,   72,   20,   72,   40,   72,   33,   72,   25,   72,   39,   72,
      38,   72,   26,   72,   28,   72,   42,   72,   24,   72,   27,   72,
      36,   72,   41,   72,   32,   72,   18,   72,   30,   72,   22,   72,
      31,   72,   43,   72,   19,   72,   34,   72,   29,   72,   37,   72,
      35,   72,   45,   72,   23,   72,   46,   72,   20,   72,   40,   72,
      33,   72,   25,   72,   39,   72,   38,   72,   26,   72,   28,   72,
      42,   72,   24,   72,   27,   72,   36,   72,   41,   72,   32,   72,
      18,   72,   30,   72,   22,   72,   31,   72,   43,   72,   19,   50,
      64,   0,    49,   20,   0,    0,    0,    32,   3,    -97,  6,    0,
      68,   -113, 0,    2,    3,    85,   8,    -113, 0,    68,   -97,  3,
      0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,
      6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113,
      0,    2,    3,    85,   8,    -113, 0,    68,   -97,  3,    0,    2,
      3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,
      68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,
      3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,
      68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,
      3,    -97,  6,    0,    50,   64,   0,    50,   53,   0,    0,    0,
      34,   3,    -97,  6,    0,    68,   -113, 0,    2,    3,    85,   8,
      -113, 0,    68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113,
      0,    2,    3,    85,   8,    -113, 0,    68,   -113, 0,    2,    3,
      -97,  6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,    68,
      -113, 0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,
      85,   8,    -113, 0,    68,   -97,  3,    0,    2,    3,    -97,  6,
      0,    68,   -113, 0,    2,    3,    85,   8,    -113, 0,    68,   -97,
      3,    0,    2,    3,    85,   8,    -113, 0,    68,   -97,  3,    0,
      2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    85,   8,
      -113, 0,    68,   -97,  3,    0,    2,    3,    85,   8,    -113, 0,
      68,   -97,  3,    0,    2,    3,    -97,  6,    0,    68,   -113, 0,
      2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,  6,
      0,    68,   -113, 0,    2,    3,    85,   8,    -113, 0,    68,   -97,
      3,    0,    2,    3,    85,   8,    -113, 0,    68,   -97,  3,    0,
      2,    3,    85,   8,    -113, 0,    68,   -97,  3,    0,    2,    3,
      -97,  6,    0,    50,   64,   0,    51,   85,   0,    0,    0,    36,
      3,    85,   8,    -113, 0,    68,   -97,  3,    0,    2,    3,    -97,
      6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113,
      0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,
      6,    0,    68,   -113, 0,    2,    -97,  5,    0,    2,    3,    85,
      8,    -113, 0,    68,   -97,  3,    0,    2,    3,    -97,  6,    0,
      68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,
      3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,  6,    0,
      68,   -113, 0,    2,    3,    -97,  6,    0,    50,   -64,  0,    51,
      -45,  0,    0,    0,    37,   68,   -113, 0,    2,    3,    -97,  6,
      0,    68,   -113, 0,    2,    3,    -97,  6,    0,    68,   -113, 0,
      2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    -97,  6,
      0,    68,   -113, 0,    2,    3,    85,   8,    -113, 0,    68,   -113,
      0,    2,    3,    -97,  6,    0,    68,   -113, 0,    2,    3,    85,
      8,    -113, 0,    68,   -97,  3,    0,    2,    3,    85,   8,    -113,
      0,    68,   -97,  3,    0,    120,  64,   0,    52,   -88,  0,    0,
      0,    39,   13,   85,   5,    72,   13,   85,   5,    72,   13,   85,
      5,    72,   13,   72,   13,   85,   5,    72,   13,   85,   5,    72,
      13,   85,   5,    72,   13,   85,   5,    72,   13,   72,   13,   85,
      5,    72,   13,   85,   5,    72,   13,   72,   13,   72,   13,   85,
      5,    72,   13,   85,   5,    72,   13,   85,   5,    72,   13,   85,
      5,    72,   13,   85,   5,    72,   13,   85,   5,    72,   13,   72,
      13,   72,   13,   72,   13,   85,   5,    72,   13,   85,   5,    72,
      13,   72,   13,   85,   5,    72,   13,   85,   5,    72,   13,   85,
      5,    72,   13,   85,   5,    72,   13,   85,   5,    72,   13,   85,
      5,    72,   13,   85,   5,    72,   13,   85,   5,    72,   13,   85,
      5,    72,   13,   85,   5,    72,   13,   85,   5,    72,   13,   85,
      5,    72,   13,   72,   13,   72,   13,   72,   13,   85,   5,    72,
      13,   85,   5,    72,   13,   85,   5,    72,   13,   72,   13,   85,
      5,    72,   13,   72,   13,   85,   5,    72,   13,   72,   13,   85,
      5,    72,   13,   -19,  -24,  -101, -35};
  test(data, 9, data.size() - 9);
}
} // namespace org::apache::lucene::codecs::compressing