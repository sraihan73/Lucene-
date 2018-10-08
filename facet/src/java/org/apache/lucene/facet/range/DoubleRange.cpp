using namespace std;

#include "DoubleRange.h"

namespace org::apache::lucene::facet::range
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using NumericUtils = org::apache::lucene::util::NumericUtils;

DoubleRange::DoubleRange(const wstring &label, double minIn, bool minInclusive,
                         double maxIn, bool maxInclusive)
    : Range(label), min(minIn), max(maxIn)
{

  // TODO: if DoubleDocValuesField used
  // NumericUtils.doubleToSortableLong format (instead of
  // Double.doubleToRawLongBits) we could do comparisons
  // in long space

  if (isnan(minIn)) {
    throw invalid_argument(L"min cannot be NaN");
  }
  if (!minInclusive) {
    minIn = Math::nextUp(minIn);
  }

  if (isnan(maxIn)) {
    throw invalid_argument(L"max cannot be NaN");
  }
  if (!maxInclusive) {
    // Why no Math.nextDown?
    maxIn = Math::nextAfter(maxIn, -numeric_limits<double>::infinity());
  }

  if (minIn > maxIn) {
    failNoMatch();
  }
}

bool DoubleRange::accept(double value) { return value >= min && value <= max; }

shared_ptr<LongRange> DoubleRange::toLongRange()
{
  return make_shared<LongRange>(label, NumericUtils::doubleToSortableLong(min),
                                true, NumericUtils::doubleToSortableLong(max),
                                true);
}

wstring DoubleRange::toString()
{
  return L"DoubleRange(" + label + L": " + to_wstring(min) + L" to " +
         to_wstring(max) + L")";
}

DoubleRange::ValueSourceQuery::ValueSourceQuery(
    shared_ptr<DoubleRange> range, shared_ptr<Query> fastMatchQuery,
    shared_ptr<DoubleValuesSource> valueSource)
    : range(range), fastMatchQuery(fastMatchQuery), valueSource(valueSource)
{
}

bool DoubleRange::ValueSourceQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool DoubleRange::ValueSourceQuery::equalsTo(shared_ptr<ValueSourceQuery> other)
{
  return range->equals(other->range) &&
         Objects::equals(fastMatchQuery, other->fastMatchQuery) &&
         valueSource->equals(other->valueSource);
}

int DoubleRange::ValueSourceQuery::hashCode()
{
  return classHash() + 31 * Objects::hash(range, fastMatchQuery, valueSource);
}

wstring DoubleRange::ValueSourceQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"Filter(" + range->toString() + L")";
}

shared_ptr<Query> DoubleRange::ValueSourceQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  if (fastMatchQuery != nullptr) {
    shared_ptr<Query> *const fastMatchRewritten =
        fastMatchQuery->rewrite(reader);
    if (fastMatchRewritten != fastMatchQuery) {
      return make_shared<ValueSourceQuery>(range, fastMatchRewritten,
                                           valueSource);
    }
  }
  return Query::rewrite(reader);
}

shared_ptr<Weight>
DoubleRange::ValueSourceQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                            bool needsScores,
                                            float boost) 
{
  shared_ptr<Weight> *const fastMatchWeight =
      fastMatchQuery == nullptr
          ? nullptr
          : searcher->createWeight(fastMatchQuery, false, 1.0f);

  return make_shared<ConstantScoreWeightAnonymousInnerClass>(
      shared_from_this(), boost, fastMatchWeight);
}

DoubleRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<ValueSourceQuery> outerInstance, float boost,
        shared_ptr<Weight> fastMatchWeight)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->fastMatchWeight = fastMatchWeight;
}

shared_ptr<Scorer>
DoubleRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr int maxDoc = context->reader()->maxDoc();

  shared_ptr<DocIdSetIterator> *const approximation;
  if (fastMatchWeight == nullptr) {
    approximation = DocIdSetIterator::all(maxDoc);
  } else {
    shared_ptr<Scorer> s = fastMatchWeight->scorer(context);
    if (s == nullptr) {
      return nullptr;
    }
    approximation = s->begin();
  }

  shared_ptr<DoubleValues> *const values =
      outerInstance->valueSource->getValues(context, nullptr);
  shared_ptr<TwoPhaseIterator> *const twoPhase =
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                       approximation, values);
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          twoPhase);
}

DoubleRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<DocIdSetIterator> approximation,
        shared_ptr<DoubleValues> values)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->approximation = approximation;
  this->values = values;
}

bool DoubleRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  return values->advanceExact(approximation->docID()) &&
         outerInstance->outerInstance.range.accept(values->doubleValue());
}

float DoubleRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // TODO: use cost of range.accept()
}

bool DoubleRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return outerInstance->valueSource->isCacheable(ctx);
}

shared_ptr<Query>
DoubleRange::getQuery(shared_ptr<Query> fastMatchQuery,
                      shared_ptr<DoubleValuesSource> valueSource)
{
  return make_shared<ValueSourceQuery>(shared_from_this(), fastMatchQuery,
                                       valueSource);
}
} // namespace org::apache::lucene::facet::range