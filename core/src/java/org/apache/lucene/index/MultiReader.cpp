using namespace std;

#include "MultiReader.h"

namespace org::apache::lucene::index
{

MultiReader::MultiReader(deque<IndexReader> &subReaders) 
    : MultiReader(subReaders, true)
{
}

MultiReader::MultiReader(std::deque<std::shared_ptr<IndexReader>> &subReaders,
                         bool closeSubReaders) 
    : BaseCompositeReader<IndexReader>(subReaders.clone()),
      closeSubReaders(closeSubReaders)
{
  if (!closeSubReaders) {
    for (int i = 0; i < subReaders.size(); i++) {
      subReaders[i]->incRef();
    }
  }
}

shared_ptr<CacheHelper> MultiReader::getReaderCacheHelper()
{
  // MultiReader instances can be short-lived, which would make caching trappy
  // so we do not cache on them, unless they wrap a single reader in which
  // case we delegate
  if (getSequentialSubReaders().size() == 1) {
    return getSequentialSubReaders()[0]->getReaderCacheHelper();
  }
  return nullptr;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MultiReader::doClose() 
{
  shared_ptr<IOException> ioe = nullptr;
  for (auto r : getSequentialSubReaders()) {
    try {
      if (closeSubReaders) {
        r->close();
      } else {
        r->decRef();
      }
    } catch (const IOException &e) {
      if (ioe == nullptr) {
        ioe = e;
      }
    }
  }
  // throw the first exception
  if (ioe != nullptr) {
    throw ioe;
  }
}
} // namespace org::apache::lucene::index