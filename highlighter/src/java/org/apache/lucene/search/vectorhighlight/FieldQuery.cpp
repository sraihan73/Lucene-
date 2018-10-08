using namespace std;

#include "FieldQuery.h"

namespace org::apache::lucene::search::vectorhighlight
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using CustomScoreQuery = org::apache::lucene::queries::CustomScoreQuery;
using FunctionScoreQuery =
    org::apache::lucene::queries::function::FunctionScoreQuery;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using SynonymQuery = org::apache::lucene::search::SynonymQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using ToParentBlockJoinQuery =
    org::apache::lucene::search::join::ToParentBlockJoinQuery;
using TermInfo =
    org::apache::lucene::search::vectorhighlight::FieldTermStack::TermInfo;

FieldQuery::FieldQuery(shared_ptr<Query> query, shared_ptr<IndexReader> reader,
                       bool phraseHighlight, bool fieldMatch) 
    : fieldMatch(fieldMatch)
{
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      make_shared<LinkedHashSet<std::shared_ptr<Query>>>();
  flatten(query, reader, flatQueries, 1.0f);
  saveTerms(flatQueries, reader);
  shared_ptr<deque<std::shared_ptr<Query>>> expandQueries =
      expand(flatQueries);

  for (auto flatQuery : expandQueries) {
    shared_ptr<QueryPhraseMap> rootMap = getRootMap(flatQuery);
    rootMap->add(flatQuery, reader);
    float boost = 1.0f;
    while (std::dynamic_pointer_cast<BoostQuery>(flatQuery) != nullptr) {
      shared_ptr<BoostQuery> bq =
          std::static_pointer_cast<BoostQuery>(flatQuery);
      flatQuery = bq->getQuery();
      boost *= bq->getBoost();
    }
    if (!phraseHighlight &&
        std::dynamic_pointer_cast<PhraseQuery>(flatQuery) != nullptr) {
      shared_ptr<PhraseQuery> pq =
          std::static_pointer_cast<PhraseQuery>(flatQuery);
      if (pq->getTerms().size() > 1) {
        for (auto term : pq->getTerms()) {
          rootMap->addTerm(term, boost);
        }
      }
    }
  }
}

FieldQuery::FieldQuery(shared_ptr<Query> query, bool phraseHighlight,
                       bool fieldMatch) 
    : FieldQuery(query, nullptr, phraseHighlight, fieldMatch)
{
}

