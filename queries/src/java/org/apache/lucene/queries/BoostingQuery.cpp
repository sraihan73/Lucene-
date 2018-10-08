using namespace std;

#include "BoostingQuery.h"

namespace org::apache::lucene::queries
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using FilterScorer = org::apache::lucene::search::FilterScorer;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;

BoostingQuery::BoostingQuery(shared_ptr<Query> match, shared_ptr<Query> context,
                             float boost)
    : contextBoost(boost), match(match),
      context(context) / *ignore context - only matches * /
{
}

shared_ptr<Query>
BoostingQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> matchRewritten = match->rewrite(reader);
  shared_ptr<Query> contextRewritten = context->rewrite(reader);
  if (match != matchRewritten || context != contextRewritten) {
    return make_shared<BoostingQuery>(matchRewritten, contextRewritten,
                                      contextBoost);
  }
  return Query::rewrite(reader);
}

shared_ptr<Weight>
BoostingQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                            bool needsScores, float boost) 
{
  if (needsScores == false) {
    return match->createWeight(searcher, needsScores, boost);
  }
  shared_ptr<Weight> *const matchWeight =
      searcher->createWeight(match, needsScores, boost);
  shared_ptr<Weight> *const contextWeight =
      searcher->createWeight(context, false, boost);
  return make_shared<WeightAnonymousInnerClass>(shared_from_this(), boost,
                                                matchWeight, contextWeight);
}

BoostingQuery::WeightAnonymousInnerClass::WeightAnonymousInnerClass(
    shared_ptr<BoostingQuery> outerInstance, float boost,
    shared_ptr<Weight> matchWeight, shared_ptr<Weight> contextWeight)
    : org::apache::lucene::search::Weight(outerInstance)
{
  this->outerInstance = outerInstance;
  this->boost = boost;
  this->matchWeight = matchWeight;
  this->contextWeight = contextWeight;
}

void BoostingQuery::WeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  matchWeight->extractTerms(terms);
  if (boost >= 1) {
    contextWeight->extractTerms(terms);
  }
}

shared_ptr<Explanation> BoostingQuery::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<Explanation> *const matchExplanation =
      matchWeight->explain(context, doc);
  shared_ptr<Explanation> *const contextExplanation =
      contextWeight->explain(context, doc);
  if (matchExplanation->isMatch() == false ||
      contextExplanation->isMatch() == false) {
    return matchExplanation;
  }
  return Explanation::match(
      matchExplanation->getValue() * outerInstance->contextBoost,
      L"product of:",
      {matchExplanation,
       Explanation::match(outerInstance->contextBoost, L"boost")});
}

shared_ptr<Scorer> BoostingQuery::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Scorer> *const matchScorer = matchWeight->scorer(context);
  if (matchScorer == nullptr) {
    return nullptr;
  }
  shared_ptr<Scorer> *const contextScorer = contextWeight->scorer(context);
  if (contextScorer == nullptr) {
    return matchScorer;
  }
  shared_ptr<TwoPhaseIterator> contextTwoPhase =
      contextScorer->twoPhaseIterator();
  shared_ptr<DocIdSetIterator> contextApproximation =
      contextTwoPhase == nullptr ? contextScorer->begin()
                                 : contextTwoPhase->approximation();
  return make_shared<FilterScorerAnonymousInnerClass>(
      shared_from_this(), contextTwoPhase, contextApproximation);
}

BoostingQuery::WeightAnonymousInnerClass::FilterScorerAnonymousInnerClass::
    FilterScorerAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance,
        shared_ptr<TwoPhaseIterator> contextTwoPhase,
        shared_ptr<DocIdSetIterator> contextApproximation)
    : org::apache::lucene::search::FilterScorer(matchScorer)
{
  this->outerInstance = outerInstance;
  this->contextTwoPhase = contextTwoPhase;
  this->contextApproximation = contextApproximation;
}

float BoostingQuery::WeightAnonymousInnerClass::
    FilterScorerAnonymousInnerClass::score() 
{
  if (contextApproximation->docID() < docID()) {
    contextApproximation->advance(docID());
  }
  assert(contextApproximation->docID() >= docID());
  float score = outerInstance->outerInstance.super.score();
  if (contextApproximation->docID() == docID() &&
      (contextTwoPhase == nullptr || contextTwoPhase->matches())) {
    score *= outerInstance->outerInstance.contextBoost;
  }
  return score;
}

bool BoostingQuery::WeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return matchWeight->isCacheable(ctx) && contextWeight->isCacheable(ctx);
}

shared_ptr<Query> BoostingQuery::getMatch() { return match; }

shared_ptr<Query> BoostingQuery::getContext() { return context; }

float BoostingQuery::getBoost() { return contextBoost; }

int BoostingQuery::hashCode()
{
  return 31 * classHash() + Objects::hash(match, context, contextBoost);
}

bool BoostingQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool BoostingQuery::equalsTo(shared_ptr<BoostingQuery> other)
{
  return match->equals(other->match) && context->equals(other->context) &&
         Float::floatToIntBits(contextBoost) ==
             Float::floatToIntBits(other->contextBoost);
}

wstring BoostingQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return match->toString(field) + L"/" + context->toString(field);
}
} // namespace org::apache::lucene::queries