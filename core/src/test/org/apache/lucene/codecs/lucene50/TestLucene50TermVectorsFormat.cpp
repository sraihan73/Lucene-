using namespace std;

#include "TestLucene50TermVectorsFormat.h"

namespace org::apache::lucene::codecs::lucene50
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseTermVectorsFormatTestCase =
    org::apache::lucene::index::BaseTermVectorsFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestLucene50TermVectorsFormat::getCodec()
{
  return TestUtil::getDefaultCodec();
}
} // namespace org::apache::lucene::codecs::lucene50