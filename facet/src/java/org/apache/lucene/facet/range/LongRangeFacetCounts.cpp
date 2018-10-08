using namespace std;

#include "LongRangeFacetCounts.h"

namespace org::apache::lucene::facet::range
{
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

LongRangeFacetCounts::LongRangeFacetCounts(
    const wstring &field, shared_ptr<FacetsCollector> hits,
    deque<LongRange> &ranges) 
    : LongRangeFacetCounts(field, LongValuesSource::fromLongField(field), hits,
                           ranges)
{
}

LongRangeFacetCounts::LongRangeFacetCounts(
    const wstring &field, shared_ptr<LongValuesSource> valueSource,
    shared_ptr<FacetsCollector> hits,
    deque<LongRange> &ranges) 
    : LongRangeFacetCounts(field, valueSource, hits, nullptr, ranges)
{
}

LongRangeFacetCounts::LongRangeFacetCounts(
    const wstring &field, shared_ptr<LongValuesSource> valueSource,
    shared_ptr<FacetsCollector> hits, shared_ptr<Query> fastMatchQuery,
    deque<LongRange> &ranges) 
    : RangeFacetCounts(field, ranges, fastMatchQuery)
{
  count(valueSource, hits->getMatchingDocs());
}

void LongRangeFacetCounts::count(
    shared_ptr<LongValuesSource> valueSource,
    deque<std::shared_ptr<FacetsCollector::MatchingDocs>>
        &matchingDocs) 
{

  std::deque<std::shared_ptr<LongRange>> ranges =
      static_cast<std::deque<std::shared_ptr<LongRange>>>(this->ranges);

  shared_ptr<LongRangeCounter> counter = make_shared<LongRangeCounter>(ranges);

  int missingCount = 0;
  for (auto hits : matchingDocs) {
    shared_ptr<LongValues> fv = valueSource->getValues(hits->context, nullptr);

    totCount += hits->totalHits;
    shared_ptr<DocIdSetIterator> *const fastMatchDocs;
    if (fastMatchQuery != nullptr) {
      shared_ptr<IndexReaderContext> *const topLevelContext =
          ReaderUtil::getTopLevelContext(hits->context);
      shared_ptr<IndexSearcher> *const searcher =
          make_shared<IndexSearcher>(topLevelContext);
      searcher->setQueryCache(nullptr);
      shared_ptr<Weight> *const fastMatchWeight =
          searcher->createWeight(searcher->rewrite(fastMatchQuery), false, 1);
      shared_ptr<Scorer> s = fastMatchWeight->scorer(hits->context);
      if (s == nullptr) {
        continue;
      }
      fastMatchDocs = s->begin();
    } else {
      fastMatchDocs.reset();
    }

    shared_ptr<DocIdSetIterator> docs = hits->bits->begin();
    for (int doc = docs->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;) {
      if (fastMatchDocs != nullptr) {
        int fastMatchDoc = fastMatchDocs->docID();
        if (fastMatchDoc < doc) {
          fastMatchDoc = fastMatchDocs->advance(doc);
        }

        if (doc != fastMatchDoc) {
          doc = docs->advance(fastMatchDoc);
          continue;
        }
      }
      // Skip missing docs:
      if (fv->advanceExact(doc)) {
        counter->add(fv->longValue());
      } else {
        missingCount++;
      }

      doc = docs->nextDoc();
    }
  }

  int x = counter->fillCounts(counts);

  missingCount += x;

  // System.out.println("totCount " + totCount + " x " + x + " missingCount " +
  // missingCount);
  totCount -= missingCount;
}
} // namespace org::apache::lucene::facet::range