using namespace std;

#include "BooleanQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
int BooleanQuery::maxClauseCount = 1024;

BooleanQuery::TooManyClauses::TooManyClauses()
    : RuntimeException(L"maxClauseCount is set to " + maxClauseCount)
{
}

int BooleanQuery::getMaxClauseCount() { return maxClauseCount; }

void BooleanQuery::setMaxClauseCount(int maxClauseCount)
{
  if (maxClauseCount < 1) {
    throw invalid_argument(L"maxClauseCount must be >= 1");
  }
  BooleanQuery::maxClauseCount = maxClauseCount;
}

BooleanQuery::Builder::Builder() {}

shared_ptr<Builder> BooleanQuery::Builder::setMinimumNumberShouldMatch(int min)
{
  this->minimumNumberShouldMatch = min;
  return shared_from_this();
}

shared_ptr<Builder> BooleanQuery::Builder::add(shared_ptr<BooleanClause> clause)
{
  if (clauses.size() >= maxClauseCount) {
    throw make_shared<TooManyClauses>();
  }
  clauses.push_back(clause);
  return shared_from_this();
}

shared_ptr<Builder> BooleanQuery::Builder::add(shared_ptr<Query> query,
                                               Occur occur)
{
  return add(make_shared<BooleanClause>(query, occur));
}

shared_ptr<BooleanQuery> BooleanQuery::Builder::build()
{
  return make_shared<BooleanQuery>(
      minimumNumberShouldMatch,
      clauses.toArray(std::deque<std::shared_ptr<BooleanClause>>(0)));
}

BooleanQuery::BooleanQuery(int minimumNumberShouldMatch,
                           std::deque<std::shared_ptr<BooleanClause>> &clauses)
    : minimumNumberShouldMatch(minimumNumberShouldMatch),
      clauses(Collections::unmodifiableList(Arrays::asList(clauses))),
      clauseSets(make_shared<EnumMap<>>(Occur::typeid))
{
  // duplicates matter for SHOULD and MUST
  clauseSets.emplace(Occur::SHOULD, make_shared<Multiset<>>());
  clauseSets.emplace(Occur::MUST, make_shared<Multiset<>>());
  // but not for FILTER and MUST_NOT
  clauseSets.emplace(Occur::FILTER, unordered_set<>());
  clauseSets.emplace(Occur::MUST_NOT, unordered_set<>());
  for (auto clause : clauses) {
    clauseSets[clause->getOccur()]->add(clause->getQuery());
  }
}

int BooleanQuery::getMinimumNumberShouldMatch()
{
  return minimumNumberShouldMatch;
}

deque<std::shared_ptr<BooleanClause>> BooleanQuery::clauses()
{
  return clauses_;
}

shared_ptr<deque<std::shared_ptr<Query>>>
BooleanQuery::getClauses(Occur occur)
{
  return clauseSets[occur];
}

shared_ptr<Iterator<std::shared_ptr<BooleanClause>>> BooleanQuery::iterator()
{
  return clauses_.begin();
}

shared_ptr<BooleanQuery> BooleanQuery::rewriteNoScoring()
{
  if (clauseSets[Occur::MUST]->size() == 0) {
    return shared_from_this();
  }
  shared_ptr<BooleanQuery::Builder> newQuery =
      make_shared<BooleanQuery::Builder>();
  newQuery->setMinimumNumberShouldMatch(getMinimumNumberShouldMatch());
  for (auto clause : clauses_) {
    if (clause->getOccur() == Occur::MUST) {
      newQuery->add(clause->getQuery(), Occur::FILTER);
    } else {
      newQuery->add(clause);
    }
  }
  return newQuery->build();
}

shared_ptr<Weight>
BooleanQuery::createWeight(shared_ptr<IndexSearcher> searcher, bool needsScores,
                           float boost) 
{
  shared_ptr<BooleanQuery> query = shared_from_this();
  if (needsScores == false) {
    query = rewriteNoScoring();
  }
  return make_shared<BooleanWeight>(query, searcher, needsScores, boost);
}

