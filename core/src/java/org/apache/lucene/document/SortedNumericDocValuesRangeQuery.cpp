using namespace std;

#include "SortedNumericDocValuesRangeQuery.h"

namespace org::apache::lucene::document
{
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocValuesFieldExistsQuery =
    org::apache::lucene::search::DocValuesFieldExistsQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;

SortedNumericDocValuesRangeQuery::SortedNumericDocValuesRangeQuery(
    const wstring &field, int64_t lowerValue, int64_t upperValue)
    : field(Objects::requireNonNull(field)), lowerValue(lowerValue),
      upperValue(upperValue)
{
}

bool SortedNumericDocValuesRangeQuery::equals(any obj)
{
  if (sameClassAs(obj) == false) {
    return false;
  }
  shared_ptr<SortedNumericDocValuesRangeQuery> that =
      any_cast<std::shared_ptr<SortedNumericDocValuesRangeQuery>>(obj);
  return Objects::equals(field, that->field) &&
         lowerValue == that->lowerValue && upperValue == that->upperValue;
}

int SortedNumericDocValuesRangeQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + field.hashCode();
  h = 31 * h + Long::hashCode(lowerValue);
  h = 31 * h + Long::hashCode(upperValue);
  return h;
}

wstring SortedNumericDocValuesRangeQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  if (this->field == field == false) {
    b->append(this->field)->append(L":");
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return b->append(L"[")
      ->append(lowerValue)
      ->append(L" TO ")
      ->append(upperValue)
      ->append(L"]")
      ->toString();
}

shared_ptr<Query> SortedNumericDocValuesRangeQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  if (lowerValue == numeric_limits<int64_t>::min() &&
      upperValue == numeric_limits<int64_t>::max()) {
    return make_shared<DocValuesFieldExistsQuery>(field);
  }
  return Query::rewrite(reader);
}

shared_ptr<Weight> SortedNumericDocValuesRangeQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

SortedNumericDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<SortedNumericDocValuesRangeQuery> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

bool SortedNumericDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {outerInstance->field});
}

shared_ptr<Scorer>
SortedNumericDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    scorer(shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedNumericDocValues> values =
      outerInstance->getValues(context->reader(), outerInstance->field);
  if (values == nullptr) {
    return nullptr;
  }
  shared_ptr<NumericDocValues> *const singleton =
      DocValues::unwrapSingleton(values);
  shared_ptr<TwoPhaseIterator> *const iterator;
  if (singleton != nullptr) {
    iterator = make_shared<TwoPhaseIteratorAnonymousInnerClass>(
        shared_from_this(), singleton);
  } else {
    iterator = make_shared<TwoPhaseIteratorAnonymousInnerClass2>(
        shared_from_this(), values);
  }
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          iterator);
}

SortedNumericDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<NumericDocValues> singleton)
    : org::apache::lucene::search::TwoPhaseIterator(singleton)
{
  this->outerInstance = outerInstance;
  this->singleton = singleton;
}

bool SortedNumericDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  constexpr int64_t value = singleton->longValue();
  return value >= outerInstance->outerInstance.lowerValue &&
         value <= outerInstance->outerInstance.upperValue;
}

float SortedNumericDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 2; // 2 comparisons
}

SortedNumericDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass2::TwoPhaseIteratorAnonymousInnerClass2(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<SortedNumericDocValues> values)
    : org::apache::lucene::search::TwoPhaseIterator(values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

bool SortedNumericDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass2::matches() 
{
  for (int i = 0, count = values->docValueCount(); i < count; ++i) {
    constexpr int64_t value = values->nextValue();
    if (value < outerInstance->outerInstance.lowerValue) {
      continue;
    }
    // Values are sorted, so the first value that is >= lowerValue is our best
    // candidate
    return value <= outerInstance->outerInstance.upperValue;
  }
  return false; // all values were < lowerValue
}

float SortedNumericDocValuesRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass2::matchCost()
{
  return 2; // 2 comparisons
}
} // namespace org::apache::lucene::document