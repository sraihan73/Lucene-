using namespace std;

#include "AssertingDirectoryReader.h"

namespace org::apache::lucene::index
{

shared_ptr<LeafReader>
AssertingDirectoryReader::AssertingSubReaderWrapper::wrap(
    shared_ptr<LeafReader> reader)
{
  return make_shared<AssertingLeafReader>(reader);
}

AssertingDirectoryReader::AssertingDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
    : FilterDirectoryReader(in_, new AssertingSubReaderWrapper())
{
}

shared_ptr<DirectoryReader> AssertingDirectoryReader::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return make_shared<AssertingDirectoryReader>(in_);
}

shared_ptr<CacheHelper> AssertingDirectoryReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}
} // namespace org::apache::lucene::index