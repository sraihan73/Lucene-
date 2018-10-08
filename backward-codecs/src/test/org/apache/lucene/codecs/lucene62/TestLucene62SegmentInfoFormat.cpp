using namespace std;

#include "TestLucene62SegmentInfoFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "Lucene62RWCodec.h"

namespace org::apache::lucene::codecs::lucene62
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseSegmentInfoFormatTestCase =
    org::apache::lucene::index::BaseSegmentInfoFormatTestCase;
using Version = org::apache::lucene::util::Version;

int TestLucene62SegmentInfoFormat::getCreatedVersionMajor()
{
  return Version::LUCENE_6_2_0->major;
}

std::deque<std::shared_ptr<Version>>
TestLucene62SegmentInfoFormat::getVersions()
{
  return std::deque<std::shared_ptr<Version>>{Version::LUCENE_6_2_0};
}

shared_ptr<Codec> TestLucene62SegmentInfoFormat::getCodec()
{
  return make_shared<Lucene62RWCodec>();
}

bool TestLucene62SegmentInfoFormat::supportsMinVersion() { return false; }
} // namespace org::apache::lucene::codecs::lucene62