using namespace std;

#include "CoveringQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

CoveringQuery::CoveringQuery(
    shared_ptr<deque<std::shared_ptr<Query>>> queries,
    shared_ptr<LongValuesSource> minimumNumberMatch)
    : queries(make_shared<Multiset<>>()),
      minimumNumberMatch(Objects::requireNonNull(minimumNumberMatch)),
      hashCode(computeHashCode())
{
  if (queries->size() > BooleanQuery::getMaxClauseCount()) {
    throw make_shared<BooleanQuery::TooManyClauses>();
  }
  if (minimumNumberMatch->needsScores()) {
    throw invalid_argument(
        L"The minimum number of matches may not depend on the score.");
  }
  this->queries->addAll(queries);
}

wstring CoveringQuery::toString(const wstring &field)
{
  wstring queriesToString = queries->stream()
                                .map_obj([&](any q) { q->toString(field); })
                                .sorted()
                                .collect(Collectors::joining(L", "));
  return L"CoveringQuery(queries=[" + queriesToString +
         L"], minimumNumberMatch=" + minimumNumberMatch + L")";
}

bool CoveringQuery::equals(any obj)
{
  if (sameClassAs(obj) == false) {
    return false;
  }
  shared_ptr<CoveringQuery> that =
      any_cast<std::shared_ptr<CoveringQuery>>(obj);
  return hashCode_ == that->hashCode_ &&
         Objects::equals(queries, that->queries) &&
         Objects::equals(minimumNumberMatch, that->minimumNumberMatch);
}

int CoveringQuery::computeHashCode()
{
  int h = classHash();
  h = 31 * h + queries->hashCode();
  h = 31 * h + minimumNumberMatch->hashCode();
  return h;
}

int CoveringQuery::hashCode() { return hashCode_; }

shared_ptr<Query>
CoveringQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Multiset<std::shared_ptr<Query>>> rewritten =
      make_shared<Multiset<std::shared_ptr<Query>>>();
  bool actuallyRewritten = false;
  for (auto query : queries) {
    shared_ptr<Query> r = query->rewrite(reader);
    rewritten->add(r);
    actuallyRewritten |= query != r;
  }
  if (actuallyRewritten) {
    return make_shared<CoveringQuery>(rewritten, minimumNumberMatch);
  }
  return Query::rewrite(reader);
}

shared_ptr<Weight>
CoveringQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                            bool needsScores, float boost) 
{
  const deque<std::shared_ptr<Weight>> weights =
      deque<std::shared_ptr<Weight>>(queries->size());
  for (auto query : queries) {
    weights.push_back(searcher->createWeight(query, needsScores, boost));
  }
  return make_shared<CoveringWeight>(shared_from_this(), weights,
                                     minimumNumberMatch->rewrite(searcher));
}

CoveringQuery::CoveringWeight::CoveringWeight(
    shared_ptr<Query> query,
    shared_ptr<deque<std::shared_ptr<Weight>>> weights,
    shared_ptr<LongValuesSource> minimumNumberMatch)
    : Weight(query), weights(weights), minimumNumberMatch(minimumNumberMatch)
{
}

void CoveringQuery::CoveringWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  for (auto weight : weights) {
    weight->extractTerms(terms);
  }
}

shared_ptr<Matches>
CoveringQuery::CoveringWeight::matches(shared_ptr<LeafReaderContext> context,
                                       int doc) 
{
  shared_ptr<LongValues> minMatchValues =
      minimumNumberMatch->getValues(context, nullptr);
  if (minMatchValues->advanceExact(doc) == false) {
    return nullptr;
  }
  constexpr int64_t minimumNumberMatch = max(1, minMatchValues->longValue());
  int64_t matchCount = 0;
  deque<std::shared_ptr<Matches>> subMatches =
      deque<std::shared_ptr<Matches>>();
  for (auto weight : weights) {
    shared_ptr<Matches> matches = weight->matches(context, doc);
    if (matches->size() > 0) {
      matchCount++;
      subMatches.push_back(matches);
    }
  }
  if (matchCount < minimumNumberMatch) {
    return nullptr;
  }
  return Matches::fromSubMatches(subMatches);
}

shared_ptr<Explanation>
CoveringQuery::CoveringWeight::explain(shared_ptr<LeafReaderContext> context,
                                       int doc) 
{
  shared_ptr<LongValues> minMatchValues =
      minimumNumberMatch->getValues(context, nullptr);
  if (minMatchValues->advanceExact(doc) == false) {
    return Explanation::noMatch(
        L"minimumNumberMatch has no value on the current document");
  }
  constexpr int64_t minimumNumberMatch = max(1, minMatchValues->longValue());
  int freq = 0;
  double score = 0;
  deque<std::shared_ptr<Explanation>> subExpls =
      deque<std::shared_ptr<Explanation>>();
  for (auto weight : weights) {
    shared_ptr<Explanation> subExpl = weight->explain(context, doc);
    if (subExpl->isMatch()) {
      freq++;
      score += subExpl->getValue();
    }
    subExpls.push_back(subExpl);
  }
  if (freq >= minimumNumberMatch) {
    return Explanation::match(static_cast<float>(score),
                              to_wstring(freq) + L" matches for " +
                                  to_wstring(minimumNumberMatch) +
                                  L" required matches, sum of:",
                              subExpls);
  } else {
    return Explanation::noMatch(to_wstring(freq) + L" matches for " +
                                    to_wstring(minimumNumberMatch) +
                                    L" required matches",
                                subExpls);
  }
}

shared_ptr<Scorer> CoveringQuery::CoveringWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<deque<std::shared_ptr<Scorer>>> scorers =
      deque<std::shared_ptr<Scorer>>();
  for (auto w : weights) {
    shared_ptr<Scorer> s = w->scorer(context);
    if (s != nullptr) {
      scorers->add(s);
    }
  }
  if (scorers->isEmpty()) {
    return nullptr;
  }
  return make_shared<CoveringScorer>(
      shared_from_this(), scorers,
      minimumNumberMatch->getValues(context, nullptr),
      context->reader()->maxDoc());
}

bool CoveringQuery::CoveringWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return minimumNumberMatch->isCacheable(ctx);
}
} // namespace org::apache::lucene::search