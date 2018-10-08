using namespace std;

#include "SegmentWriteState.h"

namespace org::apache::lucene::index
{
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using PerFieldPostingsFormat =
    org::apache::lucene::codecs::perfield::PerFieldPostingsFormat;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using InfoStream = org::apache::lucene::util::InfoStream;

SegmentWriteState::SegmentWriteState(shared_ptr<InfoStream> infoStream,
                                     shared_ptr<Directory> directory,
                                     shared_ptr<SegmentInfo> segmentInfo,
                                     shared_ptr<FieldInfos> fieldInfos,
                                     shared_ptr<BufferedUpdates> segUpdates,
                                     shared_ptr<IOContext> context)
    : SegmentWriteState(infoStream, directory, segmentInfo, fieldInfos,
                        segUpdates, context, L"")
{
}

SegmentWriteState::SegmentWriteState(shared_ptr<InfoStream> infoStream,
                                     shared_ptr<Directory> directory,
                                     shared_ptr<SegmentInfo> segmentInfo,
                                     shared_ptr<FieldInfos> fieldInfos,
                                     shared_ptr<BufferedUpdates> segUpdates,
                                     shared_ptr<IOContext> context,
                                     const wstring &segmentSuffix)
    : infoStream(infoStream), directory(directory), segmentInfo(segmentInfo),
      fieldInfos(fieldInfos), segUpdates(segUpdates),
      segmentSuffix(segmentSuffix), context(context)
{
  assert(assertSegmentSuffix(segmentSuffix));
}

SegmentWriteState::SegmentWriteState(shared_ptr<SegmentWriteState> state,
                                     const wstring &segmentSuffix)
    : infoStream(state->infoStream), directory(state->directory),
      segmentInfo(state->segmentInfo), fieldInfos(state->fieldInfos),
      segUpdates(state->segUpdates), segmentSuffix(segmentSuffix),
      context(state->context)
{
  delCountOnFlush = state->delCountOnFlush;
  liveDocs = state->liveDocs;
}

bool SegmentWriteState::assertSegmentSuffix(const wstring &segmentSuffix)
{
  assert(segmentSuffix != L"");
  if (!segmentSuffix.isEmpty()) {
    int numParts = segmentSuffix.split(L"_")->length;
    if (numParts == 2) {
      return true;
    } else if (numParts == 1) {
      // C++ TODO: Only single-argument parse and valueOf methods are converted:
      // ORIGINAL LINE: Long.parseLong(segmentSuffix, Character.MAX_RADIX);
      int64_t ::valueOf(segmentSuffix, Character::MAX_RADIX);
      return true;
    }
    return false; // invalid
  }
  return true;
}
} // namespace org::apache::lucene::index