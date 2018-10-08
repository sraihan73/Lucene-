using namespace std;

#include "TestLucene50StoredFieldsFormat.h"

namespace org::apache::lucene::codecs::lucene50
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseStoredFieldsFormatTestCase =
    org::apache::lucene::index::BaseStoredFieldsFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestLucene50StoredFieldsFormat::getCodec()
{
  return TestUtil::getDefaultCodec();
}
} // namespace org::apache::lucene::codecs::lucene50