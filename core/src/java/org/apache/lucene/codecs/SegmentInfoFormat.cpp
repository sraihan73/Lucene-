using namespace std;

#include "SegmentInfoFormat.h"
#include "../index/SegmentInfo.h"
#include "../store/Directory.h"
#include "../store/IOContext.h"

namespace org::apache::lucene::codecs
{
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

SegmentInfoFormat::SegmentInfoFormat() {}
} // namespace org::apache::lucene::codecs