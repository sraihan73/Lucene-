using namespace std;

#include "CustomScoreQuery.h"

namespace org::apache::lucene::queries
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using FunctionQuery = org::apache::lucene::queries::function::FunctionQuery;
using FunctionScoreQuery =
    org::apache::lucene::queries::function::FunctionScoreQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using FilterScorer = org::apache::lucene::search::FilterScorer;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

CustomScoreQuery::CustomScoreQuery(shared_ptr<Query> subQuery)
    : CustomScoreQuery(subQuery, new FunctionQuery[0])
{
}

CustomScoreQuery::CustomScoreQuery(shared_ptr<Query> subQuery,
                                   shared_ptr<FunctionQuery> scoringQuery)
    : CustomScoreQuery(subQuery, scoringQuery != nullptr
                                     ? new FunctionQuery[]{scoringQuery}
                                     : new FunctionQuery[0])
{
}

CustomScoreQuery::CustomScoreQuery(shared_ptr<Query> subQuery,
                                   deque<FunctionQuery> &scoringQueries)
{
  this->subQuery = subQuery;
  this->scoringQueries = scoringQueries.size() > 0
                             ? scoringQueries
                             : std::deque<std::shared_ptr<Query>>(0);
  if (subQuery == nullptr) {
    throw invalid_argument(L"<subquery> must not be null!");
  }
}

shared_ptr<Query>
CustomScoreQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<CustomScoreQuery> clone = nullptr;

  shared_ptr<Query> *const sq = subQuery->rewrite(reader);
  if (sq != subQuery) {
    clone = this->clone();
    clone->subQuery = sq;
  }

  for (int i = 0; i < scoringQueries.size(); i++) {
    shared_ptr<Query> *const v = scoringQueries[i]->rewrite(reader);
    if (v != scoringQueries[i]) {
      if (clone == nullptr) {
        clone = this->clone();
      }
      clone->scoringQueries[i] = v;
    }
  }

  return (clone == nullptr) ? shared_from_this() : clone;
}

shared_ptr<CustomScoreQuery> CustomScoreQuery::clone()
{
  shared_ptr<CustomScoreQuery> clone;
  try {
    clone = std::static_pointer_cast<CustomScoreQuery>(Query::clone());
  } catch (const CloneNotSupportedException &bogus) {
    // cannot happen
    throw make_shared<Error>(bogus);
  }
  clone->subQuery = subQuery;
  clone->scoringQueries =
      std::deque<std::shared_ptr<Query>>(scoringQueries.size());
  for (int i = 0; i < scoringQueries.size(); i++) {
    clone->scoringQueries[i] = scoringQueries[i];
  }
  return clone;
}

wstring CustomScoreQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> sb =
      (make_shared<StringBuilder>(name()))->append(L"(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  sb->append(subQuery->toString(field));
  for (auto scoringQuery : scoringQueries) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(L", ")->append(scoringQuery->toString(field));
  }
  sb->append(L")");
  return sb->toString();
}

bool CustomScoreQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool CustomScoreQuery::equalsTo(shared_ptr<CustomScoreQuery> other)
{
  return subQuery->equals(other->subQuery) &&
         scoringQueries.size() == other->scoringQueries.size() &&
         Arrays::equals(scoringQueries, other->scoringQueries);
}

int CustomScoreQuery::hashCode()
{
  // Didn't change this hashcode, but it looks suspicious.
  return (classHash() + subQuery->hashCode() +
          Arrays::hashCode(scoringQueries));
}

shared_ptr<CustomScoreProvider> CustomScoreQuery::getCustomScoreProvider(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<CustomScoreProvider>(context);
}

CustomScoreQuery::CustomWeight::CustomWeight(
    shared_ptr<CustomScoreQuery> outerInstance,
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
    : org::apache::lucene::search::Weight(CustomScoreQuery::this),
      subQueryWeight(
          outerInstance->subQuery->createWeight(searcher, needsScores, 1.0f)),
      valSrcWeights(std::deque<std::shared_ptr<Weight>>(
          outerInstance->scoringQueries.size())),
      queryWeight(boost), outerInstance(outerInstance)
{
  // note we DONT incorporate our boost, nor pass down any boost
  // (e.g. from outer BQ), as there is no guarantee that the
  // CustomScoreProvider's function obeys the distributive law... it might call
  // sqrt() on the subQuery score or some other arbitrary function other than
  // multiplication. so, instead boosts are applied directly in score()
  for (int i = 0; i < outerInstance->scoringQueries.size(); i++) {
    this->valSrcWeights[i] = outerInstance->scoringQueries[i]->createWeight(
        searcher, needsScores, 1.0f);
  }
}

void CustomScoreQuery::CustomWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  subQueryWeight->extractTerms(terms);
  for (auto scoringWeight : valSrcWeights) {
    scoringWeight->extractTerms(terms);
  }
}

