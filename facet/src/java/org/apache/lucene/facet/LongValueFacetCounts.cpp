using namespace std;

#include "LongValueFacetCounts.h"

namespace org::apache::lucene::facet
{
using com::carrotsearch::hppc::LongIntScatterMap;
using com::carrotsearch::hppc::cursors::LongIntCursor;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using ConjunctionDISI = org::apache::lucene::search::ConjunctionDISI;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

LongValueFacetCounts::LongValueFacetCounts(const wstring &field,
                                           shared_ptr<FacetsCollector> hits,
                                           bool multiValued) 
    : LongValueFacetCounts(field, nullptr, hits, multiValued)
{
}

LongValueFacetCounts::LongValueFacetCounts(
    const wstring &field, shared_ptr<LongValuesSource> valueSource,
    shared_ptr<FacetsCollector> hits) 
    : LongValueFacetCounts(field, valueSource, hits, false)
{
}

LongValueFacetCounts::LongValueFacetCounts(
    const wstring &field, shared_ptr<LongValuesSource> valueSource,
    shared_ptr<FacetsCollector> hits, bool multiValued) 
    : field(field)
{
  if (valueSource == nullptr) {
    if (multiValued) {
      countMultiValued(field, hits->getMatchingDocs());
    } else {
      count(field, hits->getMatchingDocs());
    }
  } else {
    // value source is always single valued
    if (multiValued) {
      throw invalid_argument(L"can only compute multi-valued facets directly "
                             L"from doc values (when valueSource is null)");
    }
    count(valueSource, hits->getMatchingDocs());
  }
}

LongValueFacetCounts::LongValueFacetCounts(const wstring &field,
                                           shared_ptr<IndexReader> reader,
                                           bool multiValued) 
    : field(field)
{
  if (multiValued) {
    countAllMultiValued(reader, field);
  } else {
    countAll(reader, field);
  }
}

LongValueFacetCounts::LongValueFacetCounts(
    const wstring &field, shared_ptr<LongValuesSource> valueSource,
    shared_ptr<IndexReader> reader) 
    : field(field)
{
  countAll(valueSource, field, reader);
}

void LongValueFacetCounts::count(
    shared_ptr<LongValuesSource> valueSource,
    deque<std::shared_ptr<MatchingDocs>> &matchingDocs) 
{

  for (auto hits : matchingDocs) {
    shared_ptr<LongValues> fv = valueSource->getValues(hits->context, nullptr);

    // NOTE: this is not as efficient as working directly with the doc values
    // APIs in the sparse case because we are doing a linear scan across all
    // hits, but this API is more flexible since a LongValuesSource can compute
    // interesting values at query time

    shared_ptr<DocIdSetIterator> docs = hits->bits->begin();
    for (int doc = docs->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;) {
      // Skip missing docs:
      if (fv->advanceExact(doc)) {
        increment(fv->longValue());
        totCount++;
      }

      doc = docs->nextDoc();
    }
  }
}

void LongValueFacetCounts::count(
    const wstring &field,
    deque<std::shared_ptr<MatchingDocs>> &matchingDocs) 
{
  for (auto hits : matchingDocs) {
    shared_ptr<NumericDocValues> fv =
        hits->context->reader()->getNumericDocValues(field);
    if (fv == nullptr) {
      continue;
    }
    countOneSegment(fv, hits);
  }
}

void LongValueFacetCounts::countOneSegment(
    shared_ptr<NumericDocValues> values,
    shared_ptr<MatchingDocs> hits) 
{
  shared_ptr<DocIdSetIterator> it = ConjunctionDISI::intersectIterators(
      Arrays::asList(hits->bits->begin(), values));

  for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = it->nextDoc()) {
    increment(values->longValue());
    totCount++;
  }
}

void LongValueFacetCounts::countMultiValued(
    const wstring &field,
    deque<std::shared_ptr<MatchingDocs>> &matchingDocs) 
{

  for (auto hits : matchingDocs) {
    shared_ptr<SortedNumericDocValues> values =
        hits->context->reader()->getSortedNumericDocValues(field);
    if (values == nullptr) {
      // this field has no doc values for this segment
      continue;
    }

    shared_ptr<NumericDocValues> singleValues =
        DocValues::unwrapSingleton(values);

    if (singleValues != nullptr) {
      countOneSegment(singleValues, hits);
    } else {

      shared_ptr<DocIdSetIterator> it = ConjunctionDISI::intersectIterators(
          Arrays::asList(hits->bits->begin(), values));

      for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
           doc = it->nextDoc()) {
        int limit = values->docValueCount();
        totCount += limit;
        for (int i = 0; i < limit; i++) {
          increment(values->nextValue());
        }
      }
    }
  }
}

