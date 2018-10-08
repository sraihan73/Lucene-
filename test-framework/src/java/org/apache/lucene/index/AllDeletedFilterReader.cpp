using namespace std;

#include "AllDeletedFilterReader.h"

namespace org::apache::lucene::index
{
using Bits = org::apache::lucene::util::Bits;

AllDeletedFilterReader::AllDeletedFilterReader(shared_ptr<LeafReader> in_)
    : FilterLeafReader(in_),
      liveDocs(make_shared<Bits::MatchNoBits>(in_->maxDoc()))
{
  assert(maxDoc() == 0 || hasDeletions());
}

shared_ptr<Bits> AllDeletedFilterReader::getLiveDocs() { return liveDocs; }

int AllDeletedFilterReader::numDocs() { return 0; }

shared_ptr<CacheHelper> AllDeletedFilterReader::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

shared_ptr<CacheHelper> AllDeletedFilterReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::index