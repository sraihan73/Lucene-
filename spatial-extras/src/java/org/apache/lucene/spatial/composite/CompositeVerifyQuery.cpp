using namespace std;

#include "CompositeVerifyQuery.h"

namespace org::apache::lucene::spatial::composite
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using ShapeValuesPredicate =
    org::apache::lucene::spatial::util::ShapeValuesPredicate;

CompositeVerifyQuery::CompositeVerifyQuery(
    shared_ptr<Query> indexQuery,
    shared_ptr<ShapeValuesPredicate> predicateValueSource)
    : indexQuery(indexQuery), predicateValueSource(predicateValueSource)
{
}

shared_ptr<Query>
CompositeVerifyQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> *const rewritten = indexQuery->rewrite(reader);
  if (rewritten != indexQuery) {
    return make_shared<CompositeVerifyQuery>(rewritten, predicateValueSource);
  }
  return Query::rewrite(reader);
}

bool CompositeVerifyQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool CompositeVerifyQuery::equalsTo(shared_ptr<CompositeVerifyQuery> other)
{
  return indexQuery->equals(other->indexQuery) &&
         predicateValueSource->equals(other->predicateValueSource);
}

int CompositeVerifyQuery::hashCode()
{
  int result = classHash();
  result = 31 * result + indexQuery->hashCode();
  result = 31 * result + predicateValueSource->hashCode();
  return result;
}

wstring CompositeVerifyQuery::toString(const wstring &field)
{
  // TODO verify this looks good
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + indexQuery->toString(field) +
         L", " + predicateValueSource + L")";
}

shared_ptr<Weight>
CompositeVerifyQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                   bool needsScores,
                                   float boost) 
{
  shared_ptr<Weight> *const indexQueryWeight = indexQuery->createWeight(
      searcher, false, boost); // scores aren't unsupported

  return make_shared<ConstantScoreWeightAnonymousInnerClass>(
      shared_from_this(), boost, indexQueryWeight);
}

CompositeVerifyQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<CompositeVerifyQuery> outerInstance, float boost,
        shared_ptr<Weight> indexQueryWeight)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->indexQueryWeight = indexQueryWeight;
}

shared_ptr<Scorer>
CompositeVerifyQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{

  shared_ptr<Scorer> *const indexQueryScorer =
      indexQueryWeight->scorer(context);
  if (indexQueryScorer == nullptr) {
    return nullptr;
  }

  shared_ptr<TwoPhaseIterator> *const predFuncValues =
      outerInstance->predicateValueSource->iterator(context,
                                                    indexQueryScorer->begin());
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          predFuncValues);
}

bool CompositeVerifyQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return outerInstance->predicateValueSource->isCacheable(ctx);
}
} // namespace org::apache::lucene::spatial::composite