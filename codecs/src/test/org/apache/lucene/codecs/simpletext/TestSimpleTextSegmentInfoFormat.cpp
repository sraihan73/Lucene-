using namespace std;

#include "TestSimpleTextSegmentInfoFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseSegmentInfoFormatTestCase =
    org::apache::lucene::index::BaseSegmentInfoFormatTestCase;
using Version = org::apache::lucene::util::Version;

std::deque<std::shared_ptr<Version>>
TestSimpleTextSegmentInfoFormat::getVersions()
{
  return std::deque<std::shared_ptr<Version>>{Version::LATEST};
}

shared_ptr<Codec> TestSimpleTextSegmentInfoFormat::getCodec() { return codec; }
} // namespace org::apache::lucene::codecs::simpletext