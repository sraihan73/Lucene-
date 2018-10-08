using namespace std;

#include "SortedSetDocValuesRangeQuery.h"

namespace org::apache::lucene::document
{
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocValuesFieldExistsQuery =
    org::apache::lucene::search::DocValuesFieldExistsQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using BytesRef = org::apache::lucene::util::BytesRef;

SortedSetDocValuesRangeQuery::SortedSetDocValuesRangeQuery(
    const wstring &field, shared_ptr<BytesRef> lowerValue,
    shared_ptr<BytesRef> upperValue, bool lowerInclusive, bool upperInclusive)
    : field(Objects::requireNonNull(field)), lowerValue(lowerValue),
      upperValue(upperValue),
      lowerInclusive(lowerInclusive && lowerValue != nullptr),
      upperInclusive(upperInclusive && upperValue != nullptr)
{
}

bool SortedSetDocValuesRangeQuery::equals(any obj)
{
  if (sameClassAs(obj) == false) {
    return false;
  }
  shared_ptr<SortedSetDocValuesRangeQuery> that =
      any_cast<std::shared_ptr<SortedSetDocValuesRangeQuery>>(obj);
  return Objects::equals(field, that->field) &&
         Objects::equals(lowerValue, that->lowerValue) &&
         Objects::equals(upperValue, that->upperValue) &&
         lowerInclusive == that->lowerInclusive &&
         upperInclusive == that->upperInclusive;
}

int SortedSetDocValuesRangeQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + field.hashCode();
  h = 31 * h + Objects::hashCode(lowerValue);
  h = 31 * h + Objects::hashCode(upperValue);
  h = 31 * h + Boolean::hashCode(lowerInclusive);
  h = 31 * h + Boolean::hashCode(upperInclusive);
  return h;
}

wstring SortedSetDocValuesRangeQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  if (this->field == field == false) {
    b->append(this->field)->append(L":");
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return b->append(lowerInclusive ? L"[" : L"{")
      ->append(lowerValue == nullptr ? L"*" : lowerValue)
      ->append(L" TO ")
      ->append(upperValue == nullptr ? L"*" : upperValue)
      ->append(upperInclusive ? L"]" : L"}")
      ->toString();
}

shared_ptr<Query> SortedSetDocValuesRangeQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  if (lowerValue == nullptr && upperValue == nullptr) {
    return make_shared<DocValuesFieldExistsQuery>(field);
  }
  return Query::rewrite(reader);
}

shared_ptr<Weight>
SortedSetDocValuesRangeQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                           bool needsScores,
                                           float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

SortedSetDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<SortedSetDocValuesRangeQuery> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
SortedSetDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedSetDocValues> values =
      outerInstance->getValues(context->reader(), outerInstance->field);
  if (values == nullptr) {
    return nullptr;
  }

  constexpr int64_t minOrd;
  if (outerInstance->lowerValue == nullptr) {
    minOrd = 0;
  } else {
    constexpr int64_t ord = values->lookupTerm(outerInstance->lowerValue);
    if (ord < 0) {
      minOrd = -1 - ord;
    } else if (outerInstance->lowerInclusive) {
      minOrd = ord;
    } else {
      minOrd = ord + 1;
    }
  }

  constexpr int64_t maxOrd;
  if (outerInstance->upperValue == nullptr) {
    maxOrd = values->getValueCount() - 1;
  } else {
    constexpr int64_t ord = values->lookupTerm(outerInstance->upperValue);
    if (ord < 0) {
      maxOrd = -2 - ord;
    } else if (outerInstance->upperInclusive) {
      maxOrd = ord;
    } else {
      maxOrd = ord - 1;
    }
  }

  if (minOrd > maxOrd) {
    return nullptr;
  }

  shared_ptr<SortedDocValues> *const singleton =
      DocValues::unwrapSingleton(values);
  shared_ptr<TwoPhaseIterator> *const iterator;
  if (singleton != nullptr) {
    iterator = make_shared<TwoPhaseIteratorAnonymousInnerClass>(
        shared_from_this(), minOrd, maxOrd, singleton);
  } else {
    iterator = make_shared<TwoPhaseIteratorAnonymousInnerClass2>(
        shared_from_this(), values, minOrd, maxOrd);
  }
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          iterator);
}

SortedSetDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        int64_t minOrd, int64_t maxOrd,
        shared_ptr<SortedDocValues> singleton)
    : org::apache::lucene::search::TwoPhaseIterator(singleton)
{
  this->outerInstance = outerInstance;
  this->minOrd = minOrd;
  this->maxOrd = maxOrd;
  this->singleton = singleton;
}

bool SortedSetDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  constexpr int64_t ord = singleton->ordValue();
  return ord >= minOrd && ord <= maxOrd;
}

float SortedSetDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 2; // 2 comparisons
}

SortedSetDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass2::TwoPhaseIteratorAnonymousInnerClass2(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<SortedSetDocValues> values, int64_t minOrd,
        int64_t maxOrd)
    : org::apache::lucene::search::TwoPhaseIterator(values)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->minOrd = minOrd;
  this->maxOrd = maxOrd;
}

bool SortedSetDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass2::matches() 
{
  for (int64_t ord = values->nextOrd();
       ord != SortedSetDocValues::NO_MORE_ORDS; ord = values->nextOrd()) {
    if (ord < minOrd) {
      continue;
    }
    // Values are sorted, so the first ord that is >= minOrd is our best
    // candidate
    return ord <= maxOrd;
  }
  return false; // all ords were < minOrd
}

float SortedSetDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass2::matchCost()
{
  return 2; // 2 comparisons
}

bool SortedSetDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {outerInstance->field});
}
} // namespace org::apache::lucene::document