using namespace std;

#include "TestSimpleTextFieldInfoFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseFieldInfoFormatTestCase =
    org::apache::lucene::index::BaseFieldInfoFormatTestCase;

shared_ptr<Codec> TestSimpleTextFieldInfoFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::simpletext