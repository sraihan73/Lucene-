using namespace std;

#include "SrndBooleanQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Query = org::apache::lucene::search::Query;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BooleanClause = org::apache::lucene::search::BooleanClause;

void SrndBooleanQuery::addQueriesToBoolean(
    shared_ptr<BooleanQuery::Builder> bq,
    deque<std::shared_ptr<Query>> &queries, BooleanClause::Occur occur)
{
  for (int i = 0; i < queries.size(); i++) {
    bq->add(queries[i], occur);
  }
}

shared_ptr<Query>
SrndBooleanQuery::makeBooleanQuery(deque<std::shared_ptr<Query>> &queries,
                                   BooleanClause::Occur occur)
{
  if (queries.size() <= 1) {
    throw make_shared<AssertionError>(L"Too few subqueries: " + queries.size());
  }
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  addQueriesToBoolean(bq, queries.subList(0, queries.size()), occur);
  return bq->build();
}
} // namespace org::apache::lucene::queryparser::surround::query