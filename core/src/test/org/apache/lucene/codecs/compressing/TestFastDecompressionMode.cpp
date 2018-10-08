using namespace std;

#include "TestFastDecompressionMode.h"

namespace org::apache::lucene::codecs::compressing
{

void TestFastDecompressionMode::setUp() 
{
  AbstractTestLZ4CompressionMode::setUp();
  mode = CompressionMode::FAST_DECOMPRESSION;
}

std::deque<char>
TestFastDecompressionMode::test(std::deque<char> &decompressed, int off,
                                int len) 
{
  const std::deque<char> compressed =
      AbstractTestLZ4CompressionMode::test(decompressed, off, len);
  const std::deque<char> compressed2 =
      compress(CompressionMode::FAST::newCompressor(), decompressed, off, len);
  // because of the way this compression mode works, its output is necessarily
  // smaller than the output of CompressionMode.FAST
  assertTrue(compressed.size() <= compressed2.size());
  return compressed;
}
} // namespace org::apache::lucene::codecs::compressing