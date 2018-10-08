using namespace std;

#include "BooleanWeight.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Bits = org::apache::lucene::util::Bits;

BooleanWeight::BooleanWeight(shared_ptr<BooleanQuery> query,
                             shared_ptr<IndexSearcher> searcher,
                             bool needsScores, float boost) 
    : Weight(query), similarity(searcher->getSimilarity(needsScores)),
      query(query), weights(deque<>()), needsScores(needsScores)
{
  for (auto c : query) {
    shared_ptr<Weight> w = searcher->createWeight(
        c->getQuery(), needsScores && c->isScoring(), boost);
    weights.push_back(w);
  }
}

void BooleanWeight::extractTerms(shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  int i = 0;
  for (auto clause : query) {
    if (clause->isScoring() ||
        (needsScores == false && clause->isProhibited() == false)) {
      weights[i]->extractTerms(terms);
    }
    i++;
  }
}

shared_ptr<Explanation>
BooleanWeight::explain(shared_ptr<LeafReaderContext> context,
                       int doc) 
{
  constexpr int minShouldMatch = query->getMinimumNumberShouldMatch();
  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();
  float sum = 0.0f;
  bool fail = false;
  int matchCount = 0;
  int shouldMatchCount = 0;
  org::apache::lucene::search::BooleanQuery::const_iterator cIter =
      query->begin();
  for (deque<std::shared_ptr<Weight>>::const_iterator wIter = weights.begin();
       wIter != weights.end(); ++wIter) {
    shared_ptr<Weight> w = *wIter;
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<BooleanClause> c = cIter->next();
    shared_ptr<Explanation> e = w->explain(context, doc);
    if (e->isMatch()) {
      if (c->isScoring()) {
        subs.push_back(e);
        sum += e->getValue();
      } else if (c->isRequired()) {
        subs.push_back(Explanation::match(
            0.0f, L"match on required clause, product of:",
            {Explanation::match(0.0f, Occur::FILTER + L" clause"), e}));
      } else if (c->isProhibited()) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        subs.push_back(Explanation::noMatch(
            L"match on prohibited clause (" + c->getQuery()->toString() + L")",
            e));
        fail = true;
      }
      if (!c->isProhibited()) {
        matchCount++;
      }
      if (c->getOccur() == Occur::SHOULD) {
        shouldMatchCount++;
      }
    } else if (c->isRequired()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      subs.push_back(Explanation::noMatch(L"no match on required clause (" +
                                              c->getQuery()->toString() + L")",
                                          e));
      fail = true;
    }
  }
  if (fail) {
    return Explanation::noMatch(
        L"Failure to meet condition(s) of required/prohibited clause(s)", subs);
  } else if (matchCount == 0) {
    return Explanation::noMatch(L"No matching clauses", subs);
  } else if (shouldMatchCount < minShouldMatch) {
    return Explanation::noMatch(
        L"Failure to match minimum number of optional clauses: " +
            to_wstring(minShouldMatch),
        subs);
  } else {
    // we have a match
    return Explanation::match(sum, L"sum of:", subs);
  }
}

shared_ptr<Matches>
BooleanWeight::matches(shared_ptr<LeafReaderContext> context,
                       int doc) 
{
  constexpr int minShouldMatch = query->getMinimumNumberShouldMatch();
  deque<std::shared_ptr<Matches>> matches = deque<std::shared_ptr<Matches>>();
  int shouldMatchCount = 0;
  deque<std::shared_ptr<Weight>>::const_iterator wIt = weights.begin();
  Iterator<std::shared_ptr<BooleanClause>> cIt = query->clauses().begin();
  while (wIt != weights.end()) {
    shared_ptr<Weight> w = *wIt;
    shared_ptr<BooleanClause> bc = cIt->next();
    shared_ptr<Matches> m = w->matches(context, doc);
    if (bc->isProhibited()) {
      if (m->size() > 0) {
        return nullptr;
      }
    }
    if (bc->isRequired()) {
      if (m->empty()) {
        return nullptr;
      }
      matches.push_back(m);
    }
    if (bc->getOccur() == Occur::SHOULD) {
      if (m->size() > 0) {
        matches.push_back(m);
        shouldMatchCount++;
      }
    }
    wIt++;
  }
  if (shouldMatchCount < minShouldMatch) {
    return nullptr;
  }
  return Matches::fromSubMatches(matches);
}

shared_ptr<BulkScorer>
BooleanWeight::disableScoring(shared_ptr<BulkScorer> scorer)
{
  return make_shared<BulkScorerAnonymousInnerClass>(scorer);
}

