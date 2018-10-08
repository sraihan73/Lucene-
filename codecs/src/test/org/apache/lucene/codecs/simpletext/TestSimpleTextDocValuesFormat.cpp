using namespace std;

#include "TestSimpleTextDocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseDocValuesFormatTestCase =
    org::apache::lucene::index::BaseDocValuesFormatTestCase;

shared_ptr<Codec> TestSimpleTextDocValuesFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::simpletext