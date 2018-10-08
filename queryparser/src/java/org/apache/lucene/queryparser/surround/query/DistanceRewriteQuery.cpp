using namespace std;

#include "DistanceRewriteQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Query = org::apache::lucene::search::Query;

DistanceRewriteQuery::DistanceRewriteQuery(shared_ptr<DistanceQuery> srndQuery,
                                           const wstring &fieldName,
                                           shared_ptr<BasicQueryFactory> qf)
    : RewriteQuery<DistanceQuery>(srndQuery, fieldName, qf)
{
}

shared_ptr<Query>
DistanceRewriteQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  return srndQuery->getSpanNearQuery(reader, fieldName, qf);
}
} // namespace org::apache::lucene::queryparser::surround::query