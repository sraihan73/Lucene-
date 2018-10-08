using namespace std;

#include "TaxonomyFacetCounts.h"

namespace org::apache::lucene::facet::taxonomy
{
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IntsRef = org::apache::lucene::util::IntsRef;

TaxonomyFacetCounts::TaxonomyFacetCounts(
    shared_ptr<OrdinalsReader> ordinalsReader,
    shared_ptr<TaxonomyReader> taxoReader, shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> fc) 
    : IntTaxonomyFacets(ordinalsReader->getIndexFieldName(), taxoReader, config,
                        fc),
      ordinalsReader(ordinalsReader)
{
  count(fc->getMatchingDocs());
}

void TaxonomyFacetCounts::count(
    deque<std::shared_ptr<MatchingDocs>> &matchingDocs) 
{
  shared_ptr<IntsRef> scratch = make_shared<IntsRef>();
  for (auto hits : matchingDocs) {
    shared_ptr<OrdinalsReader::OrdinalsSegmentReader> ords =
        ordinalsReader->getReader(hits->context);
    shared_ptr<DocIdSetIterator> docs = hits->bits->begin();

    int doc;
    while ((doc = docs->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
      ords->get(doc, scratch);
      for (int i = 0; i < scratch->length; i++) {
        increment(scratch->ints[scratch->offset + i]);
      }
    }
  }

  rollup();
}
} // namespace org::apache::lucene::facet::taxonomy