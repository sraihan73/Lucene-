using namespace std;

#include "MultiFieldQueryParser.h"

namespace org::apache::lucene::queryparser::classic
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;

MultiFieldQueryParser::MultiFieldQueryParser(
    std::deque<wstring> &fields, shared_ptr<Analyzer> analyzer,
    unordered_map<wstring, float> &boosts)
    : MultiFieldQueryParser(fields, analyzer)
{
  this->boosts = boosts;
}

MultiFieldQueryParser::MultiFieldQueryParser(std::deque<wstring> &fields,
                                             shared_ptr<Analyzer> analyzer)
    : QueryParser(nullptr, analyzer)
{
  this->fields = fields;
}

shared_ptr<Query>
MultiFieldQueryParser::getFieldQuery(const wstring &field,
                                     const wstring &queryText,
                                     int slop) 
{
  if (field == L"") {
    deque<std::shared_ptr<Query>> clauses = deque<std::shared_ptr<Query>>();
    for (int i = 0; i < fields.size(); i++) {
      shared_ptr<Query> q =
          QueryParser::getFieldQuery(fields[i], queryText, true);
      if (q != nullptr) {
        // If the user passes a map_obj of boosts
        if (boosts.size() > 0) {
          // Get the boost from the map_obj and apply them
          optional<float> boost = boosts[fields[i]];
          if (boost) {
            q = make_shared<BoostQuery>(q, boost.value());
          }
        }
        q = applySlop(q, slop);
        clauses.push_back(q);
      }
    }
    if (clauses.empty()) // happens for stopwords
    {
      return nullptr;
    }
    return getMultiFieldQuery(clauses);
  }
  shared_ptr<Query> q = QueryParser::getFieldQuery(field, queryText, true);
  q = applySlop(q, slop);
  return q;
}

shared_ptr<Query> MultiFieldQueryParser::applySlop(shared_ptr<Query> q,
                                                   int slop)
{
  if (std::dynamic_pointer_cast<PhraseQuery>(q) != nullptr) {
    shared_ptr<PhraseQuery::Builder> builder =
        make_shared<PhraseQuery::Builder>();
    builder->setSlop(slop);
    shared_ptr<PhraseQuery> pq = std::static_pointer_cast<PhraseQuery>(q);
    std::deque<std::shared_ptr<org::apache::lucene::index::Term>> terms =
        pq->getTerms();
    std::deque<int> positions = pq->getPositions();
    for (int i = 0; i < terms.size(); ++i) {
      builder->add(terms[i], positions[i]);
    }
    q = builder->build();
  } else if (std::dynamic_pointer_cast<MultiPhraseQuery>(q) != nullptr) {
    shared_ptr<MultiPhraseQuery> mpq =
        std::static_pointer_cast<MultiPhraseQuery>(q);

    if (slop != mpq->getSlop()) {
      q = (make_shared<MultiPhraseQuery::Builder>(mpq))->setSlop(slop)->build();
    }
  }
  return q;
}

