using namespace std;

#include "MismatchedDirectoryReader.h"

namespace org::apache::lucene::index
{

MismatchedDirectoryReader::MismatchedSubReaderWrapper::
    MismatchedSubReaderWrapper(shared_ptr<Random> random)
    : random(random)
{
}

shared_ptr<LeafReader>
MismatchedDirectoryReader::MismatchedSubReaderWrapper::wrap(
    shared_ptr<LeafReader> reader)
{
  return make_shared<MismatchedLeafReader>(reader, random);
}

MismatchedDirectoryReader::MismatchedDirectoryReader(
    shared_ptr<DirectoryReader> in_,
    shared_ptr<Random> random) 
    : FilterDirectoryReader(in_, new MismatchedSubReaderWrapper(random))
{
}

shared_ptr<DirectoryReader> MismatchedDirectoryReader::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return make_shared<AssertingDirectoryReader>(in_);
}

shared_ptr<CacheHelper> MismatchedDirectoryReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}
} // namespace org::apache::lucene::index