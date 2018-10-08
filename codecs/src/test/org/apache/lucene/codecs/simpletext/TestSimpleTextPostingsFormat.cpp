using namespace std;

#include "TestSimpleTextPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using org::apache::lucene::util::LuceneTestCase::Nightly;

shared_ptr<Codec> TestSimpleTextPostingsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::simpletext