shared_ptr<Query>
MultiFieldQueryParser::getFieldQuery(const wstring &field,
                                     const wstring &queryText,
                                     bool quoted) 
{
  if (field == L"") {
    deque<std::shared_ptr<Query>> clauses = deque<std::shared_ptr<Query>>();
    std::deque<std::shared_ptr<Query>> fieldQueries(fields.size());
    int maxTerms = 0;
    for (int i = 0; i < fields.size(); i++) {
      shared_ptr<Query> q =
          QueryParser::getFieldQuery(fields[i], queryText, quoted);
      if (q != nullptr) {
        if (std::dynamic_pointer_cast<BooleanQuery>(q) != nullptr) {
          maxTerms = max(
              maxTerms,
              (std::static_pointer_cast<BooleanQuery>(q))->clauses().size());
        } else {
          maxTerms = max(1, maxTerms);
        }
        fieldQueries[i] = q;
      }
    }
    for (int termNum = 0; termNum < maxTerms; termNum++) {
      deque<std::shared_ptr<Query>> termClauses =
          deque<std::shared_ptr<Query>>();
      for (int i = 0; i < fields.size(); i++) {
        if (fieldQueries[i] != nullptr) {
          shared_ptr<Query> q = nullptr;
          if (std::dynamic_pointer_cast<BooleanQuery>(fieldQueries[i]) !=
              nullptr) {
            deque<std::shared_ptr<BooleanClause>> nestedClauses =
                (std::static_pointer_cast<BooleanQuery>(fieldQueries[i]))
                    ->clauses();
            if (termNum < nestedClauses.size()) {
              q = nestedClauses[termNum]->getQuery();
            }
          } else if (termNum == 0) { // e.g. TermQuery-s
            q = fieldQueries[i];
          }
          if (q != nullptr) {
            if (boosts.size() > 0) {
              // Get the boost from the map_obj and apply them
              optional<float> boost = boosts[fields[i]];
              if (boost) {
                q = make_shared<BoostQuery>(q, boost);
              }
            }
            termClauses.push_back(q);
          }
        }
      }
      if (maxTerms > 1) {
        if (termClauses.size() > 0) {
          shared_ptr<BooleanQuery::Builder> builder = newBooleanQuery();
          for (auto termClause : termClauses) {
            builder->add(termClause, BooleanClause::Occur::SHOULD);
          }
          clauses.push_back(builder->build());
        }
      } else {
        clauses.insert(clauses.end(), termClauses.begin(), termClauses.end());
      }
    }
    if (clauses.empty()) // happens for stopwords
    {
      return nullptr;
    }
    return getMultiFieldQuery(clauses);
  }
  shared_ptr<Query> q = QueryParser::getFieldQuery(field, queryText, quoted);
  return q;
}

shared_ptr<Query>
MultiFieldQueryParser::getFuzzyQuery(const wstring &field,
                                     const wstring &termStr,
                                     float minSimilarity) 
{
  if (field == L"") {
    deque<std::shared_ptr<Query>> clauses = deque<std::shared_ptr<Query>>();
    for (int i = 0; i < fields.size(); i++) {
      clauses.push_back(getFuzzyQuery(fields[i], termStr, minSimilarity));
    }
    return getMultiFieldQuery(clauses);
  }
  return QueryParser::getFuzzyQuery(field, termStr, minSimilarity);
}

shared_ptr<Query> MultiFieldQueryParser::getPrefixQuery(
    const wstring &field, const wstring &termStr) 
{
  if (field == L"") {
    deque<std::shared_ptr<Query>> clauses = deque<std::shared_ptr<Query>>();
    for (int i = 0; i < fields.size(); i++) {
      clauses.push_back(getPrefixQuery(fields[i], termStr));
    }
    return getMultiFieldQuery(clauses);
  }
  return QueryParser::getPrefixQuery(field, termStr);
}

shared_ptr<Query> MultiFieldQueryParser::getWildcardQuery(
    const wstring &field, const wstring &termStr) 
{
  if (field == L"") {
    deque<std::shared_ptr<Query>> clauses = deque<std::shared_ptr<Query>>();
    for (int i = 0; i < fields.size(); i++) {
      clauses.push_back(getWildcardQuery(fields[i], termStr));
    }
    return getMultiFieldQuery(clauses);
  }
  return QueryParser::getWildcardQuery(field, termStr);
}

shared_ptr<Query>
MultiFieldQueryParser::getRangeQuery(const wstring &field, const wstring &part1,
                                     const wstring &part2, bool startInclusive,
                                     bool endInclusive) 
{
  if (field == L"") {
    deque<std::shared_ptr<Query>> clauses = deque<std::shared_ptr<Query>>();
    for (int i = 0; i < fields.size(); i++) {
      clauses.push_back(
          getRangeQuery(fields[i], part1, part2, startInclusive, endInclusive));
    }
    return getMultiFieldQuery(clauses);
  }
  return QueryParser::getRangeQuery(field, part1, part2, startInclusive,
                                    endInclusive);
}

