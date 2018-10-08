using namespace std;

#include "Lucene62RWCodec.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/SegmentInfoFormat.h"

namespace org::apache::lucene::codecs::lucene62
{
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using Lucene53RWNormsFormat =
    org::apache::lucene::codecs::lucene53::Lucene53RWNormsFormat;
using Lucene62Codec = org::apache::lucene::codecs::lucene62::Lucene62Codec;

shared_ptr<SegmentInfoFormat> Lucene62RWCodec::segmentInfoFormat()
{
  return segmentInfoFormat_;
}

shared_ptr<NormsFormat> Lucene62RWCodec::normsFormat() { return normsFormat_; }
} // namespace org::apache::lucene::codecs::lucene62