void LongValueFacetCounts::countAll(shared_ptr<IndexReader> reader,
                                    const wstring &field) 
{

  for (auto context : reader->leaves()) {

    shared_ptr<NumericDocValues> values =
        context->reader()->getNumericDocValues(field);
    if (values == nullptr) {
      // this field has no doc values for this segment
      continue;
    }

    countAllOneSegment(values);
  }
}

void LongValueFacetCounts::countAllOneSegment(
    shared_ptr<NumericDocValues> values) 
{
  int doc;
  while ((doc = values->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    totCount++;
    increment(values->longValue());
  }
}

void LongValueFacetCounts::countAll(
    shared_ptr<LongValuesSource> valueSource, const wstring &field,
    shared_ptr<IndexReader> reader) 
{

  for (auto context : reader->leaves()) {
    shared_ptr<LongValues> fv = valueSource->getValues(context, nullptr);
    int maxDoc = context->reader()->maxDoc();

    for (int doc = 0; doc < maxDoc; doc++) {
      // Skip missing docs:
      if (fv->advanceExact(doc)) {
        increment(fv->longValue());
        totCount++;
      }
    }
  }
}

void LongValueFacetCounts::countAllMultiValued(
    shared_ptr<IndexReader> reader, const wstring &field) 
{

  for (auto context : reader->leaves()) {

    shared_ptr<SortedNumericDocValues> values =
        context->reader()->getSortedNumericDocValues(field);
    if (values == nullptr) {
      // this field has no doc values for this segment
      continue;
    }
    shared_ptr<NumericDocValues> singleValues =
        DocValues::unwrapSingleton(values);
    if (singleValues != nullptr) {
      countAllOneSegment(singleValues);
    } else {
      int doc;
      while ((doc = values->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
        int limit = values->docValueCount();
        totCount += limit;
        for (int i = 0; i < limit; i++) {
          increment(values->nextValue());
        }
      }
    }
  }
}

void LongValueFacetCounts::increment(int64_t value)
{
  if (value >= 0 && value < counts.size()) {
    counts[static_cast<int>(value)]++;
  } else {
    hashCounts->addTo(value, 1);
  }
}

shared_ptr<FacetResult>
LongValueFacetCounts::getTopChildren(int topN, const wstring &dim,
                                     deque<wstring> &path)
{
  if (dim == field == false) {
    throw invalid_argument(L"invalid dim \"" + dim + L"\"; should be \"" +
                           field + L"\"");
  }
  if (path->length != 0) {
    throw invalid_argument(L"path.length should be 0");
  }
  return getTopChildrenSortByCount(topN);
}

shared_ptr<FacetResult>
LongValueFacetCounts::getTopChildrenSortByCount(int topN)
{
  shared_ptr<PriorityQueue<std::shared_ptr<Entry>>> pq =
      make_shared<PriorityQueueAnonymousInnerClass>(
          shared_from_this(), min(topN, counts.size() + hashCounts->size()));

  int childCount = 0;
  shared_ptr<Entry> e = nullptr;
  for (int i = 0; i < counts.size(); i++) {
    if (counts[i] != 0) {
      childCount++;
      if (e == nullptr) {
        e = make_shared<Entry>();
      }
      e->value = i;
      e->count = counts[i];
      e = pq->insertWithOverflow(e);
    }
  }

  if (hashCounts->size() != 0) {
    childCount += hashCounts->size();
    for (auto c : hashCounts) {
      int count = c->value;
      if (count != 0) {
        if (e == nullptr) {
          e = make_shared<Entry>();
        }
        e->value = c->key;
        e->count = count;
        e = pq->insertWithOverflow(e);
      }
    }
  }

  std::deque<std::shared_ptr<LabelAndValue>> results(pq->size());
  while (pq->size() != 0) {
    shared_ptr<Entry> entry = pq->pop();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    results[pq->size()] =
        make_shared<LabelAndValue>(Long::toString(entry->value), entry->count);
  }

  return make_shared<FacetResult>(field, std::deque<wstring>(0), totCount,
                                  results, childCount);
}

LongValueFacetCounts::PriorityQueueAnonymousInnerClass::
    PriorityQueueAnonymousInnerClass(
        shared_ptr<LongValueFacetCounts> outerInstance, int min)
    : org::apache::lucene::util::PriorityQueue<Entry>(min)
{
  this->outerInstance = outerInstance;
}

bool LongValueFacetCounts::PriorityQueueAnonymousInnerClass::lessThan(
    shared_ptr<Entry> a, shared_ptr<Entry> b)
{
  // sort by count descending, breaking ties by value ascending:
  return a->count < b->count || (a->count == b->count && a->value > b->value);
}

shared_ptr<FacetResult> LongValueFacetCounts::getAllChildrenSortByValue()
{
  deque<std::shared_ptr<LabelAndValue>> labelValues =
      deque<std::shared_ptr<LabelAndValue>>();

  // compact & sort hash table's arrays by value
  std::deque<int> hashCounts(this->hashCounts->size());
  std::deque<int64_t> hashValues(this->hashCounts->size());

  int upto = 0;
  for (auto c : this->hashCounts) {
    if (c->value != 0) {
      hashCounts[upto] = c->value;
      hashValues[upto] = c->key;
      upto++;
    }
  }

  assert((upto == this->hashCounts->size(), L"upto=" + to_wstring(upto) +
                                                L" hashCounts.size=" +
                                                this->hashCounts->size()));

  make_shared<InPlaceMergeSorterAnonymousInnerClass>(shared_from_this(),
                                                     hashCounts, hashValues)
      .sort(0, upto);

  bool countsAdded = false;
  for (int i = 0; i < upto; i++) {
    if (countsAdded == false && hashValues[i] >= counts.size()) {
      countsAdded = true;
      appendCounts(labelValues);
    }

    // C++ TODO: There is no native C++ equivalent to 'toString':
    labelValues.push_back(make_shared<LabelAndValue>(
        Long::toString(hashValues[i]), hashCounts[i]));
  }

  if (countsAdded == false) {
    appendCounts(labelValues);
  }

  return make_shared<FacetResult>(
      field, std::deque<wstring>(0), totCount,
      labelValues.toArray(std::deque<std::shared_ptr<LabelAndValue>>(0)),
      labelValues.size());
}

LongValueFacetCounts::InPlaceMergeSorterAnonymousInnerClass::
    InPlaceMergeSorterAnonymousInnerClass(
        shared_ptr<LongValueFacetCounts> outerInstance, deque<int> &hashCounts,
        deque<int64_t> &hashValues)
{
  this->outerInstance = outerInstance;
  this->hashCounts = hashCounts;
  this->hashValues = hashValues;
}

int LongValueFacetCounts::InPlaceMergeSorterAnonymousInnerClass::compare(int i,
                                                                         int j)
{
  return Long::compare(hashValues[i], hashValues[j]);
}

void LongValueFacetCounts::InPlaceMergeSorterAnonymousInnerClass::swap(int i,
                                                                       int j)
{
  int x = hashCounts[i];
  hashCounts[i] = hashCounts[j];
  hashCounts[j] = x;

  int64_t y = hashValues[j];
  hashValues[j] = hashValues[i];
  hashValues[i] = y;
}

void LongValueFacetCounts::appendCounts(
    deque<std::shared_ptr<LabelAndValue>> &labelValues)
{
  for (int i = 0; i < counts.size(); i++) {
    if (counts[i] != 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      labelValues.push_back(
          make_shared<LabelAndValue>(Long::toString(i), counts[i]));
    }
  }
}

shared_ptr<Number>
LongValueFacetCounts::getSpecificValue(const wstring &dim,
                                       deque<wstring> &path) 
{
  // TODO: should we impl this?
  throw make_shared<UnsupportedOperationException>();
}

deque<std::shared_ptr<FacetResult>>
LongValueFacetCounts::getAllDims(int topN) 
{
  return Collections::singletonList(getTopChildren(topN, field));
}

wstring LongValueFacetCounts::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"LongValueFacetCounts totCount=");
  b->append(totCount);
  b->append(L":\n");
  for (int i = 0; i < counts.size(); i++) {
    if (counts[i] != 0) {
      b->append(L"  ");
      b->append(i);
      b->append(L" -> count=");
      b->append(counts[i]);
      b->append(L'\n');
    }
  }

  if (hashCounts->size() != 0) {
    for (auto c : hashCounts) {
      if (c->value != 0) {
        b->append(L"  ");
        b->append(c->key);
        b->append(L" -> count=");
        b->append(c->value);
        b->append(L'\n');
      }
    }
  }

  return b->toString();
}
} // namespace org::apache::lucene::facet