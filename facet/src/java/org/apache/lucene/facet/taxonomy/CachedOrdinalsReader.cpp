using namespace std;

#include "CachedOrdinalsReader.h"

namespace org::apache::lucene::facet::taxonomy
{
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using IntsRef = org::apache::lucene::util::IntsRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

CachedOrdinalsReader::CachedOrdinalsReader(shared_ptr<OrdinalsReader> source)
    : source(source)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<CachedOrds> CachedOrdinalsReader::getCachedOrds(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<IndexReader::CacheHelper> cacheHelper =
      context->reader()->getCoreCacheHelper();
  if (cacheHelper == nullptr) {
    throw make_shared<IllegalStateException>(
        L"Cannot cache ordinals on leaf: " + context->reader());
  }
  any cacheKey = cacheHelper->getKey();
  shared_ptr<CachedOrds> ords = ordsCache[cacheKey];
  if (ords == nullptr) {
    ords = make_shared<CachedOrds>(source->getReader(context),
                                   context->reader()->maxDoc());
    ordsCache.emplace(cacheKey, ords);
  }

  return ords;
}

wstring CachedOrdinalsReader::getIndexFieldName()
{
  return source->getIndexFieldName();
}

shared_ptr<OrdinalsSegmentReader> CachedOrdinalsReader::getReader(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<CachedOrds> *const cachedOrds = getCachedOrds(context);
  return make_shared<OrdinalsSegmentReaderAnonymousInnerClass>(
      shared_from_this(), cachedOrds);
}

CachedOrdinalsReader::OrdinalsSegmentReaderAnonymousInnerClass::
    OrdinalsSegmentReaderAnonymousInnerClass(
        shared_ptr<CachedOrdinalsReader> outerInstance,
        shared_ptr<org::apache::lucene::facet::taxonomy::CachedOrdinalsReader::
                       CachedOrds>
            cachedOrds)
{
  this->outerInstance = outerInstance;
  this->cachedOrds = cachedOrds;
}

void CachedOrdinalsReader::OrdinalsSegmentReaderAnonymousInnerClass::get(
    int docID, shared_ptr<IntsRef> ordinals)
{
  ordinals->ints = cachedOrds->ordinals;
  ordinals->offset = cachedOrds->offsets[docID];
  ordinals->length = cachedOrds->offsets[docID + 1] - ordinals->offset;
}

CachedOrdinalsReader::CachedOrds::CachedOrds(
    shared_ptr<OrdinalsSegmentReader> source, int maxDoc) 
    : offsets(std::deque<int>(maxDoc + 1))
{
  std::deque<int> ords(
      maxDoc); // let's assume one ordinal per-document as an initial size

  // this aggregator is limited to Integer.MAX_VALUE total ordinals.
  int64_t totOrds = 0;
  shared_ptr<IntsRef> *const values = make_shared<IntsRef>(32);
  for (int docID = 0; docID < maxDoc; docID++) {
    offsets[docID] = static_cast<int>(totOrds);
    source->get(docID, values);
    int64_t nextLength = totOrds + values->length;
    if (nextLength > ords.size()) {
      if (nextLength > ArrayUtil::MAX_ARRAY_LENGTH) {
        throw make_shared<IllegalStateException>(
            L"too many ordinals (>= " + to_wstring(nextLength) + L") to cache");
      }
      ords = ArrayUtil::grow(ords, static_cast<int>(nextLength));
    }
    System::arraycopy(values->ints, 0, ords, static_cast<int>(totOrds),
                      values->length);
    totOrds = nextLength;
  }
  offsets[maxDoc] = static_cast<int>(totOrds);

  // if ords array is bigger by more than 10% of what we really need, shrink it
  if (static_cast<double>(totOrds) / ords.size() < 0.9) {
    this->ordinals = std::deque<int>(static_cast<int>(totOrds));
    System::arraycopy(ords, 0, this->ordinals, 0, static_cast<int>(totOrds));
  } else {
    this->ordinals = ords;
  }
}

int64_t CachedOrdinalsReader::CachedOrds::ramBytesUsed()
{
  int64_t mem = RamUsageEstimator::shallowSizeOf(shared_from_this()) +
                  RamUsageEstimator::sizeOf(offsets);
  if (offsets != ordinals) {
    mem += RamUsageEstimator::sizeOf(ordinals);
  }
  return mem;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t CachedOrdinalsReader::ramBytesUsed()
{
  int64_t bytes = 0;
  for (auto ords : ordsCache) {
    bytes += ords->second.ramBytesUsed();
  }

  return bytes;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<deque<std::shared_ptr<Accountable>>>
CachedOrdinalsReader::getChildResources()
{
  return Accountables::namedAccountables(L"segment", ordsCache);
}
} // namespace org::apache::lucene::facet::taxonomy