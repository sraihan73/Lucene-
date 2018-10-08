using namespace std;

#include "SortedSetDocValuesFacetCounts.h"

namespace org::apache::lucene::facet::sortedset
{
using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using LabelAndValue = org::apache::lucene::facet::LabelAndValue;
using TopOrdAndIntQueue = org::apache::lucene::facet::TopOrdAndIntQueue;
using OrdRange = org::apache::lucene::facet::sortedset::
    SortedSetDocValuesReaderState::OrdRange;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiSortedSetDocValues =
    org::apache::lucene::index::MultiDocValues::MultiSortedSetDocValues;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using ConjunctionDISI = org::apache::lucene::search::ConjunctionDISI;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongValues = org::apache::lucene::util::LongValues;

SortedSetDocValuesFacetCounts::SortedSetDocValuesFacetCounts(
    shared_ptr<SortedSetDocValuesReaderState> state) 
    : SortedSetDocValuesFacetCounts(state, nullptr)
{
}

SortedSetDocValuesFacetCounts::SortedSetDocValuesFacetCounts(
    shared_ptr<SortedSetDocValuesReaderState> state,
    shared_ptr<FacetsCollector> hits) 
    : state(state), dv(state->getDocValues()), field(state->getField()),
      counts(std::deque<int>(state->getSize()))
{
  if (hits == nullptr) {
    // browse only
    countAll();
  } else {
    count(hits->getMatchingDocs());
  }
}

shared_ptr<FacetResult> SortedSetDocValuesFacetCounts::getTopChildren(
    int topN, const wstring &dim, deque<wstring> &path) 
{
  if (topN <= 0) {
    throw invalid_argument(L"topN must be > 0 (got: " + to_wstring(topN) +
                           L")");
  }
  if (path->length > 0) {
    throw invalid_argument(L"path should be 0 length");
  }
  shared_ptr<OrdRange> ordRange = state->getOrdRange(dim);
  if (ordRange == nullptr) {
    throw invalid_argument(L"dimension \"" + dim + L"\" was not indexed");
  }
  return getDim(dim, ordRange, topN);
}

shared_ptr<FacetResult>
SortedSetDocValuesFacetCounts::getDim(const wstring &dim,
                                      shared_ptr<OrdRange> ordRange,
                                      int topN) 
{

  shared_ptr<TopOrdAndIntQueue> q;

  int bottomCount = 0;

  int dimCount = 0;
  int childCount = 0;

  shared_ptr<TopOrdAndIntQueue::OrdAndValue> reuse = nullptr;
  // System.out.println("getDim : " + ordRange.start + " - " + ordRange.end);
  for (int ord = ordRange->start; ord <= ordRange->end; ord++) {
    // System.out.println("  ord=" + ord + " count=" + counts[ord]);
    if (counts[ord] > 0) {
      dimCount += counts[ord];
      childCount++;
      if (counts[ord] > bottomCount) {
        if (reuse == nullptr) {
          reuse = make_shared<TopOrdAndIntQueue::OrdAndValue>();
        }
        reuse->ord = ord;
        reuse->value = counts[ord];
        if (q->empty()) {
          // Lazy init, so we don't create this for the
          // sparse case unnecessarily
          q = make_shared<TopOrdAndIntQueue>(topN);
        }
        reuse = q->insertWithOverflow(reuse);
        if (q->size() == topN) {
          bottomCount = q->top()->value;
        }
      }
    }
  }

  if (q->empty()) {
    return nullptr;
  }

  std::deque<std::shared_ptr<LabelAndValue>> labelValues(q->size());
  for (int i = labelValues.size() - 1; i >= 0; i--) {
    shared_ptr<TopOrdAndIntQueue::OrdAndValue> ordAndValue = q->pop();
    shared_ptr<BytesRef> *const term = dv->lookupOrd(ordAndValue->ord);
    std::deque<wstring> parts =
        FacetsConfig::stringToPath(term->utf8ToString());
    labelValues[i] = make_shared<LabelAndValue>(parts[1], ordAndValue->value);
  }

  return make_shared<FacetResult>(dim, std::deque<wstring>(0), dimCount,
                                  labelValues, childCount);
}

void SortedSetDocValuesFacetCounts::countOneSegment(
    shared_ptr<OrdinalMap> ordinalMap, shared_ptr<LeafReader> reader,
    int segOrd, shared_ptr<MatchingDocs> hits) 
{
  shared_ptr<SortedSetDocValues> segValues =
      reader->getSortedSetDocValues(field);
  if (segValues == nullptr) {
    // nothing to count
    return;
  }

  shared_ptr<DocIdSetIterator> it;
  if (hits == nullptr) {
    it = segValues;
  } else {
    it = ConjunctionDISI::intersectIterators(
        Arrays::asList(hits->bits->begin(), segValues));
  }

  // TODO: yet another option is to count all segs
  // first, only in seg-ord space, and then do a
  // merge-sort-PQ in the end to only "resolve to
  // global" those seg ords that can compete, if we know
  // we just want top K?  ie, this is the same algo
  // that'd be used for merging facets across shards
  // (distributed faceting).  but this has much higher
  // temp ram req'ts (sum of number of ords across all
  // segs)
  if (ordinalMap != nullptr) {
    shared_ptr<LongValues> *const ordMap = ordinalMap->getGlobalOrds(segOrd);

    int numSegOrds = static_cast<int>(segValues->getValueCount());

    if (hits != nullptr && hits->totalHits < numSegOrds / 10) {
      // System.out.println("    remap as-we-go");
      // Remap every ord to global ord as we iterate:
      for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
           doc = it->nextDoc()) {
        int term = static_cast<int>(segValues->nextOrd());
        while (term != SortedSetDocValues::NO_MORE_ORDS) {
          // System.out.println("      segOrd=" + segOrd + " ord=" + term + "
          // globalOrd=" + ordinalMap.getGlobalOrd(segOrd, term));
          counts[static_cast<int>(ordMap->get(term))]++;
          term = static_cast<int>(segValues->nextOrd());
        }
      }
    } else {
      // System.out.println("    count in seg ord first");

      // First count in seg-ord space:
      const std::deque<int> segCounts = std::deque<int>(numSegOrds);
      for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
           doc = it->nextDoc()) {
        int term = static_cast<int>(segValues->nextOrd());
        while (term != SortedSetDocValues::NO_MORE_ORDS) {
          // System.out.println("      ord=" + term);
          segCounts[term]++;
          term = static_cast<int>(segValues->nextOrd());
        }
      }

      // Then, migrate to global ords:
      for (int ord = 0; ord < numSegOrds; ord++) {
        int count = segCounts[ord];
        if (count != 0) {
          // System.out.println("    migrate segOrd=" + segOrd + " ord=" + ord +
          // " globalOrd=" + ordinalMap.getGlobalOrd(segOrd, ord));
          counts[static_cast<int>(ordMap->get(ord))] += count;
        }
      }
    }
  } else {
    // No ord mapping (e.g., single segment index):
    // just aggregate directly into counts:
    for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = it->nextDoc()) {
      int term = static_cast<int>(segValues->nextOrd());
      while (term != SortedSetDocValues::NO_MORE_ORDS) {
        counts[term]++;
        term = static_cast<int>(segValues->nextOrd());
      }
    }
  }
}

