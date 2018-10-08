using namespace std;

#include "SpanNearClauseFactory.h"

namespace org::apache::lucene::queryparser::surround::query
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Query = org::apache::lucene::search::Query;
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;

SpanNearClauseFactory::SpanNearClauseFactory(shared_ptr<IndexReader> reader,
                                             const wstring &fieldName,
                                             shared_ptr<BasicQueryFactory> qf)
{
  this->reader = reader;
  this->fieldName = fieldName;
  this->weightBySpanQuery = unordered_map<std::shared_ptr<SpanQuery>, float>();
  this->qf = qf;
}

shared_ptr<IndexReader> SpanNearClauseFactory::getIndexReader()
{
  return reader;
}

wstring SpanNearClauseFactory::getFieldName() { return fieldName; }

shared_ptr<BasicQueryFactory> SpanNearClauseFactory::getBasicQueryFactory()
{
  return qf;
}

int SpanNearClauseFactory::size() { return weightBySpanQuery.size(); }

void SpanNearClauseFactory::clear() { weightBySpanQuery.clear(); }

void SpanNearClauseFactory::addSpanQueryWeighted(shared_ptr<SpanQuery> sq,
                                                 float weight)
{
  optional<float> w = weightBySpanQuery[sq];
  if (w) {
    w = static_cast<Float>(w.value() + weight);
  } else {
    w = static_cast<Float>(weight);
  }
  weightBySpanQuery.emplace(sq, w);
}

void SpanNearClauseFactory::addTermWeighted(shared_ptr<Term> t,
                                            float weight) 
{
  shared_ptr<SpanTermQuery> stq = qf->newSpanTermQuery(t);
  /* CHECKME: wrap in Hashable...? */
  addSpanQueryWeighted(stq, weight);
}

void SpanNearClauseFactory::addSpanQuery(shared_ptr<Query> q)
{
  if (q->getClass() == MatchNoDocsQuery::typeid) {
    return;
  }
  if (!(std::dynamic_pointer_cast<SpanQuery>(q) != nullptr)) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<AssertionError>(L"Expected SpanQuery: " +
                                      q->toString(getFieldName()));
  }
  float boost = 1.0f;
  if (std::dynamic_pointer_cast<SpanBoostQuery>(q) != nullptr) {
    shared_ptr<SpanBoostQuery> bq = std::static_pointer_cast<SpanBoostQuery>(q);
    boost = bq->getBoost();
    q = bq->getQuery();
  }
  addSpanQueryWeighted(std::static_pointer_cast<SpanQuery>(q), boost);
}

shared_ptr<SpanQuery> SpanNearClauseFactory::makeSpanClause()
{
  std::deque<std::shared_ptr<SpanQuery>> spanQueries(size());
  Iterator<std::shared_ptr<SpanQuery>> sqi = weightBySpanQuery.keySet().begin();
  int i = 0;
  while (sqi->hasNext()) {
    shared_ptr<SpanQuery> sq = sqi->next();
    float boost = weightBySpanQuery[sq];
    if (boost != 1.0f) {
      sq = make_shared<SpanBoostQuery>(sq, boost);
    }
    spanQueries[i++] = sq;
    sqi++;
  }

  if (spanQueries.size() == 1) {
    return spanQueries[0];
  } else {
    return make_shared<SpanOrQuery>(spanQueries);
  }
}
} // namespace org::apache::lucene::queryparser::surround::query