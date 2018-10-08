using namespace std;

#include "TestAssertingNormsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseNormsFormatTestCase =
    org::apache::lucene::index::BaseNormsFormatTestCase;

shared_ptr<Codec> TestAssertingNormsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::asserting