using namespace std;

#include "TestHighCompressionMode.h"

namespace org::apache::lucene::codecs::compressing
{

void TestHighCompressionMode::setUp() 
{
  AbstractTestCompressionMode::setUp();
  mode = CompressionMode::HIGH_COMPRESSION;
}
} // namespace org::apache::lucene::codecs::compressing