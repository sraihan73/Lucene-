using namespace std;

#include "CrankySegmentInfoFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

CrankySegmentInfoFormat::CrankySegmentInfoFormat(
    shared_ptr<SegmentInfoFormat> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

shared_ptr<SegmentInfo>
CrankySegmentInfoFormat::read(shared_ptr<Directory> directory,
                              const wstring &segmentName,
                              std::deque<char> &segmentID,
                              shared_ptr<IOContext> context) 
{
  return delegate_->read(directory, segmentName, segmentID, context);
}

void CrankySegmentInfoFormat::write(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> info,
    shared_ptr<IOContext> ioContext) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from SegmentInfoFormat.write()");
  }
  delegate_->write(dir, info, ioContext);
}
} // namespace org::apache::lucene::codecs::cranky