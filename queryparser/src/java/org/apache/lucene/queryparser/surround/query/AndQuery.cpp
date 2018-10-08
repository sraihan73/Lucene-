using namespace std;

#include "AndQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Query = org::apache::lucene::search::Query;
using BooleanClause = org::apache::lucene::search::BooleanClause;

AndQuery::AndQuery(deque<std::shared_ptr<SrndQuery>> &queries, bool inf,
                   const wstring &opName)
    : ComposedQuery(queries, inf, opName)
{
}

shared_ptr<Query>
AndQuery::makeLuceneQueryFieldNoBoost(const wstring &fieldName,
                                      shared_ptr<BasicQueryFactory> qf)
{
  return SrndBooleanQuery::makeBooleanQuery(
      makeLuceneSubQueriesField(fieldName, qf), BooleanClause::Occur::MUST);
}
} // namespace org::apache::lucene::queryparser::surround::query