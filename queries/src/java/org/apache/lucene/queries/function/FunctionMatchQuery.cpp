using namespace std;

#include "FunctionMatchQuery.h"

namespace org::apache::lucene::queries::function
{
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

FunctionMatchQuery::FunctionMatchQuery(shared_ptr<DoubleValuesSource> source,
                                       function<bool(double)> &filter)
    : source(source), filter(filter)
{
}

wstring FunctionMatchQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"FunctionMatchQuery(" + source->toString() + L")";
}

shared_ptr<Weight>
FunctionMatchQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                 bool needsScores,
                                 float boost) 
{
  shared_ptr<DoubleValuesSource> vs = source->rewrite(searcher);
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost, vs);
}

FunctionMatchQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<FunctionMatchQuery> outerInstance, float boost,
        shared_ptr<DoubleValuesSource> vs)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->vs = vs;
}

shared_ptr<Scorer>
FunctionMatchQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<DoubleValues> values = vs->getValues(context, nullptr);
  shared_ptr<DocIdSetIterator> approximation =
      DocIdSetIterator::all(context->reader()->maxDoc());
  shared_ptr<TwoPhaseIterator> twoPhase =
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                       approximation, values);
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          twoPhase);
}

FunctionMatchQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<DocIdSetIterator> approximation,
        shared_ptr<DoubleValues> values)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->approximation = approximation;
}

bool FunctionMatchQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  return values->advanceExact(approximation->docID()) &&
         outerInstance->outerInstance.filter.test(values->doubleValue());
}

float FunctionMatchQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // TODO maybe DoubleValuesSource should have a matchCost?
}

bool FunctionMatchQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return outerInstance->source->isCacheable(ctx);
}

bool FunctionMatchQuery::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<FunctionMatchQuery> that =
      any_cast<std::shared_ptr<FunctionMatchQuery>>(o);
  return Objects::equals(source, that->source) &&
         Objects::equals(filter, that->filter);
}

int FunctionMatchQuery::hashCode() { return Objects::hash(source, filter); }
} // namespace org::apache::lucene::queries::function