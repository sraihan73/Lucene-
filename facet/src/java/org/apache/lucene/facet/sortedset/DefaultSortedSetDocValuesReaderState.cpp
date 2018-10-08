using namespace std;

#include "DefaultSortedSetDocValuesReaderState.h"

namespace org::apache::lucene::facet::sortedset
{
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using OrdRange = org::apache::lucene::facet::sortedset::
    SortedSetDocValuesReaderState::OrdRange;
using DocValues = org::apache::lucene::index::DocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiSortedSetDocValues =
    org::apache::lucene::index::MultiDocValues::MultiSortedSetDocValues;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;

DefaultSortedSetDocValuesReaderState::DefaultSortedSetDocValuesReaderState(
    shared_ptr<IndexReader> reader) 
    : DefaultSortedSetDocValuesReaderState(
          reader, FacetsConfig::DEFAULT_INDEX_FIELD_NAME)
{
}

DefaultSortedSetDocValuesReaderState::DefaultSortedSetDocValuesReaderState(
    shared_ptr<IndexReader> reader, const wstring &field) 
    : field(field), valueCount(static_cast<int>(dv::getValueCount())),
      reader(reader)
{

  // We need this to create thread-safe MultiSortedSetDV
  // per collector:
  shared_ptr<SortedSetDocValues> dv = getDocValues();
  if (dv == nullptr) {
    throw invalid_argument(L"field \"" + field +
                           L"\" was not indexed with SortedSetDocValues");
  }
  if (dv->getValueCount() > numeric_limits<int>::max()) {
    throw invalid_argument(
        L"can only handle valueCount < Integer.MAX_VALUE; got " +
        to_wstring(dv->getValueCount()));
  }

  // TODO: we can make this more efficient if eg we can be
  // "involved" when OrdinalMap is being created?  Ie see
  // each term/ord it's assigning as it goes...
  wstring lastDim = L"";
  int startOrd = -1;

  // TODO: this approach can work for full hierarchy?;
  // TaxoReader can't do this since ords are not in
  // "sorted order" ... but we should generalize this to
  // support arbitrary hierarchy:
  for (int ord = 0; ord < valueCount; ord++) {
    shared_ptr<BytesRef> *const term = dv->lookupOrd(ord);
    std::deque<wstring> components =
        FacetsConfig::stringToPath(term->utf8ToString());
    if (components.size() != 2) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw invalid_argument(
          L"this class can only handle 2 level hierarchy (dim/value); got: " +
          Arrays->toString(components) + L" " + term->utf8ToString());
    }
    if (components[0] != lastDim) {
      if (lastDim != L"") {
        prefixToOrdRange.emplace(lastDim,
                                 make_shared<OrdRange>(startOrd, ord - 1));
      }
      startOrd = ord;
      lastDim = components[0];
    }
  }

  if (lastDim != L"") {
    prefixToOrdRange.emplace(lastDim,
                             make_shared<OrdRange>(startOrd, valueCount - 1));
  }
}

int64_t DefaultSortedSetDocValuesReaderState::ramBytesUsed()
{
  {
    lock_guard<mutex> lock(cachedOrdMaps);
    int64_t bytes = 0;
    for (auto map_obj : cachedOrdMaps) {
      bytes += map_obj->second.ramBytesUsed();
    }

    return bytes;
  }
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
DefaultSortedSetDocValuesReaderState::getChildResources()
{
  {
    lock_guard<mutex> lock(cachedOrdMaps);
    return Accountables::namedAccountables(
        L"DefaultSortedSetDocValuesReaderState", cachedOrdMaps);
  }
}

wstring DefaultSortedSetDocValuesReaderState::toString()
{
  return L"DefaultSortedSetDocValuesReaderState(field=" + field + L" reader=" +
         reader + L")";
}

shared_ptr<SortedSetDocValues>
DefaultSortedSetDocValuesReaderState::getDocValues() 
{
  // TODO: this is dup'd from slow composite reader wrapper ... can we factor it
  // out to share?
  shared_ptr<OrdinalMap> map_obj = nullptr;
  // TODO: why are we lazy about this?  It's better if ctor pays the cost, not
  // first query?  Oh, but we call this method from ctor, ok.  Also, we only
  // ever store one entry in the map_obj (for key=field) so why are we using a map_obj?
  {
    lock_guard<mutex> lock(cachedOrdMaps);
    map_obj = cachedOrdMaps[field];
    if (map_obj == nullptr) {
      // uncached, or not a multi dv
      shared_ptr<SortedSetDocValues> dv =
          MultiDocValues::getSortedSetValues(reader, field);
      if (std::dynamic_pointer_cast<MultiSortedSetDocValues>(dv) != nullptr) {
        map_obj = (std::static_pointer_cast<MultiSortedSetDocValues>(dv))->mapping;
        shared_ptr<IndexReader::CacheHelper> cacheHelper =
            reader->getReaderCacheHelper();
        if (cacheHelper != nullptr && map_obj->owner == cacheHelper->getKey()) {
          cachedOrdMaps.emplace(field, map_obj);
        }
      }
      return dv;
    }
  }

  assert(map_obj != nullptr);
  int size = reader->leaves().size();
  std::deque<std::shared_ptr<SortedSetDocValues>> values(size);
  const std::deque<int> starts = std::deque<int>(size + 1);
  int64_t cost = 0;
  for (int i = 0; i < size; i++) {
    shared_ptr<LeafReaderContext> context = reader->leaves()[i];
    shared_ptr<LeafReader> *const reader = context->reader();
    shared_ptr<FieldInfo> *const fieldInfo =
        reader->getFieldInfos()->fieldInfo(field);
    if (fieldInfo != nullptr &&
        fieldInfo->getDocValuesType() != DocValuesType::SORTED_SET) {
      return nullptr;
    }
    shared_ptr<SortedSetDocValues> v = reader->getSortedSetDocValues(field);
    if (v == nullptr) {
      v = DocValues::emptySortedSet();
    }
    values[i] = v;
    starts[i] = context->docBase;
    cost += v->cost();
  }
  starts[size] = reader->maxDoc();
  return make_shared<MultiSortedSetDocValues>(values, starts, map_obj, cost);
}

unordered_map<wstring, std::shared_ptr<OrdRange>>
DefaultSortedSetDocValuesReaderState::getPrefixToOrdRange()
{
  return prefixToOrdRange;
}

shared_ptr<OrdRange>
DefaultSortedSetDocValuesReaderState::getOrdRange(const wstring &dim)
{
  return prefixToOrdRange[dim];
}

wstring DefaultSortedSetDocValuesReaderState::getField() { return field; }

shared_ptr<IndexReader> DefaultSortedSetDocValuesReaderState::getReader()
{
  return reader;
}

int DefaultSortedSetDocValuesReaderState::getSize() { return valueCount; }
} // namespace org::apache::lucene::facet::sortedset