shared_ptr<Query> MultiFieldQueryParser::getRegexpQuery(
    const wstring &field, const wstring &termStr) 
{
  if (field == L"") {
    deque<std::shared_ptr<Query>> clauses = deque<std::shared_ptr<Query>>();
    for (int i = 0; i < fields.size(); i++) {
      clauses.push_back(getRegexpQuery(fields[i], termStr));
    }
    return getMultiFieldQuery(clauses);
  }
  return QueryParser::getRegexpQuery(field, termStr);
}

shared_ptr<Query> MultiFieldQueryParser::getMultiFieldQuery(
    deque<std::shared_ptr<Query>> &queries) 
{
  if (queries.empty()) {
    return nullptr; // all clause words were filtered away by the analyzer.
  }
  shared_ptr<BooleanQuery::Builder> query = newBooleanQuery();
  for (auto sub : queries) {
    query->add(sub, BooleanClause::Occur::SHOULD);
  }
  return query->build();
}

shared_ptr<Query> MultiFieldQueryParser::parse(
    std::deque<wstring> &queries, std::deque<wstring> &fields,
    shared_ptr<Analyzer> analyzer) 
{
  if (queries.size() != fields.size()) {
    throw invalid_argument(L"queries.length != fields.length");
  }
  shared_ptr<BooleanQuery::Builder> bQuery =
      make_shared<BooleanQuery::Builder>();
  for (int i = 0; i < fields.size(); i++) {
    shared_ptr<QueryParser> qp = make_shared<QueryParser>(fields[i], analyzer);
    shared_ptr<Query> q = qp->parse(queries[i]);
    if (q != nullptr &&
        (!(std::dynamic_pointer_cast<BooleanQuery>(q) != nullptr) ||
         (std::static_pointer_cast<BooleanQuery>(q))->clauses().size() > 0)) {
      bQuery->add(q, BooleanClause::Occur::SHOULD);
    }
  }
  return bQuery->build();
}

shared_ptr<Query> MultiFieldQueryParser::parse(
    const wstring &query, std::deque<wstring> &fields,
    std::deque<BooleanClause::Occur> &flags,
    shared_ptr<Analyzer> analyzer) 
{
  if (fields.size() != flags.size()) {
    throw invalid_argument(L"fields.length != flags.length");
  }
  shared_ptr<BooleanQuery::Builder> bQuery =
      make_shared<BooleanQuery::Builder>();
  for (int i = 0; i < fields.size(); i++) {
    shared_ptr<QueryParser> qp = make_shared<QueryParser>(fields[i], analyzer);
    shared_ptr<Query> q = qp->parse(query);
    if (q != nullptr &&
        (!(std::dynamic_pointer_cast<BooleanQuery>(q) != nullptr) ||
         (std::static_pointer_cast<BooleanQuery>(q))->clauses().size() > 0)) {
      bQuery->add(q, flags[i]);
    }
  }
  return bQuery->build();
}

shared_ptr<Query> MultiFieldQueryParser::parse(
    std::deque<wstring> &queries, std::deque<wstring> &fields,
    std::deque<BooleanClause::Occur> &flags,
    shared_ptr<Analyzer> analyzer) 
{
  if (!(queries.size() == fields.size() && queries.size() == flags.size())) {
    throw invalid_argument(
        L"queries, fields, and flags array have have different length");
  }
  shared_ptr<BooleanQuery::Builder> bQuery =
      make_shared<BooleanQuery::Builder>();
  for (int i = 0; i < fields.size(); i++) {
    shared_ptr<QueryParser> qp = make_shared<QueryParser>(fields[i], analyzer);
    shared_ptr<Query> q = qp->parse(queries[i]);
    if (q != nullptr &&
        (!(std::dynamic_pointer_cast<BooleanQuery>(q) != nullptr) ||
         (std::static_pointer_cast<BooleanQuery>(q))->clauses().size() > 0)) {
      bQuery->add(q, flags[i]);
    }
  }
  return bQuery->build();
}
} // namespace org::apache::lucene::queryparser::classic