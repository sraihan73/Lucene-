using namespace std;

#include "Lucene60RWCodec.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/SegmentInfoFormat.h"

namespace org::apache::lucene::codecs::lucene60
{
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using Lucene50RWSegmentInfoFormat =
    org::apache::lucene::codecs::lucene50::Lucene50RWSegmentInfoFormat;
using Lucene53RWNormsFormat =
    org::apache::lucene::codecs::lucene53::Lucene53RWNormsFormat;

shared_ptr<SegmentInfoFormat> Lucene60RWCodec::segmentInfoFormat()
{
  return segmentInfoFormat_;
}

shared_ptr<NormsFormat> Lucene60RWCodec::normsFormat() { return normsFormat_; }
} // namespace org::apache::lucene::codecs::lucene60