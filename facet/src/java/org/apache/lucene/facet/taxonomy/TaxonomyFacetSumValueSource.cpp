using namespace std;

#include "TaxonomyFacetSumValueSource.h"

namespace org::apache::lucene::facet::taxonomy
{
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IntsRef = org::apache::lucene::util::IntsRef;

TaxonomyFacetSumValueSource::TaxonomyFacetSumValueSource(
    shared_ptr<TaxonomyReader> taxoReader, shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> fc,
    shared_ptr<DoubleValuesSource> valueSource) 
    : TaxonomyFacetSumValueSource(
          new DocValuesOrdinalsReader(FacetsConfig::DEFAULT_INDEX_FIELD_NAME),
          taxoReader, config, fc, valueSource)
{
}

TaxonomyFacetSumValueSource::TaxonomyFacetSumValueSource(
    shared_ptr<OrdinalsReader> ordinalsReader,
    shared_ptr<TaxonomyReader> taxoReader, shared_ptr<FacetsConfig> config,
    shared_ptr<FacetsCollector> fc,
    shared_ptr<DoubleValuesSource> vs) 
    : FloatTaxonomyFacets(ordinalsReader->getIndexFieldName(), taxoReader,
                          config),
      ordinalsReader(ordinalsReader)
{
  sumValues(fc->getMatchingDocs(), fc->getKeepScores(), vs);
}

shared_ptr<DoubleValues> TaxonomyFacetSumValueSource::scores(
    shared_ptr<FacetsCollector::MatchingDocs> hits)
{
  return make_shared<DoubleValuesAnonymousInnerClass>(hits);
}

TaxonomyFacetSumValueSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<FacetsCollector::MatchingDocs> hits)
{
  this->hits = hits;
}

double TaxonomyFacetSumValueSource::DoubleValuesAnonymousInnerClass::
    doubleValue() 
{
  return hits->scores[index];
}

bool TaxonomyFacetSumValueSource::DoubleValuesAnonymousInnerClass::advanceExact(
    int doc) 
{
  index++;
  return true;
}

void TaxonomyFacetSumValueSource::sumValues(
    deque<std::shared_ptr<FacetsCollector::MatchingDocs>> &matchingDocs,
    bool keepScores,
    shared_ptr<DoubleValuesSource> valueSource) 
{

  shared_ptr<IntsRef> scratch = make_shared<IntsRef>();
  for (auto hits : matchingDocs) {
    shared_ptr<OrdinalsReader::OrdinalsSegmentReader> ords =
        ordinalsReader->getReader(hits->context);
    shared_ptr<DoubleValues> scores =
        keepScores ? TaxonomyFacetSumValueSource::scores(hits) : nullptr;
    shared_ptr<DoubleValues> functionValues =
        valueSource->getValues(hits->context, scores);
    shared_ptr<DocIdSetIterator> docs = hits->bits->begin();

    int doc;
    while ((doc = docs->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
      ords->get(doc, scratch);
      if (functionValues->advanceExact(doc)) {
        float value = static_cast<float>(functionValues->doubleValue());
        for (int i = 0; i < scratch->length; i++) {
          values[scratch->ints[i]] += value;
        }
      }
    }
  }

  rollup();
}
} // namespace org::apache::lucene::facet::taxonomy