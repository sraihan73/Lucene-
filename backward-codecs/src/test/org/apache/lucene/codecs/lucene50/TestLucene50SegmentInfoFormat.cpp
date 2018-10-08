using namespace std;

#include "TestLucene50SegmentInfoFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../lucene60/Lucene60RWCodec.h"

namespace org::apache::lucene::codecs::lucene50
{
using Codec = org::apache::lucene::codecs::Codec;
using Lucene60RWCodec = org::apache::lucene::codecs::lucene60::Lucene60RWCodec;
using BaseSegmentInfoFormatTestCase =
    org::apache::lucene::index::BaseSegmentInfoFormatTestCase;
using Version = org::apache::lucene::util::Version;

shared_ptr<Codec> TestLucene50SegmentInfoFormat::getCodec()
{
  return make_shared<Lucene60RWCodec>();
}

int TestLucene50SegmentInfoFormat::getCreatedVersionMajor()
{
  return Version::LUCENE_6_0_0->major;
}

std::deque<std::shared_ptr<Version>>
TestLucene50SegmentInfoFormat::getVersions()
{
  return std::deque<std::shared_ptr<Version>>{Version::LUCENE_6_0_0};
}

bool TestLucene50SegmentInfoFormat::supportsIndexSort() { return false; }

bool TestLucene50SegmentInfoFormat::supportsMinVersion() { return false; }
} // namespace org::apache::lucene::codecs::lucene50