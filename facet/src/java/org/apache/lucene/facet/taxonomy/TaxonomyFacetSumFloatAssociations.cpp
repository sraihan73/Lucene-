using namespace std;

#include "TaxonomyFacetSumFloatAssociations.h"

namespace org::apache::lucene::facet::taxonomy
{
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BytesRef = org::apache::lucene::util::BytesRef;

TaxonomyFacetSumFloatAssociations::TaxonomyFacetSumFloatAssociations(
    shared_ptr<TaxonomyReader> taxoReader, shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> fc) 
    : TaxonomyFacetSumFloatAssociations(FacetsConfig::DEFAULT_INDEX_FIELD_NAME,
                                        taxoReader, config, fc)
{
}

TaxonomyFacetSumFloatAssociations::TaxonomyFacetSumFloatAssociations(
    const wstring &indexFieldName, shared_ptr<TaxonomyReader> taxoReader,
    shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> fc) 
    : FloatTaxonomyFacets(indexFieldName, taxoReader, config)
{
  sumValues(fc->getMatchingDocs());
}

void TaxonomyFacetSumFloatAssociations::sumValues(
    deque<std::shared_ptr<MatchingDocs>> &matchingDocs) 
{
  // System.out.println("count matchingDocs=" + matchingDocs + " facetsField=" +
  // facetsFieldName);
  for (auto hits : matchingDocs) {
    shared_ptr<BinaryDocValues> dv =
        hits->context->reader()->getBinaryDocValues(indexFieldName);
    if (dv == nullptr) { // this reader does not have DocValues for the
                         // requested category deque
      continue;
    }

    shared_ptr<DocIdSetIterator> docs = hits->bits->begin();

    int doc;
    while ((doc = docs->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
      // System.out.println("  doc=" + doc);
      // TODO: use OrdinalsReader?  we'd need to add a
      // BytesRef getAssociation()?
      if (dv->docID() < doc) {
        dv->advance(doc);
      }
      if (dv->docID() == doc) {
        shared_ptr<BytesRef> *const bytesRef = dv->binaryValue();
        std::deque<char> bytes = bytesRef->bytes;
        int end = bytesRef->offset + bytesRef->length;
        int offset = bytesRef->offset;
        while (offset < end) {
          int ord = ((bytes[offset] & 0xFF) << 24) |
                    ((bytes[offset + 1] & 0xFF) << 16) |
                    ((bytes[offset + 2] & 0xFF) << 8) |
                    (bytes[offset + 3] & 0xFF);
          offset += 4;
          int value = ((bytes[offset] & 0xFF) << 24) |
                      ((bytes[offset + 1] & 0xFF) << 16) |
                      ((bytes[offset + 2] & 0xFF) << 8) |
                      (bytes[offset + 3] & 0xFF);
          offset += 4;
          values[ord] += Float::intBitsToFloat(value);
        }
      }
    }
  }
}
} // namespace org::apache::lucene::facet::taxonomy