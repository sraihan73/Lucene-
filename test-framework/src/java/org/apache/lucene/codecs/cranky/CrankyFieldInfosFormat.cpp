using namespace std;

#include "CrankyFieldInfosFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

CrankyFieldInfosFormat::CrankyFieldInfosFormat(
    shared_ptr<FieldInfosFormat> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

shared_ptr<FieldInfos>
CrankyFieldInfosFormat::read(shared_ptr<Directory> directory,
                             shared_ptr<SegmentInfo> segmentInfo,
                             const wstring &segmentSuffix,
                             shared_ptr<IOContext> iocontext) 
{
  return delegate_->read(directory, segmentInfo, segmentSuffix, iocontext);
}

void CrankyFieldInfosFormat::write(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> segmentInfo,
    const wstring &segmentSuffix, shared_ptr<FieldInfos> infos,
    shared_ptr<IOContext> context) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from FieldInfosFormat.getFieldInfosWriter()");
  }
  delegate_->write(directory, segmentInfo, segmentSuffix, infos, context);
}
} // namespace org::apache::lucene::codecs::cranky