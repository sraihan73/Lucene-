using namespace std;

#include "NotQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Query = org::apache::lucene::search::Query;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BooleanClause = org::apache::lucene::search::BooleanClause;

NotQuery::NotQuery(deque<std::shared_ptr<SrndQuery>> &queries,
                   const wstring &opName)
    : ComposedQuery(queries, true, opName)
{
}

shared_ptr<Query>
NotQuery::makeLuceneQueryFieldNoBoost(const wstring &fieldName,
                                      shared_ptr<BasicQueryFactory> qf)
{
  deque<std::shared_ptr<Query>> luceneSubQueries =
      makeLuceneSubQueriesField(fieldName, qf);
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(luceneSubQueries[0], BooleanClause::Occur::MUST);
  SrndBooleanQuery::addQueriesToBoolean(
      bq, luceneSubQueries.subList(1, luceneSubQueries.size()),
      BooleanClause::Occur::MUST_NOT);
  return bq->build();
}
} // namespace org::apache::lucene::queryparser::surround::query