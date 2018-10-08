using namespace std;

#include "TestDirectDocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::memory
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseDocValuesFormatTestCase =
    org::apache::lucene::index::BaseDocValuesFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestDirectDocValuesFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::memory