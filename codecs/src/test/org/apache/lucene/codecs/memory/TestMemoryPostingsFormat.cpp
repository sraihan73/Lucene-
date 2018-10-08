using namespace std;

#include "TestMemoryPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::memory
{
using Codec = org::apache::lucene::codecs::Codec;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestMemoryPostingsFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::memory