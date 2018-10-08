using namespace std;

#include "TestLucene70SegmentInfoFormat.h"

namespace org::apache::lucene::codecs::lucene70
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseSegmentInfoFormatTestCase =
    org::apache::lucene::index::BaseSegmentInfoFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

std::deque<std::shared_ptr<Version>>
TestLucene70SegmentInfoFormat::getVersions()
{
  return std::deque<std::shared_ptr<Version>>{Version::LATEST};
}

shared_ptr<Codec> TestLucene70SegmentInfoFormat::getCodec()
{
  return TestUtil::getDefaultCodec();
}
} // namespace org::apache::lucene::codecs::lucene70