shared_ptr<Scorer> CustomScoreQuery::CustomWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Scorer> subQueryScorer = subQueryWeight->scorer(context);
  if (subQueryScorer == nullptr) {
    return nullptr;
  }
  std::deque<std::shared_ptr<Scorer>> valSrcScorers(valSrcWeights.size());
  for (int i = 0; i < valSrcScorers.size(); i++) {
    valSrcScorers[i] = valSrcWeights[i]->scorer(context);
  }
  return make_shared<CustomScorer>(
      outerInstance->getCustomScoreProvider(context), shared_from_this(),
      queryWeight, subQueryScorer, valSrcScorers);
}

bool CustomScoreQuery::CustomWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  if (subQueryWeight->isCacheable(ctx) == false) {
    return false;
  }
  for (auto w : valSrcWeights) {
    if (w->isCacheable(ctx) == false) {
      return false;
    }
  }
  return true;
}

shared_ptr<Explanation>
CustomScoreQuery::CustomWeight::explain(shared_ptr<LeafReaderContext> context,
                                        int doc) 
{
  shared_ptr<Explanation> explain = doExplain(context, doc);
  return explain == nullptr ? Explanation::noMatch(L"no matching docs")
                            : explain;
}

shared_ptr<Explanation>
CustomScoreQuery::CustomWeight::doExplain(shared_ptr<LeafReaderContext> info,
                                          int doc) 
{
  shared_ptr<Explanation> subQueryExpl = subQueryWeight->explain(info, doc);
  if (!subQueryExpl->isMatch()) {
    return subQueryExpl;
  }
  // match
  std::deque<std::shared_ptr<Explanation>> valSrcExpls(valSrcWeights.size());
  for (int i = 0; i < valSrcWeights.size(); i++) {
    valSrcExpls[i] = valSrcWeights[i]->explain(info, doc);
  }
  shared_ptr<Explanation> customExp =
      outerInstance->getCustomScoreProvider(info)->customExplain(
          doc, subQueryExpl, valSrcExpls);
  float sc = queryWeight * customExp->getValue();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Explanation::match(
      sc, outerInstance->toString() + L", product of:",
      {customExp, Explanation::match(queryWeight, L"queryWeight")});
}

CustomScoreQuery::CustomScorer::CustomScorer(
    shared_ptr<CustomScoreProvider> provider, shared_ptr<CustomWeight> w,
    float qWeight, shared_ptr<Scorer> subQueryScorer,
    std::deque<std::shared_ptr<Scorer>> &valSrcScorers)
    : org::apache::lucene::search::FilterScorer(subQueryScorer, w),
      qWeight(qWeight), subQueryScorer(subQueryScorer),
      valSrcScorers(valSrcScorers), provider(provider),
      vScores(std::deque<float>(valSrcScorers.size()))
{
}

float CustomScoreQuery::CustomScorer::score() 
{
  // lazily advance to current doc.
  int doc = docID();
  if (doc > valSrcDocID) {
    for (auto valSrcScorer : valSrcScorers) {
      valSrcScorer->begin().advance(doc);
    }
    valSrcDocID = doc;
  }
  // TODO: this thing technically takes any Query, so what about when subs don't
  // match?
  for (int i = 0; i < valSrcScorers.size(); i++) {
    vScores[i] = valSrcScorers[i]->score();
  }
  return qWeight * provider->customScore(subQueryScorer->docID(),
                                         subQueryScorer->score(), vScores);
}

shared_ptr<deque<std::shared_ptr<Scorer::ChildScorer>>>
CustomScoreQuery::CustomScorer::getChildren()
{
  return Collections::singleton(
      make_shared<Scorer::ChildScorer>(subQueryScorer, L"CUSTOM"));
}

shared_ptr<Weight>
CustomScoreQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                               bool needsScores, float boost) 
{
  return make_shared<CustomWeight>(shared_from_this(), searcher, needsScores,
                                   boost);
}

shared_ptr<Query> CustomScoreQuery::getSubQuery() { return subQuery; }

std::deque<std::shared_ptr<Query>> CustomScoreQuery::getScoringQueries()
{
  return scoringQueries;
}

wstring CustomScoreQuery::name() { return L"custom"; }
} // namespace org::apache::lucene::queries