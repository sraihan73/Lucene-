using namespace std;

#include "TestDirectPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::memory
{
using Codec = org::apache::lucene::codecs::Codec;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

bool TestDirectPostingsFormat::isPostingsEnumReuseImplemented()
{
  return false;
}

shared_ptr<Codec> TestDirectPostingsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::memory