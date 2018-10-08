using namespace std;

#include "DoubleRangeFacetCounts.h"

namespace org::apache::lucene::facet::range
{
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using NumericUtils = org::apache::lucene::util::NumericUtils;

DoubleRangeFacetCounts::DoubleRangeFacetCounts(
    const wstring &field, shared_ptr<FacetsCollector> hits,
    deque<DoubleRange> &ranges) 
    : DoubleRangeFacetCounts(field, DoubleValuesSource::fromDoubleField(field),
                             hits, ranges)
{
}

DoubleRangeFacetCounts::DoubleRangeFacetCounts(
    const wstring &field, shared_ptr<DoubleValuesSource> valueSource,
    shared_ptr<FacetsCollector> hits,
    deque<DoubleRange> &ranges) 
    : DoubleRangeFacetCounts(field, valueSource, hits, nullptr, ranges)
{
}

DoubleRangeFacetCounts::DoubleRangeFacetCounts(
    const wstring &field, shared_ptr<DoubleValuesSource> valueSource,
    shared_ptr<FacetsCollector> hits, shared_ptr<Query> fastMatchQuery,
    deque<DoubleRange> &ranges) 
    : RangeFacetCounts(field, ranges, fastMatchQuery)
{
  count(valueSource, hits->getMatchingDocs());
}

void DoubleRangeFacetCounts::count(
    shared_ptr<DoubleValuesSource> valueSource,
    deque<std::shared_ptr<FacetsCollector::MatchingDocs>>
        &matchingDocs) 
{

  std::deque<std::shared_ptr<DoubleRange>> ranges =
      static_cast<std::deque<std::shared_ptr<DoubleRange>>>(this->ranges);

  std::deque<std::shared_ptr<LongRange>> longRanges(ranges.size());
  for (int i = 0; i < ranges.size(); i++) {
    shared_ptr<DoubleRange> range = ranges[i];
    longRanges[i] = make_shared<LongRange>(
        range->label, NumericUtils::doubleToSortableLong(range->min), true,
        NumericUtils::doubleToSortableLong(range->max), true);
  }

  shared_ptr<LongRangeCounter> counter =
      make_shared<LongRangeCounter>(longRanges);

  int missingCount = 0;
  for (auto hits : matchingDocs) {
    shared_ptr<DoubleValues> fv =
        valueSource->getValues(hits->context, nullptr);

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
        counter->add(NumericUtils::doubleToSortableLong(fv->doubleValue()));
      } else {
        missingCount++;
      }

      doc = docs->nextDoc();
    }
  }

  missingCount += counter->fillCounts(counts);
  totCount -= missingCount;
}
} // namespace org::apache::lucene::facet::range