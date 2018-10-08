using namespace std;

#include "TestAssertingPointsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using Codec = org::apache::lucene::codecs::Codec;
using BasePointsFormatTestCase =
    org::apache::lucene::index::BasePointsFormatTestCase;

shared_ptr<Codec> TestAssertingPointsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::asserting