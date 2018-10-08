using namespace std;

#include "DrillSidewaysQuery.h"

namespace org::apache::lucene::facet
{
using DocsAndCost =
    org::apache::lucene::facet::DrillSidewaysScorer::DocsAndCost;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using BulkScorer = org::apache::lucene::search::BulkScorer;
using Collector = org::apache::lucene::search::Collector;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

DrillSidewaysQuery::DrillSidewaysQuery(
    shared_ptr<Query> baseQuery, shared_ptr<Collector> drillDownCollector,
    std::deque<std::shared_ptr<Collector>> &drillSidewaysCollectors,
    std::deque<std::shared_ptr<Query>> &drillDownQueries,
    bool scoreSubDocsAtOnce)
    : baseQuery(Objects::requireNonNull(baseQuery)),
      drillDownCollector(drillDownCollector),
      drillSidewaysCollectors(drillSidewaysCollectors),
      drillDownQueries(drillDownQueries), scoreSubDocsAtOnce(scoreSubDocsAtOnce)
{
}

wstring DrillSidewaysQuery::toString(const wstring &field)
{
  return L"DrillSidewaysQuery";
}

shared_ptr<Query>
DrillSidewaysQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> newQuery = baseQuery;
  while (true) {
    shared_ptr<Query> rewrittenQuery = newQuery->rewrite(reader);
    if (rewrittenQuery == newQuery) {
      break;
    }
    newQuery = rewrittenQuery;
  }
  if (newQuery == baseQuery) {
    return Query::rewrite(reader);
  } else {
    return make_shared<DrillSidewaysQuery>(
        newQuery, drillDownCollector, drillSidewaysCollectors, drillDownQueries,
        scoreSubDocsAtOnce);
  }
}

shared_ptr<Weight>
DrillSidewaysQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                 bool needsScores,
                                 float boost) 
{
  shared_ptr<Weight> *const baseWeight =
      baseQuery->createWeight(searcher, needsScores, boost);
  std::deque<std::shared_ptr<Weight>> drillDowns(drillDownQueries.size());
  for (int dim = 0; dim < drillDownQueries.size(); dim++) {
    drillDowns[dim] = searcher->createWeight(
        searcher->rewrite(drillDownQueries[dim]), false, 1);
  }

  return make_shared<WeightAnonymousInnerClass>(
      shared_from_this(), DrillSidewaysQuery::this, baseWeight, drillDowns);
}

// C++ TODO: You cannot use 'shared_from_this' in a constructor:
DrillSidewaysQuery::WeightAnonymousInnerClass::WeightAnonymousInnerClass(
    shared_ptr<DrillSidewaysQuery> outerInstance,
    shared_ptr<org::apache::lucene::facet::DrillSidewaysQuery>
        shared_from_this(),
    shared_ptr<Weight> baseWeight, deque<std::shared_ptr<Weight>> &drillDowns)
    : org::apache::lucene::search::Weight(this)
{
  this->outerInstance = outerInstance;
  this->baseWeight = baseWeight;
  this->drillDowns = drillDowns;
}

void DrillSidewaysQuery::WeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

shared_ptr<Explanation> DrillSidewaysQuery::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  return baseWeight->explain(context, doc);
}

shared_ptr<Scorer> DrillSidewaysQuery::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  // We can only run as a top scorer:
  throw make_shared<UnsupportedOperationException>();
}

bool DrillSidewaysQuery::WeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  if (baseWeight->isCacheable(ctx) == false) {
    return false;
  }
  for (auto w : drillDowns) {
    if (w->isCacheable(ctx) == false) {
      return false;
    }
  }
  return true;
}

shared_ptr<BulkScorer>
DrillSidewaysQuery::WeightAnonymousInnerClass::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Scorer> baseScorer = baseWeight->scorer(context);

  std::deque<std::shared_ptr<DrillSidewaysScorer::DocsAndCost>> dims(
      drillDowns.size());
  int nullCount = 0;
  for (int dim = 0; dim < dims.size(); dim++) {
    shared_ptr<Scorer> scorer = drillDowns[dim]->scorer(context);
    if (scorer == nullptr) {
      nullCount++;
      scorer = make_shared<ConstantScoreScorer>(drillDowns[dim], 0.0f,
                                                DocIdSetIterator::empty());
    }

    dims[dim] = make_shared<DrillSidewaysScorer::DocsAndCost>(
        scorer, outerInstance->drillSidewaysCollectors[dim]);
  }

  // If more than one dim has no matches, then there
  // are no hits nor drill-sideways counts.  Or, if we
  // have only one dim and that dim has no matches,
  // same thing.
  // if (nullCount > 1 || (nullCount == 1 && dims.length == 1)) {
  if (nullCount > 1) {
    return nullptr;
  }

  // Sort drill-downs by most restrictive first:
  Arrays::sort(dims,
               make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  if (baseScorer == nullptr) {
    return nullptr;
  }

  return make_shared<DrillSidewaysScorer>(
      context, baseScorer, outerInstance->drillDownCollector, dims,
      outerInstance->scoreSubDocsAtOnce);
}

DrillSidewaysQuery::WeightAnonymousInnerClass::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

int DrillSidewaysQuery::WeightAnonymousInnerClass::
    ComparatorAnonymousInnerClass::compare(shared_ptr<DocsAndCost> o1,
                                           shared_ptr<DocsAndCost> o2)
{
  return Long::compare(o1->approximation->cost(), o2->approximation->cost());
}

int DrillSidewaysQuery::hashCode()
{
  constexpr int prime = 31;
  int result = classHash();
  result = prime * result + Objects::hashCode(baseQuery);
  result = prime * result + Objects::hashCode(drillDownCollector);
  result = prime * result + Arrays::hashCode(drillDownQueries);
  result = prime * result + Arrays::hashCode(drillSidewaysCollectors);
  return result;
}

bool DrillSidewaysQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool DrillSidewaysQuery::equalsTo(shared_ptr<DrillSidewaysQuery> other)
{
  return Objects::equals(baseQuery, other->baseQuery) &&
         Objects::equals(drillDownCollector, other->drillDownCollector) &&
         Arrays::equals(drillDownQueries, other->drillDownQueries) &&
         Arrays::equals(drillSidewaysCollectors,
                        other->drillSidewaysCollectors);
}
} // namespace org::apache::lucene::facet