using namespace std;

#include "TestAssertingTermVectorsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseTermVectorsFormatTestCase =
    org::apache::lucene::index::BaseTermVectorsFormatTestCase;

shared_ptr<Codec> TestAssertingTermVectorsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::asserting