using namespace std;

#include "TestLucene50LiveDocsFormat.h"

namespace org::apache::lucene::codecs::lucene50
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseLiveDocsFormatTestCase =
    org::apache::lucene::index::BaseLiveDocsFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestLucene50LiveDocsFormat::getCodec()
{
  return TestUtil::getDefaultCodec();
}
} // namespace org::apache::lucene::codecs::lucene50