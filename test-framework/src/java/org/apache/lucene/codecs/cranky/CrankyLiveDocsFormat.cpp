using namespace std;

#include "CrankyLiveDocsFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;

CrankyLiveDocsFormat::CrankyLiveDocsFormat(shared_ptr<LiveDocsFormat> delegate_,
                                           shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

shared_ptr<Bits> CrankyLiveDocsFormat::readLiveDocs(
    shared_ptr<Directory> dir, shared_ptr<SegmentCommitInfo> info,
    shared_ptr<IOContext> context) 
{
  return delegate_->readLiveDocs(dir, info, context);
}

void CrankyLiveDocsFormat::writeLiveDocs(
    shared_ptr<Bits> bits, shared_ptr<Directory> dir,
    shared_ptr<SegmentCommitInfo> info, int newDelCount,
    shared_ptr<IOContext> context) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from LiveDocsFormat.writeLiveDocs()");
  }
  delegate_->writeLiveDocs(bits, dir, info, newDelCount, context);
}

void CrankyLiveDocsFormat::files(
    shared_ptr<SegmentCommitInfo> info,
    shared_ptr<deque<wstring>> files) 
{
  // TODO: is this called only from write? if so we should throw exception!
  delegate_->files(info, files);
}
} // namespace org::apache::lucene::codecs::cranky