void FieldQuery::flatten(
    shared_ptr<Query> sourceQuery, shared_ptr<IndexReader> reader,
    shared_ptr<deque<std::shared_ptr<Query>>> flatQueries,
    float boost) 
{
  while (std::dynamic_pointer_cast<BoostQuery>(sourceQuery) != nullptr) {
    shared_ptr<BoostQuery> bq =
        std::static_pointer_cast<BoostQuery>(sourceQuery);
    sourceQuery = bq->getQuery();
    boost *= bq->getBoost();
  }
  if (std::dynamic_pointer_cast<BooleanQuery>(sourceQuery) != nullptr) {
    shared_ptr<BooleanQuery> bq =
        std::static_pointer_cast<BooleanQuery>(sourceQuery);
    for (auto clause : bq) {
      if (!clause->isProhibited()) {
        flatten(clause->getQuery(), reader, flatQueries, boost);
      }
    }
  } else if (std::dynamic_pointer_cast<DisjunctionMaxQuery>(sourceQuery) !=
             nullptr) {
    shared_ptr<DisjunctionMaxQuery> dmq =
        std::static_pointer_cast<DisjunctionMaxQuery>(sourceQuery);
    for (auto query : dmq) {
      flatten(query, reader, flatQueries, boost);
    }
  } else if (std::dynamic_pointer_cast<TermQuery>(sourceQuery) != nullptr) {
    if (boost != 1.0f) {
      sourceQuery = make_shared<BoostQuery>(sourceQuery, boost);
    }
    if (!flatQueries->contains(sourceQuery)) {
      flatQueries->add(sourceQuery);
    }
  } else if (std::dynamic_pointer_cast<SynonymQuery>(sourceQuery) != nullptr) {
    shared_ptr<SynonymQuery> synQuery =
        std::static_pointer_cast<SynonymQuery>(sourceQuery);
    for (auto term : synQuery->getTerms()) {
      flatten(make_shared<TermQuery>(term), reader, flatQueries, boost);
    }
  } else if (std::dynamic_pointer_cast<PhraseQuery>(sourceQuery) != nullptr) {
    shared_ptr<PhraseQuery> pq =
        std::static_pointer_cast<PhraseQuery>(sourceQuery);
    if (pq->getTerms().size() == 1) {
      sourceQuery = make_shared<TermQuery>(pq->getTerms()[0]);
    }
    if (boost != 1.0f) {
      sourceQuery = make_shared<BoostQuery>(sourceQuery, boost);
    }
    flatQueries->add(sourceQuery);
  } else if (std::dynamic_pointer_cast<ConstantScoreQuery>(sourceQuery) !=
             nullptr) {
    shared_ptr<Query> *const q =
        (std::static_pointer_cast<ConstantScoreQuery>(sourceQuery))->getQuery();
    if (q != nullptr) {
      flatten(q, reader, flatQueries, boost);
    }
  } else if (std::dynamic_pointer_cast<CustomScoreQuery>(sourceQuery) !=
             nullptr) {
    shared_ptr<Query> *const q =
        (std::static_pointer_cast<CustomScoreQuery>(sourceQuery))
            ->getSubQuery();
    if (q != nullptr) {
      flatten(q, reader, flatQueries, boost);
    }
  } else if (std::dynamic_pointer_cast<FunctionScoreQuery>(sourceQuery) !=
             nullptr) {
    shared_ptr<Query> *const q =
        (std::static_pointer_cast<FunctionScoreQuery>(sourceQuery))
            ->getWrappedQuery();
    if (q != nullptr) {
      flatten(q, reader, flatQueries, boost);
    }
  } else if (std::dynamic_pointer_cast<ToParentBlockJoinQuery>(sourceQuery) !=
             nullptr) {
    shared_ptr<Query> childQuery =
        (std::static_pointer_cast<ToParentBlockJoinQuery>(sourceQuery))
            ->getChildQuery();
    if (childQuery != nullptr) {
      flatten(childQuery, reader, flatQueries, boost);
    }
  } else if (reader != nullptr) {
    shared_ptr<Query> query = sourceQuery;
    shared_ptr<Query> rewritten;
    if (std::dynamic_pointer_cast<MultiTermQuery>(sourceQuery) != nullptr) {
      rewritten =
          (make_shared<MultiTermQuery::TopTermsScoringBooleanQueryRewrite>(
               MAX_MTQ_TERMS))
              ->rewrite(reader,
                        std::static_pointer_cast<MultiTermQuery>(query));
    } else {
      rewritten = query->rewrite(reader);
    }
    if (rewritten != query) {
      // only rewrite once and then flatten again - the rewritten query could
      // have a speacial treatment if this method is overwritten in a subclass.
      flatten(rewritten, reader, flatQueries, boost);
    }
    // if the query is already rewritten we discard it
  }
  // else discard queries
}

