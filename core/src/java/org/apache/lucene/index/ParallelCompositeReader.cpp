using namespace std;

#include "ParallelCompositeReader.h"

namespace org::apache::lucene::index
{

ParallelCompositeReader::ParallelCompositeReader(
    deque<CompositeReader> &readers) 
    : ParallelCompositeReader(true, readers)
{
}

ParallelCompositeReader::ParallelCompositeReader(
    bool closeSubReaders, deque<CompositeReader> &readers) 
    : ParallelCompositeReader(closeSubReaders, readers, readers)
{
}

ParallelCompositeReader::ParallelCompositeReader(
    bool closeSubReaders,
    std::deque<std::shared_ptr<CompositeReader>> &readers,
    std::deque<std::shared_ptr<CompositeReader>>
        &storedFieldReaders) 
    : BaseCompositeReader<LeafReader>(
          prepareLeafReaders(readers, storedFieldReaders)),
      closeSubReaders(closeSubReaders)
{
  Collections::addAll(completeReaderSet, readers);
  Collections::addAll(completeReaderSet, storedFieldReaders);
  // update ref-counts (like MultiReader):
  if (!closeSubReaders) {
    for (auto reader : completeReaderSet) {
      reader->incRef();
    }
  }
  // finally add our own synthetic readers, so we close or decRef them, too (it
  // does not matter what we do)
  completeReaderSet->addAll(getSequentialSubReaders());
  // ParallelReader instances can be short-lived, which would make caching
  // trappy so we do not cache on them, unless they wrap a single reader in
  // which case we delegate
  if (readers.size() == 1 && storedFieldReaders.size() == 1 &&
      readers[0] == storedFieldReaders[0]) {
    cacheHelper = readers[0]->getReaderCacheHelper();
  } else {
    cacheHelper.reset();
  }
}

std::deque<std::shared_ptr<LeafReader>>
ParallelCompositeReader::prepareLeafReaders(
    std::deque<std::shared_ptr<CompositeReader>> &readers,
    std::deque<std::shared_ptr<CompositeReader>>
        &storedFieldsReaders) 
{
  if (readers.empty()) {
    if (storedFieldsReaders.size() > 0) {
      throw invalid_argument(L"There must be at least one main reader if "
                             L"storedFieldsReaders are used.");
    }
    return std::deque<std::shared_ptr<LeafReader>>(0);
  } else {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.List<? extends LeafReaderContext>
    // firstLeaves = readers[0].leaves();
    const deque < ? extends LeafReaderContext > firstLeaves =
                         readers[0]->leaves();

    // check compatibility:
    constexpr int maxDoc = readers[0]->maxDoc(), noLeaves = firstLeaves.size();
    const std::deque<int> leafMaxDoc = std::deque<int>(noLeaves);
    for (int i = 0; i < noLeaves; i++) {
      shared_ptr<LeafReader> *const r = firstLeaves[i]->reader();
      leafMaxDoc[i] = r->maxDoc();
    }
    validate(readers, maxDoc, leafMaxDoc);
    validate(storedFieldsReaders, maxDoc, leafMaxDoc);

    // flatten structure of each Composite to just LeafReader[]
    // and combine parallel structure with ParallelLeafReaders:
    std::deque<std::shared_ptr<LeafReader>> wrappedLeaves(noLeaves);
    for (int i = 0; i < wrappedLeaves.size(); i++) {
      std::deque<std::shared_ptr<LeafReader>> subs(readers.size());
      for (int j = 0; j < readers.size(); j++) {
        subs[j] = readers[j]->leaves()[i].reader();
      }
      std::deque<std::shared_ptr<LeafReader>> storedSubs(
          storedFieldsReaders.size());
      for (int j = 0; j < storedFieldsReaders.size(); j++) {
        storedSubs[j] = storedFieldsReaders[j]->leaves()[i].reader();
      }
      // We pass true for closeSubs and we prevent touching of subreaders in
      // doClose(): By this the synthetic throw-away readers used here are
      // completely invisible to ref-counting
      wrappedLeaves[i] = make_shared<ParallelLeafReaderAnonymousInnerClass>();
    }
    return wrappedLeaves;
  }
}

ParallelCompositeReader::ParallelLeafReaderAnonymousInnerClass::
    ParallelLeafReaderAnonymousInnerClass()
    : ParallelLeafReader(true, subs, storedSubs)
{
}

void ParallelCompositeReader::ParallelLeafReaderAnonymousInnerClass::doClose()
{
}

void ParallelCompositeReader::validate(
    std::deque<std::shared_ptr<CompositeReader>> &readers, int maxDoc,
    std::deque<int> &leafMaxDoc)
{
  for (int i = 0; i < readers.size(); i++) {
    shared_ptr<CompositeReader> *const reader = readers[i];
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.List<? extends LeafReaderContext> subs =
    // reader.leaves();
    const deque < ? extends LeafReaderContext > subs = reader->leaves();
    if (reader->maxDoc() != maxDoc) {
      throw invalid_argument(L"All readers must have same maxDoc: " +
                             to_wstring(maxDoc) + L"!=" +
                             to_wstring(reader->maxDoc()));
    }
    constexpr int noSubs = subs.size();
    if (noSubs != leafMaxDoc.size()) {
      throw invalid_argument(
          L"All readers must have same number of leaf readers");
    }
    for (int subIDX = 0; subIDX < noSubs; subIDX++) {
      shared_ptr<LeafReader> *const r = subs[subIDX]->reader();
      if (r->maxDoc() != leafMaxDoc[subIDX]) {
        throw invalid_argument(
            L"All leaf readers must have same corresponding subReader maxDoc");
      }
    }
  }
}

shared_ptr<CacheHelper> ParallelCompositeReader::getReaderCacheHelper()
{
  return cacheHelper;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ParallelCompositeReader::doClose() 
{
  shared_ptr<IOException> ioe = nullptr;
  for (auto reader : completeReaderSet) {
    try {
      if (closeSubReaders) {
        reader->close();
      } else {
        reader->decRef();
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