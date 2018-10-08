using namespace std;

#include "OrdinalMappingLeafReader.h"

namespace org::apache::lucene::facet::taxonomy
{
using DimConfig = org::apache::lucene::facet::FacetsConfig::DimConfig;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using OrdinalsSegmentReader =
    org::apache::lucene::facet::taxonomy::OrdinalsReader::OrdinalsSegmentReader;
using OrdinalMap = org::apache::lucene::facet::taxonomy::directory::
    DirectoryTaxonomyWriter::OrdinalMap;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FilterBinaryDocValues = org::apache::lucene::index::FilterBinaryDocValues;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;

OrdinalMappingLeafReader::InnerFacetsConfig::InnerFacetsConfig() {}

shared_ptr<BytesRef>
OrdinalMappingLeafReader::InnerFacetsConfig::dedupAndEncode(
    shared_ptr<IntsRef> ordinals)
{
  return FacetsConfig::dedupAndEncode(ordinals);
}

OrdinalMappingLeafReader::OrdinalMappingBinaryDocValues::
    OrdinalMappingBinaryDocValues(
        shared_ptr<OrdinalMappingLeafReader> outerInstance,
        shared_ptr<OrdinalsSegmentReader> ordsReader,
        shared_ptr<BinaryDocValues> in_) 
    : org::apache::lucene::index::FilterBinaryDocValues(in_),
      ordsReader(ordsReader), outerInstance(outerInstance)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override public
// org.apache.lucene.util.BytesRef binaryValue()
shared_ptr<BytesRef>
OrdinalMappingLeafReader::OrdinalMappingBinaryDocValues::binaryValue()
{
  try {
    // NOTE: this isn't quite koscher, because in general
    // multiple threads can call BinaryDV.get which would
    // then conflict on the single ordinals instance, but
    // because this impl is only used for merging, we know
    // only 1 thread calls us:
    ordsReader->get(docID(), ordinals);

    // map_obj the ordinals
    for (int i = 0; i < ordinals->length; i++) {
      ordinals->ints[i] = outerInstance->ordinalMap[ordinals->ints[i]];
    }

    return outerInstance->encode(ordinals);
  } catch (const IOException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("error reading category
    // ordinals for doc " + docID(), e);
    throw runtime_error(L"error reading category ordinals for doc " +
                        to_wstring(docID()));
  }
}

OrdinalMappingLeafReader::OrdinalMappingLeafReader(
    shared_ptr<LeafReader> in_, std::deque<int> &ordinalMap,
    shared_ptr<FacetsConfig> srcConfig)
    : org::apache::lucene::index::FilterLeafReader(in_), ordinalMap(ordinalMap),
      facetsConfig(make_shared<InnerFacetsConfig>()),
      facetFields(unordered_set<>())
{
  for (auto dc : srcConfig->getDimConfigs()) {
    facetFields->add(dc->second.indexFieldName);
  }
  // always add the default indexFieldName. This is because FacetsConfig does
  // not explicitly record dimensions that were indexed under the default
  // DimConfig, unless they have a custome DimConfig.
  facetFields->add(FacetsConfig::DEFAULT_DIM_CONFIG->indexFieldName);
}

shared_ptr<BytesRef>
OrdinalMappingLeafReader::encode(shared_ptr<IntsRef> ordinals)
{
  return facetsConfig->dedupAndEncode(ordinals);
}

shared_ptr<OrdinalsReader>
OrdinalMappingLeafReader::getOrdinalsReader(const wstring &field)
{
  return make_shared<DocValuesOrdinalsReader>(field);
}

shared_ptr<BinaryDocValues> OrdinalMappingLeafReader::getBinaryDocValues(
    const wstring &field) 
{
  if (facetFields->contains(field)) {
    shared_ptr<OrdinalsReader> *const ordsReader = getOrdinalsReader(field);
    return make_shared<OrdinalMappingBinaryDocValues>(
        shared_from_this(), ordsReader->getReader(in_->getContext()),
        in_->getBinaryDocValues(field));
  } else {
    return in_->getBinaryDocValues(field);
  }
}

shared_ptr<CacheHelper> OrdinalMappingLeafReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper> OrdinalMappingLeafReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::facet::taxonomy