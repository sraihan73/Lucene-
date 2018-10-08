using namespace std;

#include "LongRange.h"

namespace org::apache::lucene::facet::range
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;

LongRange::LongRange(const wstring &label, int64_t minIn, bool minInclusive,
                     int64_t maxIn, bool maxInclusive)
    : Range(label), min(minIn), max(maxIn)
{

  if (!minInclusive) {
    if (minIn != numeric_limits<int64_t>::max()) {
      minIn++;
    } else {
      failNoMatch();
    }
  }

  if (!maxInclusive) {
    if (maxIn != numeric_limits<int64_t>::min()) {
      maxIn--;
    } else {
      failNoMatch();
    }
  }

  if (minIn > maxIn) {
    failNoMatch();
  }
}

bool LongRange::accept(int64_t value) { return value >= min && value <= max; }

wstring LongRange::toString()
{
  return L"LongRange(" + label + L": " + to_wstring(min) + L" to " +
         to_wstring(max) + L")";
}

LongRange::ValueSourceQuery::ValueSourceQuery(
    shared_ptr<LongRange> range, shared_ptr<Query> fastMatchQuery,
    shared_ptr<LongValuesSource> valueSource)
    : range(range), fastMatchQuery(fastMatchQuery), valueSource(valueSource)
{
}

bool LongRange::ValueSourceQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool LongRange::ValueSourceQuery::equalsTo(shared_ptr<ValueSourceQuery> other)
{
  return range->equals(other->range) &&
         Objects::equals(fastMatchQuery, other->fastMatchQuery) &&
         valueSource->equals(other->valueSource);
}

int LongRange::ValueSourceQuery::hashCode()
{
  return classHash() + 31 * Objects::hash(range, fastMatchQuery, valueSource);
}

wstring LongRange::ValueSourceQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"Filter(" + range->toString() + L")";
}

shared_ptr<Query> LongRange::ValueSourceQuery::rewrite(
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
LongRange::ValueSourceQuery::createWeight(shared_ptr<IndexSearcher> searcher,
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

LongRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<ValueSourceQuery> outerInstance, float boost,
        shared_ptr<Weight> fastMatchWeight)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->fastMatchWeight = fastMatchWeight;
}

shared_ptr<Scorer>
LongRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
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

  shared_ptr<LongValues> *const values =
      outerInstance->valueSource->getValues(context, nullptr);
  shared_ptr<TwoPhaseIterator> *const twoPhase =
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                       approximation, values);
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          twoPhase);
}

LongRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<DocIdSetIterator> approximation,
        shared_ptr<LongValues> values)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->approximation = approximation;
  this->values = values;
}

bool LongRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  return values->advanceExact(approximation->docID()) &&
         outerInstance->outerInstance.range.accept(values->longValue());
}

float LongRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // TODO: use cost of range.accept()
}

bool LongRange::ValueSourceQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return outerInstance->valueSource->isCacheable(ctx);
}

shared_ptr<Query> LongRange::getQuery(shared_ptr<Query> fastMatchQuery,
                                      shared_ptr<LongValuesSource> valueSource)
{
  return make_shared<ValueSourceQuery>(shared_from_this(), fastMatchQuery,
                                       valueSource);
}
} // namespace org::apache::lucene::facet::range