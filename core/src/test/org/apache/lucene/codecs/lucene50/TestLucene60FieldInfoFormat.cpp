using namespace std;

#include "TestLucene60FieldInfoFormat.h"

namespace org::apache::lucene::codecs::lucene50
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseFieldInfoFormatTestCase =
    org::apache::lucene::index::BaseFieldInfoFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestLucene60FieldInfoFormat::getCodec()
{
  return TestUtil::getDefaultCodec();
}
} // namespace org::apache::lucene::codecs::lucene50