using namespace std;

#include "GlobalOrdinalsCollector.h"

namespace org::apache::lucene::search::join
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Collector = org::apache::lucene::search::Collector;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Scorer = org::apache::lucene::search::Scorer;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using LongValues = org::apache::lucene::util::LongValues;

GlobalOrdinalsCollector::GlobalOrdinalsCollector(
    const wstring &field, shared_ptr<OrdinalMap> ordinalMap,
    int64_t valueCount)
    : field(field), collectedOrds(make_shared<LongBitSet>(valueCount)),
      ordinalMap(ordinalMap)
{
}

shared_ptr<LongBitSet> GlobalOrdinalsCollector::getCollectorOrdinals()
{
  return collectedOrds;
}

bool GlobalOrdinalsCollector::needsScores() { return false; }

shared_ptr<LeafCollector> GlobalOrdinalsCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedDocValues> docTermOrds =
      DocValues::getSorted(context->reader(), field);
  if (ordinalMap != nullptr) {
    shared_ptr<LongValues> segmentOrdToGlobalOrdLookup =
        ordinalMap->getGlobalOrds(context->ord);
    return make_shared<OrdinalMapCollector>(shared_from_this(), docTermOrds,
                                            segmentOrdToGlobalOrdLookup);
  } else {
    return make_shared<SegmentOrdinalCollector>(shared_from_this(),
                                                docTermOrds);
  }
}

GlobalOrdinalsCollector::OrdinalMapCollector::OrdinalMapCollector(
    shared_ptr<GlobalOrdinalsCollector> outerInstance,
    shared_ptr<SortedDocValues> docTermOrds,
    shared_ptr<LongValues> segmentOrdToGlobalOrdLookup)
    : docTermOrds(docTermOrds),
      segmentOrdToGlobalOrdLookup(segmentOrdToGlobalOrdLookup),
      outerInstance(outerInstance)
{
}

void GlobalOrdinalsCollector::OrdinalMapCollector::collect(int doc) throw(
    IOException)
{
  if (docTermOrds->advanceExact(doc)) {
    int64_t segmentOrd = docTermOrds->ordValue();
    int64_t globalOrd = segmentOrdToGlobalOrdLookup->get(segmentOrd);
    outerInstance->collectedOrds->set(globalOrd);
  }
}

void GlobalOrdinalsCollector::OrdinalMapCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
}

GlobalOrdinalsCollector::SegmentOrdinalCollector::SegmentOrdinalCollector(
    shared_ptr<GlobalOrdinalsCollector> outerInstance,
    shared_ptr<SortedDocValues> docTermOrds)
    : docTermOrds(docTermOrds), outerInstance(outerInstance)
{
}

void GlobalOrdinalsCollector::SegmentOrdinalCollector::collect(int doc) throw(
    IOException)
{
  if (docTermOrds->advanceExact(doc)) {
    outerInstance->collectedOrds->set(docTermOrds->ordValue());
  }
}

void GlobalOrdinalsCollector::SegmentOrdinalCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
}
} // namespace org::apache::lucene::search::join