shared_ptr<Query>
BooleanQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  if (clauses_.empty()) {
    return make_shared<MatchNoDocsQuery>(L"empty BooleanQuery");
  }

  // optimize 1-clause queries
  if (clauses_.size() == 1) {
    shared_ptr<BooleanClause> c = clauses_[0];
    shared_ptr<Query> query = c->getQuery();
    if (minimumNumberShouldMatch == 1 && c->getOccur() == Occur::SHOULD) {
      return query;
    } else if (minimumNumberShouldMatch == 0) {
      switch (c->getOccur()) {
      case SHOULD:
      case MUST:
        return query;
      case FILTER:
        // no scoring clauses, so return a score of 0
        return make_shared<BoostQuery>(make_shared<ConstantScoreQuery>(query),
                                       0);
      case MUST_NOT:
        // no positive clauses
        return make_shared<MatchNoDocsQuery>(L"pure negative BooleanQuery");
      default:
        throw make_shared<AssertionError>();
      }
    }
  }

  {
    // recursively rewrite
    shared_ptr<BooleanQuery::Builder> builder =
        make_shared<BooleanQuery::Builder>();
    builder->setMinimumNumberShouldMatch(getMinimumNumberShouldMatch());
    bool actuallyRewritten = false;
    for (auto clause : shared_from_this()) {
      shared_ptr<Query> query = clause->getQuery();
      shared_ptr<Query> rewritten = query->rewrite(reader);
      if (rewritten != query) {
        actuallyRewritten = true;
      }
      builder->add(rewritten, clause->getOccur());
    }
    if (actuallyRewritten) {
      return builder->build();
    }
  }

  {
    // remove duplicate FILTER and MUST_NOT clauses
    int clauseCount = 0;
    for (auto queries : clauseSets) {
      clauseCount += queries->second->size();
    }
    if (clauseCount != clauses_.size()) {
      // since clauseSets implicitly deduplicates FILTER and MUST_NOT
      // clauses, this means there were duplicates
      shared_ptr<BooleanQuery::Builder> rewritten =
          make_shared<BooleanQuery::Builder>();
      rewritten->setMinimumNumberShouldMatch(minimumNumberShouldMatch);
      for (auto entry : clauseSets) {
        constexpr Occur occur = entry.first;
        for (shared_ptr<Query> query : entry.second) {
          rewritten->add(query, occur);
        }
      }
      return rewritten->build();
    }
  }

  // Check whether some clauses are both required and excluded
  shared_ptr<deque<std::shared_ptr<Query>>> *const mustNotClauses =
      clauseSets[Occur::MUST_NOT];
  if (!mustNotClauses->isEmpty()) {
    const function<bool(Query *)> p = clauseSets[Occur::MUST] ::contains;
    if (mustNotClauses->stream().anyMatch(
            p.or (clauseSets[Occur::FILTER] ::contains))) {
      return make_shared<MatchNoDocsQuery>(
          L"FILTER or MUST clause also in MUST_NOT");
    }
    if (mustNotClauses->contains(make_shared<MatchAllDocsQuery>())) {
      return make_shared<MatchNoDocsQuery>(
          L"MUST_NOT clause is MatchAllDocsQuery");
    }
  }

  // remove FILTER clauses that are also MUST clauses
  // or that match all documents
  if (clauseSets[Occur::MUST]->size() > 0 &&
      clauseSets[Occur::FILTER]->size() > 0) {
    shared_ptr<Set<std::shared_ptr<Query>>> *const filters =
        unordered_set<std::shared_ptr<Query>>(clauseSets[Occur::FILTER]);
    bool modified = filters->remove(make_shared<MatchAllDocsQuery>());
    modified |= filters->removeAll(clauseSets[Occur::MUST]);
    if (modified) {
      shared_ptr<BooleanQuery::Builder> builder =
          make_shared<BooleanQuery::Builder>();
      builder->setMinimumNumberShouldMatch(getMinimumNumberShouldMatch());
      for (auto clause : clauses_) {
        if (clause->getOccur() != Occur::FILTER) {
          builder->add(clause);
        }
      }
      for (auto filter : filters) {
        builder->add(filter, Occur::FILTER);
      }
      return builder->build();
    }
  }

  // convert FILTER clauses that are also SHOULD clauses to MUST clauses
  if (clauseSets[Occur::SHOULD]->size() > 0 &&
      clauseSets[Occur::FILTER]->size() > 0) {
    shared_ptr<deque<std::shared_ptr<Query>>> *const filters =
        clauseSets[Occur::FILTER];
    shared_ptr<deque<std::shared_ptr<Query>>> *const shoulds =
        clauseSets[Occur::SHOULD];

    shared_ptr<Set<std::shared_ptr<Query>>> intersection =
        unordered_set<std::shared_ptr<Query>>(filters);
    intersection->retainAll(shoulds);

    if (intersection->isEmpty() == false) {
      shared_ptr<BooleanQuery::Builder> builder =
          make_shared<BooleanQuery::Builder>();
      int minShouldMatch = getMinimumNumberShouldMatch();

      for (auto clause : clauses_) {
        if (intersection->contains(clause->getQuery())) {
          if (clause->getOccur() == Occur::SHOULD) {
            builder->add(
                make_shared<BooleanClause>(clause->getQuery(), Occur::MUST));
            minShouldMatch--;
          }
        } else {
          builder->add(clause);
        }
      }

      builder->setMinimumNumberShouldMatch(max(0, minShouldMatch));
      return builder->build();
    }
  }

  // Deduplicate SHOULD clauses by summing up their boosts
  if (clauseSets[Occur::SHOULD]->size() > 0 && minimumNumberShouldMatch <= 1) {
    unordered_map<std::shared_ptr<Query>, double> shouldClauses =
        unordered_map<std::shared_ptr<Query>, double>();
    for (auto query : clauseSets[Occur::SHOULD]) {
      double boost = 1;
      while (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
        shared_ptr<BoostQuery> bq = std::static_pointer_cast<BoostQuery>(query);
        boost *= bq->getBoost();
        query = bq->getQuery();
      }
      shouldClauses.emplace(query,
                            shouldClauses.getOrDefault(query, 0) + boost);
    }
    if (shouldClauses.size() != clauseSets[Occur::SHOULD]->size()) {
      shared_ptr<BooleanQuery::Builder> builder =
          (make_shared<BooleanQuery::Builder>())
              ->setMinimumNumberShouldMatch(minimumNumberShouldMatch);
      for (auto entry : shouldClauses) {
        shared_ptr<Query> query = entry.first;
        float boost = entry.second::floatValue();
        if (boost != 1.0f) {
          query = make_shared<BoostQuery>(query, boost);
        }
        builder->add(query, Occur::SHOULD);
      }
      for (auto clause : clauses_) {
        if (clause->getOccur() != Occur::SHOULD) {
          builder->add(clause);
        }
      }
      return builder->build();
    }
  }

  // Deduplicate MUST clauses by summing up their boosts
  if (clauseSets[Occur::MUST]->size() > 0) {
    unordered_map<std::shared_ptr<Query>, double> mustClauses =
        unordered_map<std::shared_ptr<Query>, double>();
    for (auto query : clauseSets[Occur::MUST]) {
      double boost = 1;
      while (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
        shared_ptr<BoostQuery> bq = std::static_pointer_cast<BoostQuery>(query);
        boost *= bq->getBoost();
        query = bq->getQuery();
      }
      mustClauses.emplace(query, mustClauses.getOrDefault(query, 0) + boost);
    }
    if (mustClauses.size() != clauseSets[Occur::MUST]->size()) {
      shared_ptr<BooleanQuery::Builder> builder =
          (make_shared<BooleanQuery::Builder>())
              ->setMinimumNumberShouldMatch(minimumNumberShouldMatch);
      for (auto entry : mustClauses) {
        shared_ptr<Query> query = entry.first;
        float boost = entry.second::floatValue();
        if (boost != 1.0f) {
          query = make_shared<BoostQuery>(query, boost);
        }
        builder->add(query, Occur::MUST);
      }
      for (auto clause : clauses_) {
        if (clause->getOccur() != Occur::MUST) {
          builder->add(clause);
        }
      }
      return builder->build();
    }
  }

  // Rewrite queries whose single scoring clause is a MUST clause on a
  // MatchAllDocsQuery to a ConstantScoreQuery
  {
    shared_ptr<deque<std::shared_ptr<Query>>> *const musts =
        clauseSets[Occur::MUST];
    shared_ptr<deque<std::shared_ptr<Query>>> *const filters =
        clauseSets[Occur::FILTER];
    if (musts->size() == 1 && filters->size() > 0) {
      shared_ptr<Query> must = musts->begin()->next();
      float boost = 1.0f;
      if (std::dynamic_pointer_cast<BoostQuery>(must) != nullptr) {
        shared_ptr<BoostQuery> boostQuery =
            std::static_pointer_cast<BoostQuery>(must);
        must = boostQuery->getQuery();
        boost = boostQuery->getBoost();
      }
      if (must->getClass() == MatchAllDocsQuery::typeid) {
        // our single scoring clause matches everything: rewrite to a CSQ on the
        // filter ignore SHOULD clause for now
        shared_ptr<BooleanQuery::Builder> builder =
            make_shared<BooleanQuery::Builder>();
        for (auto clause : clauses_) {
          switch (clause->getOccur()) {
          case FILTER:
          case MUST_NOT:
            builder->add(clause);
            break;
          default:
            // ignore
            break;
          }
        }
        shared_ptr<Query> rewritten = builder->build();
        rewritten = make_shared<ConstantScoreQuery>(rewritten);
        if (boost != 1.0f) {
          rewritten = make_shared<BoostQuery>(rewritten, boost);
        }

        // now add back the SHOULD clauses
        builder =
            (make_shared<BooleanQuery::Builder>())
                ->setMinimumNumberShouldMatch(getMinimumNumberShouldMatch())
                ->add(rewritten, Occur::MUST);
        for (auto query : clauseSets[Occur::SHOULD]) {
          builder->add(query, Occur::SHOULD);
        }
        rewritten = builder->build();
        return rewritten;
      }
    }
  }

  return Query::rewrite(reader);
}