BooleanWeight::BulkScorerAnonymousInnerClass::BulkScorerAnonymousInnerClass(
    shared_ptr<org::apache::lucene::search::BulkScorer> scorer)
{
  this->scorer = scorer;
}

int BooleanWeight::BulkScorerAnonymousInnerClass::score(
    shared_ptr<LeafCollector> collector, shared_ptr<Bits> acceptDocs, int min,
    int max) 
{
  shared_ptr<LeafCollector> *const noScoreCollector =
      make_shared<LeafCollectorAnonymousInnerClass>(shared_from_this(),
                                                    collector);
  return scorer->score(noScoreCollector, acceptDocs, min, max);
}

BooleanWeight::BulkScorerAnonymousInnerClass::LeafCollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass(
        shared_ptr<BulkScorerAnonymousInnerClass> outerInstance,
        shared_ptr<org::apache::lucene::search::LeafCollector> collector)
{
  this->outerInstance = outerInstance;
  this->collector = collector;
  fake = make_shared<FakeScorer>();
}

void BooleanWeight::BulkScorerAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  collector->setScorer(fake);
}

void BooleanWeight::BulkScorerAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass::collect(int doc) 
{
  fake->doc = doc;
  collector->collect(doc);
}

int64_t BooleanWeight::BulkScorerAnonymousInnerClass::cost()
{
  return scorer->cost();
}

shared_ptr<BulkScorer> BooleanWeight::optionalBulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  deque<std::shared_ptr<BulkScorer>> optional =
      deque<std::shared_ptr<BulkScorer>>();
  org::apache::lucene::search::BooleanQuery::const_iterator cIter =
      query->begin();
  for (auto w : weights) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<BooleanClause> c = cIter->next();
    if (c->getOccur() != Occur::SHOULD) {
      continue;
    }
    shared_ptr<BulkScorer> subScorer = w->bulkScorer(context);

    if (subScorer != nullptr) {
      optional.push_back(subScorer);
    }
  }

  if (optional.empty()) {
    return nullptr;
  }

  if (query->getMinimumNumberShouldMatch() > optional.size()) {
    return nullptr;
  }

  if (optional.size() == 1) {
    return optional[0];
  }

  return make_shared<BooleanScorer>(
      shared_from_this(), optional,
      max(1, query->getMinimumNumberShouldMatch()), needsScores);
}

shared_ptr<BulkScorer> BooleanWeight::requiredBulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<BulkScorer> scorer = nullptr;

  org::apache::lucene::search::BooleanQuery::const_iterator cIter =
      query->begin();
  for (auto w : weights) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<BooleanClause> c = cIter->next();
    if (c->isRequired() == false) {
      continue;
    }
    if (scorer != nullptr) {
      // we don't have a BulkScorer for conjunctions
      return nullptr;
    }
    scorer = w->bulkScorer(context);
    if (scorer == nullptr) {
      // no matches
      return nullptr;
    }
    if (c->isScoring() == false && needsScores) {
      scorer = disableScoring(scorer);
    }
  }
  return scorer;
}

shared_ptr<BulkScorer> BooleanWeight::booleanScorer(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr int numOptionalClauses = query->getClauses(Occur::SHOULD)->size();
  constexpr int numRequiredClauses = query->getClauses(Occur::MUST)->size() +
                                     query->getClauses(Occur::FILTER)->size();

  shared_ptr<BulkScorer> positiveScorer;
  if (numRequiredClauses == 0) {
    positiveScorer = optionalBulkScorer(context);
    if (positiveScorer == nullptr) {
      return nullptr;
    }

    // TODO: what is the right heuristic here?
    constexpr int64_t costThreshold;
    if (query->getMinimumNumberShouldMatch() <= 1) {
      // when all clauses are optional, use BooleanScorer aggressively
      // TODO: is there actually a threshold under which we should rather
      // use the regular scorer?
      costThreshold = -1;
    } else {
      // when a minimum number of clauses should match, BooleanScorer is
      // going to score all windows that have at least minNrShouldMatch
      // matches in the window. But there is no way to know if there is
      // an intersection (all clauses might match a different doc ID and
      // there will be no matches in the end) so we should only use
      // BooleanScorer if matches are very dense
      costThreshold = context->reader()->maxDoc() / 3;
    }

    if (positiveScorer->cost() < costThreshold) {
      return nullptr;
    }

  } else if (numRequiredClauses == 1 && numOptionalClauses == 0 &&
             query->getMinimumNumberShouldMatch() == 0) {
    positiveScorer = requiredBulkScorer(context);
  } else {
    // TODO: there are some cases where BooleanScorer
    // would handle conjunctions faster than
    // BooleanScorer2...
    return nullptr;
  }

  if (positiveScorer == nullptr) {
    return nullptr;
  }

  deque<std::shared_ptr<Scorer>> prohibited =
      deque<std::shared_ptr<Scorer>>();
  org::apache::lucene::search::BooleanQuery::const_iterator cIter =
      query->begin();
  for (auto w : weights) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<BooleanClause> c = cIter->next();
    if (c->isProhibited()) {
      shared_ptr<Scorer> scorer = w->scorer(context);
      if (scorer != nullptr) {
        prohibited.push_back(scorer);
      }
    }
  }

  if (prohibited.empty()) {
    return positiveScorer;
  } else {
    shared_ptr<Scorer> prohibitedScorer =
        prohibited.size() == 1 ? prohibited[0]
                               : make_shared<DisjunctionSumScorer>(
                                     shared_from_this(), prohibited, false);
    if (prohibitedScorer->twoPhaseIterator() != nullptr) {
      // ReqExclBulkScorer can't deal efficiently with two-phased prohibited
      // clauses
      return nullptr;
    }
    return make_shared<ReqExclBulkScorer>(positiveScorer,
                                          prohibitedScorer->begin());
  }
}

