using namespace std;

#include "Boolean2ScorerSupplier.h"

namespace org::apache::lucene::search
{
using Occur = org::apache::lucene::search::BooleanClause::Occur;

Boolean2ScorerSupplier::Boolean2ScorerSupplier(
    shared_ptr<BooleanWeight> weight,
    unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>> &subs,
    bool needsScores, int minShouldMatch)
    : weight(weight), subs(subs), needsScores(needsScores),
      minShouldMatch(minShouldMatch)
{
  if (minShouldMatch < 0) {
    throw invalid_argument(L"minShouldMatch must be positive, but got: " +
                           to_wstring(minShouldMatch));
  }
  if (minShouldMatch != 0 && minShouldMatch >= subs[Occur::SHOULD]->size()) {
    throw invalid_argument(L"minShouldMatch must be strictly less than the "
                           L"number of SHOULD clauses");
  }
  if (needsScores == false && minShouldMatch == 0 &&
      subs[Occur::SHOULD]->size() > 0 &&
      subs[Occur::MUST]->size() + subs[Occur::FILTER]->size() > 0) {
    throw invalid_argument(
        L"Cannot pass purely optional clauses if scores are not needed");
  }
  if (subs[Occur::SHOULD]->size() + subs[Occur::MUST]->size() +
          subs[Occur::FILTER]->size() ==
      0) {
    throw invalid_argument(L"There should be at least one positive clause");
  }
}

int64_t Boolean2ScorerSupplier::computeCost()
{
  std::optional<long> minRequiredCost =
      Stream::concat(subs[Occur::MUST]->stream(), subs[Occur::FILTER]->stream())
          .mapToLong(ScorerSupplier::cost)
          .min();
  if (minRequiredCost && minShouldMatch == 0) {
    return minRequiredCost.value();
  } else {
    shared_ptr<deque<std::shared_ptr<ScorerSupplier>>>
        *const optionalScorers = subs[Occur::SHOULD];
    constexpr int64_t shouldCost = MinShouldMatchSumScorer::cost(
        optionalScorers->stream().mapToLong(ScorerSupplier::cost),
        optionalScorers->size(), minShouldMatch);
    return min(minRequiredCost.value_or(numeric_limits<int64_t>::max()),
               shouldCost);
  }
}

int64_t Boolean2ScorerSupplier::cost()
{
  if (cost_ == -1) {
    cost_ = computeCost();
  }
  return cost_;
}

shared_ptr<Scorer>
Boolean2ScorerSupplier::get(int64_t leadCost) 
{
  // three cases: conjunction, disjunction, or mix
  leadCost = min(leadCost, cost());

  // pure conjunction
  if (subs[Occur::SHOULD]->isEmpty()) {
    return excl(req(subs[Occur::FILTER], subs[Occur::MUST], leadCost),
                subs[Occur::MUST_NOT], leadCost);
  }

  // pure disjunction
  if (subs[Occur::FILTER]->isEmpty() && subs[Occur::MUST]->isEmpty()) {
    return excl(opt(subs[Occur::SHOULD], minShouldMatch, needsScores, leadCost),
                subs[Occur::MUST_NOT], leadCost);
  }

  // conjunction-disjunction mix:
  // we create the required and optional pieces, and then
  // combine the two: if minNrShouldMatch > 0, then it's a conjunction: because
  // the optional side must match. otherwise it's required + optional

  if (minShouldMatch > 0) {
    shared_ptr<Scorer> req =
        excl(this->req(subs[Occur::FILTER], subs[Occur::MUST], leadCost),
             subs[Occur::MUST_NOT], leadCost);
    shared_ptr<Scorer> opt =
        this->opt(subs[Occur::SHOULD], minShouldMatch, needsScores, leadCost);
    return make_shared<ConjunctionScorer>(weight, Arrays::asList(req, opt),
                                          Arrays::asList(req, opt));
  } else {
    assert(needsScores);
    return make_shared<ReqOptSumScorer>(
        excl(req(subs[Occur::FILTER], subs[Occur::MUST], leadCost),
             subs[Occur::MUST_NOT], leadCost),
        opt(subs[Occur::SHOULD], minShouldMatch, needsScores, leadCost));
  }
}

shared_ptr<Scorer> Boolean2ScorerSupplier::req(
    shared_ptr<deque<std::shared_ptr<ScorerSupplier>>> requiredNoScoring,
    shared_ptr<deque<std::shared_ptr<ScorerSupplier>>> requiredScoring,
    int64_t leadCost) 
{
  if (requiredNoScoring->size() + requiredScoring->size() == 1) {
    shared_ptr<Scorer> req =
        (requiredNoScoring->isEmpty() ? requiredScoring : requiredNoScoring)
            ->begin()
            ->next()
            ->get(leadCost);

    if (needsScores == false) {
      return req;
    }

    if (requiredScoring->isEmpty()) {
      // Scores are needed but we only have a filter clause
      // BooleanWeight expects that calling score() is ok so we need to wrap
      // to prevent score() from being propagated
      return make_shared<FilterScorerAnonymousInnerClass>(shared_from_this(),
                                                          req);
    }

    return req;
  } else {
    deque<std::shared_ptr<Scorer>> requiredScorers =
        deque<std::shared_ptr<Scorer>>();
    deque<std::shared_ptr<Scorer>> scoringScorers =
        deque<std::shared_ptr<Scorer>>();
    for (auto s : requiredNoScoring) {
      requiredScorers.push_back(s->get(leadCost));
    }
    for (auto s : requiredScoring) {
      shared_ptr<Scorer> scorer = s->get(leadCost);
      requiredScorers.push_back(scorer);
      scoringScorers.push_back(scorer);
    }
    return make_shared<ConjunctionScorer>(weight, requiredScorers,
                                          scoringScorers);
  }
}

Boolean2ScorerSupplier::FilterScorerAnonymousInnerClass::
    FilterScorerAnonymousInnerClass(
        shared_ptr<Boolean2ScorerSupplier> outerInstance,
        shared_ptr<org::apache::lucene::search::Scorer> req)
    : FilterScorer(req)
{
  this->outerInstance = outerInstance;
}

float Boolean2ScorerSupplier::FilterScorerAnonymousInnerClass::score() throw(
    IOException)
{
  return 0.0f;
}

shared_ptr<Scorer> Boolean2ScorerSupplier::excl(
    shared_ptr<Scorer> main,
    shared_ptr<deque<std::shared_ptr<ScorerSupplier>>> prohibited,
    int64_t leadCost) 
{
  if (prohibited->isEmpty()) {
    return main;
  } else {
    return make_shared<ReqExclScorer>(main,
                                      opt(prohibited, 1, false, leadCost));
  }
}

shared_ptr<Scorer> Boolean2ScorerSupplier::opt(
    shared_ptr<deque<std::shared_ptr<ScorerSupplier>>> optional,
    int minShouldMatch, bool needsScores, int64_t leadCost) 
{
  if (optional->size() == 1) {
    return optional->begin()->next()->get(leadCost);
  } else {
    const deque<std::shared_ptr<Scorer>> optionalScorers =
        deque<std::shared_ptr<Scorer>>();
    for (auto scorer : optional) {
      optionalScorers.push_back(scorer->get(leadCost));
    }
    if (minShouldMatch > 1) {
      return make_shared<MinShouldMatchSumScorer>(weight, optionalScorers,
                                                  minShouldMatch);
    } else {
      return make_shared<DisjunctionSumScorer>(weight, optionalScorers,
                                               needsScores);
    }
  }
}
} // namespace org::apache::lucene::search