shared_ptr<deque<std::shared_ptr<Query>>>
FieldQuery::expand(shared_ptr<deque<std::shared_ptr<Query>>> flatQueries)
{
  shared_ptr<Set<std::shared_ptr<Query>>> expandQueries =
      make_shared<LinkedHashSet<std::shared_ptr<Query>>>();
  for (shared_ptr<deque<std::shared_ptr<Query>>::const_iterator> i =
           flatQueries->begin();
       i != flatQueries->end(); ++i) {
    shared_ptr<Query> query = *i;
    (*i)->remove();
    expandQueries->add(query);
    float queryBoost = 1.0f;
    while (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
      shared_ptr<BoostQuery> bq = std::static_pointer_cast<BoostQuery>(query);
      queryBoost *= bq->getBoost();
      query = bq->getQuery();
    }
    if (!(std::dynamic_pointer_cast<PhraseQuery>(query) != nullptr)) {
      continue;
    }
    for (shared_ptr<deque<std::shared_ptr<Query>>::const_iterator> j =
             flatQueries->begin();
         j != flatQueries->end(); ++j) {
      shared_ptr<Query> qj = *j;
      float qjBoost = 1.0f;
      while (std::dynamic_pointer_cast<BoostQuery>(qj) != nullptr) {
        shared_ptr<BoostQuery> bq = std::static_pointer_cast<BoostQuery>(qj);
        qjBoost *= bq->getBoost();
        qj = bq->getQuery();
      }
      if (!(std::dynamic_pointer_cast<PhraseQuery>(qj) != nullptr)) {
        continue;
      }
      checkOverlap(expandQueries, std::static_pointer_cast<PhraseQuery>(query),
                   queryBoost, std::static_pointer_cast<PhraseQuery>(qj),
                   qjBoost);
    }
  }
  return expandQueries;
}

void FieldQuery::checkOverlap(
    shared_ptr<deque<std::shared_ptr<Query>>> expandQueries,
    shared_ptr<PhraseQuery> a, float aBoost, shared_ptr<PhraseQuery> b,
    float bBoost)
{
  if (a->getSlop() != b->getSlop()) {
    return;
  }
  std::deque<std::shared_ptr<Term>> ats = a->getTerms();
  std::deque<std::shared_ptr<Term>> bts = b->getTerms();
  if (fieldMatch && ats[0]->field() != bts[0]->field()) {
    return;
  }
  checkOverlap(expandQueries, ats, bts, a->getSlop(), aBoost);
  checkOverlap(expandQueries, bts, ats, b->getSlop(), bBoost);
}

void FieldQuery::checkOverlap(
    shared_ptr<deque<std::shared_ptr<Query>>> expandQueries,
    std::deque<std::shared_ptr<Term>> &src,
    std::deque<std::shared_ptr<Term>> &dest, int slop, float boost)
{
  // beginning from 1 (not 0) is safe because that the PhraseQuery has multiple
  // terms is guaranteed in flatten() method (if PhraseQuery has only one term,
  // flatten() converts PhraseQuery to TermQuery)
  for (int i = 1; i < src.size(); i++) {
    bool overlap = true;
    for (int j = i; j < src.size(); j++) {
      if ((j - i) < dest.size() && src[j]->text() != dest[j - i]->text()) {
        overlap = false;
        break;
      }
    }
    if (overlap && src.size() - i < dest.size()) {
      shared_ptr<PhraseQuery::Builder> pqBuilder =
          make_shared<PhraseQuery::Builder>();
      for (auto srcTerm : src) {
        pqBuilder->add(srcTerm);
      }
      for (int k = src.size() - i; k < dest.size(); k++) {
        pqBuilder->add(make_shared<Term>(src[0]->field(), dest[k]->text()));
      }
      pqBuilder->setSlop(slop);
      shared_ptr<Query> pq = pqBuilder->build();
      if (boost != 1.0f) {
        pq = make_shared<BoostQuery>(pq, 1.0f);
      }
      if (!expandQueries->contains(pq)) {
        expandQueries->add(pq);
      }
    }
  }
}

shared_ptr<QueryPhraseMap> FieldQuery::getRootMap(shared_ptr<Query> query)
{
  wstring key = getKey(query);
  shared_ptr<QueryPhraseMap> map_obj = rootMaps[key];
  if (map_obj == nullptr) {
    map_obj = make_shared<QueryPhraseMap>(shared_from_this());
    rootMaps.emplace(key, map_obj);
  }
  return map_obj;
}

