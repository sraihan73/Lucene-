using namespace std;

#include "OrQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Query = org::apache::lucene::search::Query;
using BooleanClause = org::apache::lucene::search::BooleanClause;

OrQuery::OrQuery(deque<std::shared_ptr<SrndQuery>> &queries, bool infix,
                 const wstring &opName)
    : ComposedQuery(queries, infix, opName)
{
}

shared_ptr<Query>
OrQuery::makeLuceneQueryFieldNoBoost(const wstring &fieldName,
                                     shared_ptr<BasicQueryFactory> qf)
{
  return SrndBooleanQuery::makeBooleanQuery(
      makeLuceneSubQueriesField(fieldName, qf), BooleanClause::Occur::SHOULD);
}

wstring OrQuery::distanceSubQueryNotAllowed()
{
  Iterator<std::shared_ptr<SrndQuery>> sqi = getSubQueriesIterator();
  while (sqi->hasNext()) {
    shared_ptr<SrndQuery> leq = sqi->next();
    if (std::dynamic_pointer_cast<DistanceSubQuery>(leq) != nullptr) {
      wstring m = (std::static_pointer_cast<DistanceSubQuery>(leq))
                      ->distanceSubQueryNotAllowed();
      if (m != L"") {
        return m;
      }
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      return L"subquery not allowed: " + leq->toString();
    }
    sqi++;
  }
  return L"";
}

void OrQuery::addSpanQueries(shared_ptr<SpanNearClauseFactory> sncf) throw(
    IOException)
{
  Iterator<std::shared_ptr<SrndQuery>> sqi = getSubQueriesIterator();
  while (sqi->hasNext()) {
    shared_ptr<SrndQuery> s = sqi->next();
    (std::static_pointer_cast<DistanceSubQuery>(s))->addSpanQueries(sncf);
    sqi++;
  }
}
} // namespace org::apache::lucene::queryparser::surround::query