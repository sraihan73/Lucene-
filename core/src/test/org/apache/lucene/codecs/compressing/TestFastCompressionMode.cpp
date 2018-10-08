using namespace std;

#include "TestFastCompressionMode.h"

namespace org::apache::lucene::codecs::compressing
{

void TestFastCompressionMode::setUp() 
{
  AbstractTestLZ4CompressionMode::setUp();
  mode = CompressionMode::FAST;
}
} // namespace org::apache::lucene::codecs::compressing