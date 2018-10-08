using namespace std;

#include "TestSimpleTextPointsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BasePointsFormatTestCase =
    org::apache::lucene::index::BasePointsFormatTestCase;

shared_ptr<Codec> TestSimpleTextPointsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::simpletext