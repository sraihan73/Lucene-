using namespace std;

#include "TestSimpleTextNormsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseNormsFormatTestCase =
    org::apache::lucene::index::BaseNormsFormatTestCase;

shared_ptr<Codec> TestSimpleTextNormsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::simpletext