shared_ptr<BulkScorer> BooleanWeight::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<BulkScorer> *const bulkScorer = booleanScorer(context);
  if (bulkScorer != nullptr) {
    // bulk scoring is applicable, use it
    return bulkScorer;
  } else {
    // use a Scorer-based impl (BS2)
    return Weight::bulkScorer(context);
  }
}

shared_ptr<Scorer>
BooleanWeight::scorer(shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> scorerSupplier = this->scorerSupplier(context);
  if (scorerSupplier == nullptr) {
    return nullptr;
  }
  return scorerSupplier->get(numeric_limits<int64_t>::max());
}

bool BooleanWeight::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  if (weights.size() > TermInSetQuery::BOOLEAN_REWRITE_TERM_COUNT_THRESHOLD) {
    // Disallow caching large bool queries to not encourage users
    // to build large bool queries as a workaround to the fact that
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

shared_ptr<ScorerSupplier> BooleanWeight::scorerSupplier(
    shared_ptr<LeafReaderContext> context) 
{
  int minShouldMatch = query->getMinimumNumberShouldMatch();

  const unordered_map<Occur, deque<std::shared_ptr<ScorerSupplier>>>
      scorers = make_shared<
          EnumMap<Occur, deque<std::shared_ptr<ScorerSupplier>>>>(
          Occur::typeid);
  for (Occur occur : Occur::values()) {
    scorers.emplace(occur, deque<>());
  }

  org::apache::lucene::search::BooleanQuery::const_iterator cIter =
      query->begin();
  for (auto w : weights) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<BooleanClause> c = cIter->next();
    shared_ptr<ScorerSupplier> subScorer = w->scorerSupplier(context);
    if (subScorer == nullptr) {
      if (c->isRequired()) {
        return nullptr;
      }
    } else {
      scorers[c->getOccur()]->add(subScorer);
    }
  }

  // scorer simplifications:

  if (scorers[Occur::SHOULD]->size() == minShouldMatch) {
    // any optional clauses are in fact required
    scorers[Occur::MUST]->addAll(scorers[Occur::SHOULD]);
    scorers[Occur::SHOULD]->clear();
    minShouldMatch = 0;
  }

  if (scorers[Occur::FILTER]->isEmpty() && scorers[Occur::MUST]->isEmpty() &&
      scorers[Occur::SHOULD]->isEmpty()) {
    // no required and optional clauses.
    return nullptr;
  } else if (scorers[Occur::SHOULD]->size() < minShouldMatch) {
    // either >1 req scorer, or there are 0 req scorers and at least 1
    // optional scorer. Therefore if there are not enough optional scorers
    // no documents will be matched by the query
    return nullptr;
  }

  // we don't need scores, so if we have required clauses, drop optional clauses
  // completely
  if (!needsScores && minShouldMatch == 0 &&
      scorers[Occur::MUST]->size() + scorers[Occur::FILTER]->size() > 0) {
    scorers[Occur::SHOULD]->clear();
  }

  return make_shared<Boolean2ScorerSupplier>(shared_from_this(), scorers,
                                             needsScores, minShouldMatch);
}
} // namespace org::apache::lucene::search