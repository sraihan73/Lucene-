using namespace std;

#include "SegmentReadState.h"

namespace org::apache::lucene::index
{
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using PerFieldPostingsFormat =
    org::apache::lucene::codecs::perfield::PerFieldPostingsFormat;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

SegmentReadState::SegmentReadState(shared_ptr<Directory> dir,
                                   shared_ptr<SegmentInfo> info,
                                   shared_ptr<FieldInfos> fieldInfos,
                                   shared_ptr<IOContext> context)
    : SegmentReadState(dir, info, fieldInfos, context, L"")
{
}

SegmentReadState::SegmentReadState(shared_ptr<Directory> dir,
                                   shared_ptr<SegmentInfo> info,
                                   shared_ptr<FieldInfos> fieldInfos,
                                   shared_ptr<IOContext> context,
                                   const wstring &segmentSuffix)
    : directory(dir), segmentInfo(info), fieldInfos(fieldInfos),
      context(context), segmentSuffix(segmentSuffix)
{
}

SegmentReadState::SegmentReadState(shared_ptr<SegmentReadState> other,
                                   const wstring &newSegmentSuffix)
    : directory(other->directory), segmentInfo(other->segmentInfo),
      fieldInfos(other->fieldInfos), context(other->context),
      segmentSuffix(newSegmentSuffix)
{
}
} // namespace org::apache::lucene::index