wstring FieldQuery::getKey(shared_ptr<Query> query)
{
  if (!fieldMatch) {
    return L"";
  }
  while (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
    query = (std::static_pointer_cast<BoostQuery>(query))->getQuery();
  }
  if (std::dynamic_pointer_cast<TermQuery>(query) != nullptr) {
    return (std::static_pointer_cast<TermQuery>(query))->getTerm()->field();
  } else if (std::dynamic_pointer_cast<PhraseQuery>(query) != nullptr) {
    shared_ptr<PhraseQuery> pq = std::static_pointer_cast<PhraseQuery>(query);
    std::deque<std::shared_ptr<Term>> terms = pq->getTerms();
    return terms[0]->field();
  } else if (std::dynamic_pointer_cast<MultiTermQuery>(query) != nullptr) {
    return (std::static_pointer_cast<MultiTermQuery>(query))->getField();
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw runtime_error(L"query \"" + query->toString() +
                        L"\" must be flatten first.");
  }
}

void FieldQuery::saveTerms(
    shared_ptr<deque<std::shared_ptr<Query>>> flatQueries,
    shared_ptr<IndexReader> reader) 
{
  for (auto query : flatQueries) {
    while (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
      query = (std::static_pointer_cast<BoostQuery>(query))->getQuery();
    }
    shared_ptr<Set<wstring>> termSet = getTermSet(query);
    if (std::dynamic_pointer_cast<TermQuery>(query) != nullptr) {
      termSet->add(
          (std::static_pointer_cast<TermQuery>(query))->getTerm()->text());
    } else if (std::dynamic_pointer_cast<PhraseQuery>(query) != nullptr) {
      for (auto term :
           (std::static_pointer_cast<PhraseQuery>(query))->getTerms()) {
        termSet->add(term->text());
      }
    } else if (std::dynamic_pointer_cast<MultiTermQuery>(query) != nullptr &&
               reader != nullptr) {
      shared_ptr<BooleanQuery> mtqTerms =
          std::static_pointer_cast<BooleanQuery>(query->rewrite(reader));
      for (auto clause : mtqTerms) {
        termSet->add((std::static_pointer_cast<TermQuery>(clause->getQuery()))
                         ->getTerm()
                         ->text());
      }
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw runtime_error(L"query \"" + query->toString() +
                          L"\" must be flatten first.");
    }
  }
}

shared_ptr<Set<wstring>> FieldQuery::getTermSet(shared_ptr<Query> query)
{
  wstring key = getKey(query);
  shared_ptr<Set<wstring>> set = termSetMap[key];
  if (set == nullptr) {
    set = unordered_set<>();
    termSetMap.emplace(key, set);
  }
  return set;
}

shared_ptr<Set<wstring>> FieldQuery::getTermSet(const wstring &field)
{
  return termSetMap[fieldMatch ? field : nullptr];
}

shared_ptr<QueryPhraseMap> FieldQuery::getFieldTermMap(const wstring &fieldName,
                                                       const wstring &term)
{
  shared_ptr<QueryPhraseMap> rootMap = getRootMap(fieldName);
  return rootMap == nullptr ? nullptr : rootMap->subMap[term];
}

shared_ptr<QueryPhraseMap>
FieldQuery::searchPhrase(const wstring &fieldName,
                         deque<std::shared_ptr<TermInfo>> &phraseCandidate)
{
  shared_ptr<QueryPhraseMap> root = getRootMap(fieldName);
  if (root == nullptr) {
    return nullptr;
  }
  return root->searchPhrase(phraseCandidate);
}

shared_ptr<QueryPhraseMap> FieldQuery::getRootMap(const wstring &fieldName)
{
  return rootMaps[fieldMatch ? fieldName : nullptr];
}

int FieldQuery::nextTermOrPhraseNumber() { return termOrPhraseNumber++; }

FieldQuery::QueryPhraseMap::QueryPhraseMap(shared_ptr<FieldQuery> fieldQuery)
{
  this->fieldQuery = fieldQuery;
}

