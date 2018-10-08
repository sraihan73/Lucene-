using namespace std;

#include "TestAssertingPostingsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using Codec = org::apache::lucene::codecs::Codec;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;

shared_ptr<Codec> TestAssertingPostingsFormat::getCodec() { return codec; }

bool TestAssertingPostingsFormat::isPostingsEnumReuseImplemented()
{
  return false;
}
} // namespace org::apache::lucene::codecs::asserting