using namespace std;

#include "IntervalQuery.h"

namespace org::apache::lucene::search::intervals
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Weight = org::apache::lucene::search::Weight;
using Similarity = org::apache::lucene::search::similarities::Similarity;

IntervalQuery::IntervalQuery(const wstring &field,
                             shared_ptr<IntervalsSource> intervalsSource)
    : field(field), intervalsSource(intervalsSource)
{
}

wstring IntervalQuery::getField() { return field; }

wstring IntervalQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return intervalsSource->toString();
}

shared_ptr<Weight>
IntervalQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                            bool needsScores, float boost) 
{
  return make_shared<IntervalWeight>(
      shared_from_this(), shared_from_this(),
      needsScores ? buildSimScorer(searcher, needsScores, boost) : nullptr,
      searcher->getSimilarity(needsScores), needsScores);
}

shared_ptr<Similarity::SimWeight>
IntervalQuery::buildSimScorer(shared_ptr<IndexSearcher> searcher,
                              bool needsScores, float boost) 
{
  shared_ptr<Set<std::shared_ptr<Term>>> terms =
      unordered_set<std::shared_ptr<Term>>();
  intervalsSource->extractTerms(field, terms);
  std::deque<std::shared_ptr<TermStatistics>> termStats(terms->size());
  int termUpTo = 0;
  for (auto term : terms) {
    shared_ptr<TermStatistics> termStatistics = searcher->termStatistics(
        term, TermContext::build(searcher->getTopReaderContext(), term));
    if (termStatistics != nullptr) {
      termStats[termUpTo++] = termStatistics;
    }
  }
  if (termUpTo == 0) {
    return nullptr;
  }
  shared_ptr<CollectionStatistics> collectionStats =
      searcher->collectionStatistics(field);
  return searcher->getSimilarity(needsScores)
      ->computeWeight(boost, collectionStats,
                      {Arrays::copyOf(termStats, termUpTo)});
}

bool IntervalQuery::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<IntervalQuery> that = any_cast<std::shared_ptr<IntervalQuery>>(o);
  return Objects::equals(field, that->field) &&
         Objects::equals(intervalsSource, that->intervalsSource);
}

int IntervalQuery::hashCode() { return Objects::hash(field, intervalsSource); }

IntervalQuery::IntervalWeight::IntervalWeight(
    shared_ptr<IntervalQuery> outerInstance, shared_ptr<Query> query,
    shared_ptr<Similarity::SimWeight> simWeight,
    shared_ptr<Similarity> similarity, bool needsScores)
    : org::apache::lucene::search::Weight(query), simWeight(simWeight),
      similarity(similarity), needsScores(needsScores),
      outerInstance(outerInstance)
{
}

void IntervalQuery::IntervalWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  outerInstance->intervalsSource->extractTerms(outerInstance->field, terms);
}

shared_ptr<Explanation>
IntervalQuery::IntervalWeight::explain(shared_ptr<LeafReaderContext> context,
                                       int doc) 
{
  shared_ptr<IntervalScorer> scorer =
      std::static_pointer_cast<IntervalScorer>(this->scorer(context));
  if (scorer != nullptr) {
    int newDoc = scorer->begin().advance(doc);
    if (newDoc == doc) {
      return scorer->explain(L"weight(" + getQuery() + L" in " +
                             to_wstring(doc) + L") [" +
                             similarity->getClass().getSimpleName() + L"]");
    }
  }
  return Explanation::noMatch(L"no matching intervals");
}

shared_ptr<Scorer> IntervalQuery::IntervalWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<IntervalIterator> intervals =
      outerInstance->intervalsSource->intervals(outerInstance->field, context);
  if (intervals == nullptr) {
    return nullptr;
  }
  shared_ptr<Similarity::SimScorer> leafScorer =
      simWeight == nullptr ? nullptr
                           : similarity->simScorer(simWeight, context);
  return make_shared<IntervalScorer>(shared_from_this(), intervals, leafScorer);
}

bool IntervalQuery::IntervalWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}
} // namespace org::apache::lucene::search::intervals