void FieldQuery::QueryPhraseMap::addTerm(shared_ptr<Term> term, float boost)
{
  shared_ptr<QueryPhraseMap> map_obj = getOrNewMap(subMap, term->text());
  map_obj->markTerminal(boost);
}

shared_ptr<QueryPhraseMap> FieldQuery::QueryPhraseMap::getOrNewMap(
    unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &subMap,
    const wstring &term)
{
  shared_ptr<QueryPhraseMap> map_obj = subMap[term];
  if (map_obj == nullptr) {
    map_obj = make_shared<QueryPhraseMap>(fieldQuery);
    subMap.emplace(term, map_obj);
  }
  return map_obj;
}

void FieldQuery::QueryPhraseMap::add(shared_ptr<Query> query,
                                     shared_ptr<IndexReader> reader)
{
  float boost = 1.0f;
  while (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
    shared_ptr<BoostQuery> bq = std::static_pointer_cast<BoostQuery>(query);
    query = bq->getQuery();
    boost = bq->getBoost();
  }
  if (std::dynamic_pointer_cast<TermQuery>(query) != nullptr) {
    addTerm((std::static_pointer_cast<TermQuery>(query))->getTerm(), boost);
  } else if (std::dynamic_pointer_cast<PhraseQuery>(query) != nullptr) {
    shared_ptr<PhraseQuery> pq = std::static_pointer_cast<PhraseQuery>(query);
    std::deque<std::shared_ptr<Term>> terms = pq->getTerms();
    unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = subMap;
    shared_ptr<QueryPhraseMap> qpm = nullptr;
    for (auto term : terms) {
      qpm = getOrNewMap(map_obj, term->text());
      map_obj = qpm->subMap;
    }
    qpm->markTerminal(pq->getSlop(), boost);
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw runtime_error(L"query \"" + query->toString() +
                        L"\" must be flatten first.");
  }
}

shared_ptr<QueryPhraseMap>
FieldQuery::QueryPhraseMap::getTermMap(const wstring &term)
{
  return subMap[term];
}

void FieldQuery::QueryPhraseMap::markTerminal(float boost)
{
  markTerminal(0, boost);
}

void FieldQuery::QueryPhraseMap::markTerminal(int slop, float boost)
{
  this->terminal = true;
  this->slop = slop;
  this->boost = boost;
  this->termOrPhraseNumber = fieldQuery->nextTermOrPhraseNumber();
}

bool FieldQuery::QueryPhraseMap::isTerminal() { return terminal; }

int FieldQuery::QueryPhraseMap::getSlop() { return slop; }

float FieldQuery::QueryPhraseMap::getBoost() { return boost; }

int FieldQuery::QueryPhraseMap::getTermOrPhraseNumber()
{
  return termOrPhraseNumber;
}

shared_ptr<QueryPhraseMap> FieldQuery::QueryPhraseMap::searchPhrase(
    deque<std::shared_ptr<TermInfo>> &phraseCandidate)
{
  shared_ptr<QueryPhraseMap> currMap = shared_from_this();
  for (auto ti : phraseCandidate) {
    currMap = currMap->subMap[ti->getText()];
    if (currMap == nullptr) {
      return nullptr;
    }
  }
  return currMap->isValidTermOrPhrase(phraseCandidate) ? currMap : nullptr;
}

bool FieldQuery::QueryPhraseMap::isValidTermOrPhrase(
    deque<std::shared_ptr<TermInfo>> &phraseCandidate)
{
  // check terminal
  if (!terminal) {
    return false;
  }

  // if the candidate is a term, it is valid
  if (phraseCandidate.size() == 1) {
    return true;
  }

  // else check whether the candidate is valid phrase
  // compare position-gaps between terms to slop
  int pos = phraseCandidate[0]->getPosition();
  for (int i = 1; i < phraseCandidate.size(); i++) {
    int nextPos = phraseCandidate[i]->getPosition();
    if (abs(nextPos - pos - 1) > slop) {
      return false;
    }
    pos = nextPos;
  }
  return true;
}
} // namespace org::apache::lucene::search::vectorhighlight