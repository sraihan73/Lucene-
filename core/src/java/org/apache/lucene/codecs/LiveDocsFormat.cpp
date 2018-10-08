using namespace std;

#include "LiveDocsFormat.h"
#include "../index/SegmentCommitInfo.h"
#include "../store/Directory.h"
#include "../store/IOContext.h"
#include "../util/Bits.h"

namespace org::apache::lucene::codecs
{
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;

LiveDocsFormat::LiveDocsFormat() {}
} // namespace org::apache::lucene::codecs