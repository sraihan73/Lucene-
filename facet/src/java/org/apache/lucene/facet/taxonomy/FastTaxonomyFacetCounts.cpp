using namespace std;

#include "FastTaxonomyFacetCounts.h"

namespace org::apache::lucene::facet::taxonomy
{
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ConjunctionDISI = org::apache::lucene::search::ConjunctionDISI;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

FastTaxonomyFacetCounts::FastTaxonomyFacetCounts(
    shared_ptr<TaxonomyReader> taxoReader, shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> fc) 
    : FastTaxonomyFacetCounts(FacetsConfig::DEFAULT_INDEX_FIELD_NAME,
                              taxoReader, config, fc)
{
}

FastTaxonomyFacetCounts::FastTaxonomyFacetCounts(
    const wstring &indexFieldName, shared_ptr<TaxonomyReader> taxoReader,
    shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> fc) 
    : IntTaxonomyFacets(indexFieldName, taxoReader, config, fc)
{
  count(fc->getMatchingDocs());
}

FastTaxonomyFacetCounts::FastTaxonomyFacetCounts(
    const wstring &indexFieldName, shared_ptr<IndexReader> reader,
    shared_ptr<TaxonomyReader> taxoReader,
    shared_ptr<FacetsConfig> config) 
    : IntTaxonomyFacets(indexFieldName, taxoReader, config, nullptr)
{
  countAll(reader);
}

void FastTaxonomyFacetCounts::count(
    deque<std::shared_ptr<MatchingDocs>> &matchingDocs) 
{
  for (auto hits : matchingDocs) {
    shared_ptr<BinaryDocValues> dv =
        hits->context->reader()->getBinaryDocValues(indexFieldName);
    if (dv == nullptr) { // this reader does not have DocValues for the
                         // requested category deque
      continue;
    }

    shared_ptr<DocIdSetIterator> it = ConjunctionDISI::intersectIterators(
        Arrays::asList(hits->bits->begin(), dv));

    for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = it->nextDoc()) {
      shared_ptr<BytesRef> *const bytesRef = dv->binaryValue();
      std::deque<char> bytes = bytesRef->bytes;
      int end = bytesRef->offset + bytesRef->length;
      int ord = 0;
      int offset = bytesRef->offset;
      int prev = 0;
      while (offset < end) {
        char b = bytes[offset++];
        if (b >= 0) {
          prev = ord = ((ord << 7) | b) + prev;
          increment(ord);
          ord = 0;
        } else {
          ord = (ord << 7) | (b & 0x7F);
        }
      }
    }
  }

  rollup();
}

void FastTaxonomyFacetCounts::countAll(shared_ptr<IndexReader> reader) throw(
    IOException)
{
  for (auto context : reader->leaves()) {
    shared_ptr<BinaryDocValues> dv =
        context->reader()->getBinaryDocValues(indexFieldName);
    if (dv == nullptr) { // this reader does not have DocValues for the
                         // requested category deque
      continue;
    }

    shared_ptr<Bits> liveDocs = context->reader()->getLiveDocs();

    for (int doc = dv->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = dv->nextDoc()) {
      if (liveDocs != nullptr && liveDocs->get(doc) == false) {
        continue;
      }
      shared_ptr<BytesRef> *const bytesRef = dv->binaryValue();
      std::deque<char> bytes = bytesRef->bytes;
      int end = bytesRef->offset + bytesRef->length;
      int ord = 0;
      int offset = bytesRef->offset;
      int prev = 0;
      while (offset < end) {
        char b = bytes[offset++];
        if (b >= 0) {
          prev = ord = ((ord << 7) | b) + prev;
          increment(ord);
          ord = 0;
        } else {
          ord = (ord << 7) | (b & 0x7F);
        }
      }
    }
  }

  rollup();
}
} // namespace org::apache::lucene::facet::taxonomy