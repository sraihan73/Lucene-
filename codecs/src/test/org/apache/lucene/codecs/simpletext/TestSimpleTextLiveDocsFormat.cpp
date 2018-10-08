using namespace std;

#include "TestSimpleTextLiveDocsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseLiveDocsFormatTestCase =
    org::apache::lucene::index::BaseLiveDocsFormatTestCase;

shared_ptr<Codec> TestSimpleTextLiveDocsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::simpletext