void SortedSetDocValuesFacetCounts::count(
    deque<std::shared_ptr<MatchingDocs>> &matchingDocs) 
{
  // System.out.println("ssdv count");

  shared_ptr<OrdinalMap> ordinalMap;

  // TODO: is this right?  really, we need a way to
  // verify that this ordinalMap "matches" the leaves in
  // matchingDocs...
  if (std::dynamic_pointer_cast<MultiSortedSetDocValues>(dv) != nullptr &&
      matchingDocs.size() > 1) {
    ordinalMap =
        (std::static_pointer_cast<MultiSortedSetDocValues>(dv))->mapping;
  } else {
    ordinalMap.reset();
  }

  shared_ptr<IndexReader> reader = state->getReader();

  for (auto hits : matchingDocs) {

    // LUCENE-5090: make sure the provided reader context "matches"
    // the top-level reader passed to the
    // SortedSetDocValuesReaderState, else cryptic
    // AIOOBE can happen:
    if (ReaderUtil::getTopLevelContext(hits->context)->reader() != reader) {
      throw make_shared<IllegalStateException>(
          L"the SortedSetDocValuesReaderState provided to this class does not "
          L"match the reader being searched; you must create a new "
          L"SortedSetDocValuesReaderState every time you open a new "
          L"IndexReader");
    }

    countOneSegment(ordinalMap, hits->context->reader(), hits->context->ord,
                    hits);
  }
}

void SortedSetDocValuesFacetCounts::countAll() 
{
  // System.out.println("ssdv count");

  shared_ptr<OrdinalMap> ordinalMap;

  // TODO: is this right?  really, we need a way to
  // verify that this ordinalMap "matches" the leaves in
  // matchingDocs...
  if (std::dynamic_pointer_cast<MultiSortedSetDocValues>(dv) != nullptr) {
    ordinalMap =
        (std::static_pointer_cast<MultiSortedSetDocValues>(dv))->mapping;
  } else {
    ordinalMap.reset();
  }

  for (auto context : state->getReader()->leaves()) {
    countOneSegment(ordinalMap, context->reader(), context->ord, nullptr);
  }
}

shared_ptr<Number> SortedSetDocValuesFacetCounts::getSpecificValue(
    const wstring &dim, deque<wstring> &path) 
{
  if (path->length != 1) {
    throw invalid_argument(L"path must be length=1");
  }
  int ord = static_cast<int>(dv->lookupTerm(
      make_shared<BytesRef>(FacetsConfig::pathToString(dim, path))));
  if (ord < 0) {
    return -1;
  }

  return counts[ord];
}

deque<std::shared_ptr<FacetResult>>
SortedSetDocValuesFacetCounts::getAllDims(int topN) 
{

  deque<std::shared_ptr<FacetResult>> results =
      deque<std::shared_ptr<FacetResult>>();
  for (auto ent : state->getPrefixToOrdRange()) {
    shared_ptr<FacetResult> fr = getDim(ent.first, ent.second, topN);
    if (fr != nullptr) {
      results.push_back(fr);
    }
  }

  // Sort by highest count:
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(results, new
  // java.util.Comparator<org.apache.lucene.facet.FacetResult>()
  sort(results.begin(), results.end(),
       make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  return results;
}

SortedSetDocValuesFacetCounts::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<SortedSetDocValuesFacetCounts> outerInstance)
{
  this->outerInstance = outerInstance;
}

int SortedSetDocValuesFacetCounts::ComparatorAnonymousInnerClass::compare(
    shared_ptr<FacetResult> a, shared_ptr<FacetResult> b)
{
  if (a->value->intValue() > b->value->intValue()) {
    return -1;
  } else if (b->value->intValue() > a->value->intValue()) {
    return 1;
  } else {
    return a->dim.compare(b->dim);
  }
}
} // namespace org::apache::lucene::facet::sortedset