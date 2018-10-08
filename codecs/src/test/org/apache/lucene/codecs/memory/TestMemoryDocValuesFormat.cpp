using namespace std;

#include "TestMemoryDocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::memory
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseCompressingDocValuesFormatTestCase =
    org::apache::lucene::index::BaseCompressingDocValuesFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestMemoryDocValuesFormat::getCodec() { return codec; }

bool TestMemoryDocValuesFormat::codecAcceptsHugeBinaryValues(
    const wstring &field)
{
  return false;
}
} // namespace org::apache::lucene::codecs::memory