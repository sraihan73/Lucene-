using namespace std;

#include "CrankyCompoundFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

CrankyCompoundFormat::CrankyCompoundFormat(shared_ptr<CompoundFormat> delegate_,
                                           shared_ptr<Random> random)
{
  this->delegate_ = delegate_;
  this->random = random;
}

shared_ptr<Directory> CrankyCompoundFormat::getCompoundReader(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  return delegate_->getCompoundReader(dir, si, context);
}

void CrankyCompoundFormat::write(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException from CompoundFormat.write()");
  }
  delegate_->write(dir, si, context);
}
} // namespace org::apache::lucene::codecs::cranky