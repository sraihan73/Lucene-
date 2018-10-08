using namespace std;

#include "TestLucene53NormsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"

namespace org::apache::lucene::codecs::lucene53
{
using Codec = org::apache::lucene::codecs::Codec;
using Lucene62RWCodec = org::apache::lucene::codecs::lucene62::Lucene62RWCodec;
using BaseNormsFormatTestCase =
    org::apache::lucene::index::BaseNormsFormatTestCase;
using Version = org::apache::lucene::util::Version;

int TestLucene53NormsFormat::getCreatedVersionMajor()
{
  return Version::LUCENE_6_2_0->major;
}

shared_ptr<Codec> TestLucene53NormsFormat::getCodec() { return codec; }

bool TestLucene53NormsFormat::codecSupportsSparsity() { return false; }
} // namespace org::apache::lucene::codecs::lucene53