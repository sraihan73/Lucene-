using namespace std;

#include "FieldInfosFormat.h"
#include "../index/FieldInfos.h"
#include "../index/SegmentInfo.h"
#include "../store/Directory.h"
#include "../store/IOContext.h"

namespace org::apache::lucene::codecs
{
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

FieldInfosFormat::FieldInfosFormat() {}
} // namespace org::apache::lucene::codecs