using namespace std;

#include "TestSimpleTextCompoundFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseCompoundFormatTestCase =
    org::apache::lucene::index::BaseCompoundFormatTestCase;

shared_ptr<Codec> TestSimpleTextCompoundFormat::getCodec() { return codec; }

void TestSimpleTextCompoundFormat::testCorruptFilesAreCaught()
{
  // SimpleText does not catch broken sub-files in CFS!
}

void TestSimpleTextCompoundFormat::testMissingCodecHeadersAreCaught()
{
  // SimpleText does not catch broken sub-files in CFS!
}
} // namespace org::apache::lucene::codecs::simpletext