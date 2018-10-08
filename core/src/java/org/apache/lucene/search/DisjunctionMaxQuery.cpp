using namespace std;

#include "DisjunctionMaxQuery.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;

DisjunctionMaxQuery::DisjunctionMaxQuery(
    shared_ptr<deque<std::shared_ptr<Query>>> disjuncts,
    float tieBreakerMultiplier)
    : disjuncts(disjuncts->toArray(
          std::deque<std::shared_ptr<Query>>(disjuncts->size()))),
      tieBreakerMultiplier(tieBreakerMultiplier)
{
  Objects::requireNonNull(disjuncts, L"deque of Querys must not be null");
}

shared_ptr<Iterator<std::shared_ptr<Query>>> DisjunctionMaxQuery::iterator()
{
  return getDisjuncts().begin();
}

deque<std::shared_ptr<Query>> DisjunctionMaxQuery::getDisjuncts()
{
  return Collections::unmodifiableList(Arrays::asList(disjuncts));
}

float DisjunctionMaxQuery::getTieBreakerMultiplier()
{
  return tieBreakerMultiplier;
}

DisjunctionMaxQuery::DisjunctionMaxWeight::DisjunctionMaxWeight(
    shared_ptr<DisjunctionMaxQuery> outerInstance,
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
    : Weight(DisjunctionMaxQuery::this), needsScores(needsScores),
      outerInstance(outerInstance)
{
  for (auto disjunctQuery : outerInstance->disjuncts) {
    weights.push_back(
        searcher->createWeight(disjunctQuery, needsScores, boost));
  }
}

void DisjunctionMaxQuery::DisjunctionMaxWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  for (auto weight : weights) {
    weight->extractTerms(terms);
  }
}

shared_ptr<Matches> DisjunctionMaxQuery::DisjunctionMaxWeight::matches(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  deque<std::shared_ptr<Matches>> mis = deque<std::shared_ptr<Matches>>();
  for (auto weight : weights) {
    shared_ptr<Matches> mi = weight->matches(context, doc);
    if (mi->size() > 0) {
      mis.push_back(mi);
    }
  }
  return Matches::fromSubMatches(mis);
}

shared_ptr<Scorer> DisjunctionMaxQuery::DisjunctionMaxWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  deque<std::shared_ptr<Scorer>> scorers = deque<std::shared_ptr<Scorer>>();
  for (auto w : weights) {
    // we will advance() subscorers
    shared_ptr<Scorer> subScorer = w->scorer(context);
    if (subScorer != nullptr) {
      scorers.push_back(subScorer);
    }
  }
  if (scorers.empty()) {
    // no sub-scorers had any documents
    return nullptr;
  } else if (scorers.size() == 1) {
    // only one sub-scorer in this segment
    return scorers[0];
  } else {
    return make_shared<DisjunctionMaxScorer>(
        shared_from_this(), outerInstance->tieBreakerMultiplier, scorers,
        needsScores);
  }
}

bool DisjunctionMaxQuery::DisjunctionMaxWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  if (weights.size() > TermInSetQuery::BOOLEAN_REWRITE_TERM_COUNT_THRESHOLD) {
    // Disallow caching large dismax queries to not encourage users
    // to build large dismax queries as a workaround to the fact that
    // we disallow caching large TermInSetQueries.
    return false;
  }
  for (auto w : weights) {
    if (w->isCacheable(ctx) == false) {
      return false;
    }
  }
  return true;
}

shared_ptr<Explanation> DisjunctionMaxQuery::DisjunctionMaxWeight::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  bool match = false;
  float max = -numeric_limits<float>::infinity(), sum = 0.0f;
  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();
  for (auto wt : weights) {
    shared_ptr<Explanation> e = wt->explain(context, doc);
    if (e->isMatch()) {
      match = true;
      subs.push_back(e);
      sum += e->getValue();
      max = max(max, e->getValue());
    }
  }
  if (match) {
    constexpr float score =
        max + (sum - max) * outerInstance->tieBreakerMultiplier;
    const wstring desc =
        outerInstance->tieBreakerMultiplier == 0.0f
            ? L"max of:"
            : L"max plus " + to_wstring(outerInstance->tieBreakerMultiplier) +
                  L" times others of:";
    return Explanation::match(score, desc, subs);
  } else {
    return Explanation::noMatch(L"No matching clause");
  }
}

shared_ptr<Weight>
DisjunctionMaxQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                  bool needsScores,
                                  float boost) 
{
  return make_shared<DisjunctionMaxWeight>(shared_from_this(), searcher,
                                           needsScores, boost);
}

shared_ptr<Query>
DisjunctionMaxQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  if (disjuncts.size() == 1) {
    return disjuncts[0];
  }

  if (tieBreakerMultiplier == 1.0f) {
    shared_ptr<BooleanQuery::Builder> builder =
        make_shared<BooleanQuery::Builder>();
    for (auto sub : disjuncts) {
      builder->add(sub, BooleanClause::Occur::SHOULD);
    }
    return builder->build();
  }

  bool actuallyRewritten = false;
  deque<std::shared_ptr<Query>> rewrittenDisjuncts =
      deque<std::shared_ptr<Query>>();
  for (auto sub : disjuncts) {
    shared_ptr<Query> rewrittenSub = sub->rewrite(reader);
    actuallyRewritten |= rewrittenSub != sub;
    rewrittenDisjuncts.push_back(rewrittenSub);
  }

  if (actuallyRewritten) {
    return make_shared<DisjunctionMaxQuery>(rewrittenDisjuncts,
                                            tieBreakerMultiplier);
  }

  return Query::rewrite(reader);
}

wstring DisjunctionMaxQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"(");
  for (int i = 0; i < disjuncts.size(); i++) {
    shared_ptr<Query> subquery = disjuncts[i];
    if (std::dynamic_pointer_cast<BooleanQuery>(subquery) !=
        nullptr) { // wrap sub-bools in parens
      buffer->append(L"(");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buffer->append(subquery->toString(field));
      buffer->append(L")");
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buffer->append(subquery->toString(field));
    }
    if (i != disjuncts.size() - 1) {
      buffer->append(L" | ");
    }
  }
  buffer->append(L")");
  if (tieBreakerMultiplier != 0.0f) {
    buffer->append(L"~");
    buffer->append(tieBreakerMultiplier);
  }
  return buffer->toString();
}

bool DisjunctionMaxQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool DisjunctionMaxQuery::equalsTo(shared_ptr<DisjunctionMaxQuery> other)
{
  return tieBreakerMultiplier == other->tieBreakerMultiplier &&
         Arrays::equals(disjuncts, other->disjuncts);
}

int DisjunctionMaxQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + Float::floatToIntBits(tieBreakerMultiplier);
  h = 31 * h + Arrays::hashCode(disjuncts);
  return h;
}
} // namespace org::apache::lucene::search