wstring BooleanQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  bool needParens = getMinimumNumberShouldMatch() > 0;
  if (needParens) {
    buffer->append(L"(");
  }

  int i = 0;
  for (auto c : shared_from_this()) {
    buffer->append(c->getOccur().toString());

    shared_ptr<Query> subQuery = c->getQuery();
    if (std::dynamic_pointer_cast<BooleanQuery>(subQuery) !=
        nullptr) { // wrap sub-bools in parens
      buffer->append(L"(");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buffer->append(subQuery->toString(field));
      buffer->append(L")");
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buffer->append(subQuery->toString(field));
    }

    if (i != clauses_.size() - 1) {
      buffer->append(L" ");
    }
    i += 1;
  }

  if (needParens) {
    buffer->append(L")");
  }

  if (getMinimumNumberShouldMatch() > 0) {
    buffer->append(L'~');
    buffer->append(getMinimumNumberShouldMatch());
  }

  return buffer->toString();
}

bool BooleanQuery::equals(any o)
{
  return sameClassAs(o) && equalsTo(getClass().cast(o));
}

bool BooleanQuery::equalsTo(shared_ptr<BooleanQuery> other)
{
  return getMinimumNumberShouldMatch() ==
             other->getMinimumNumberShouldMatch() &&
         clauseSets.equals(other->clauseSets);
}

int BooleanQuery::computeHashCode()
{
  int hashCode = Objects::hash(minimumNumberShouldMatch, clauseSets);
  if (hashCode == 0) {
    hashCode = 1;
  }
  return hashCode;
}

int BooleanQuery::hashCode()
{
  // no need for synchronization, in the worst case we would just compute the
  // hash several times.
  if (hashCode_ == 0) {
    hashCode_ = computeHashCode();
    assert(hashCode_ != 0);
  }
  assert(hashCode_ == computeHashCode());
  return hashCode_;
}
} // namespace org::apache::lucene::search