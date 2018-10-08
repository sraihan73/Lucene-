using namespace std;

#include "DistanceQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using IndexReader = org::apache::lucene::index::IndexReader;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Query = org::apache::lucene::search::Query;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

DistanceQuery::DistanceQuery(deque<std::shared_ptr<SrndQuery>> &queries,
                             bool infix, int opDistance, const wstring &opName,
                             bool ordered)
    : ComposedQuery(queries, infix, opName)
{
  this->opDistance = opDistance; // the distance indicated in the operator
  this->ordered = ordered;
}

int DistanceQuery::getOpDistance() { return opDistance; }

bool DistanceQuery::subQueriesOrdered() { return ordered; }

wstring DistanceQuery::distanceSubQueryNotAllowed()
{
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: java.util.Iterator<?> sqi = getSubQueriesIterator();
  Iterator < ? > sqi = getSubQueriesIterator();
  while (sqi->hasNext()) {
    any leq = sqi->next();
    if (std::dynamic_pointer_cast<DistanceSubQuery>(leq) != nullptr) {
      shared_ptr<DistanceSubQuery> dsq =
          any_cast<std::shared_ptr<DistanceSubQuery>>(leq);
      wstring m = dsq->distanceSubQueryNotAllowed();
      if (m != L"") {
        return m;
      }
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      return L"Operator " + getOperatorName() + L" does not allow subquery " +
             leq.toString();
    }
    sqi++;
  }
  return L""; // subqueries acceptable
}

void DistanceQuery::addSpanQueries(
    shared_ptr<SpanNearClauseFactory> sncf) 
{
  shared_ptr<Query> snq =
      getSpanNearQuery(sncf->getIndexReader(), sncf->getFieldName(),
                       sncf->getBasicQueryFactory());
  sncf->addSpanQuery(snq);
}

shared_ptr<Query> DistanceQuery::getSpanNearQuery(
    shared_ptr<IndexReader> reader, const wstring &fieldName,
    shared_ptr<BasicQueryFactory> qf) 
{
  std::deque<std::shared_ptr<SpanQuery>> spanClauses(getNrSubQueries());
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: java.util.Iterator<?> sqi = getSubQueriesIterator();
  Iterator < ? > sqi = getSubQueriesIterator();
  int qi = 0;
  while (sqi->hasNext()) {
    shared_ptr<SpanNearClauseFactory> sncf =
        make_shared<SpanNearClauseFactory>(reader, fieldName, qf);

    (std::static_pointer_cast<DistanceSubQuery>(sqi->next()))
        ->addSpanQueries(sncf);
    if (sncf->size() == 0) {   // distance operator requires all sub queries
      while (sqi->hasNext()) { // produce evt. error messages but ignore results
        (std::static_pointer_cast<DistanceSubQuery>(sqi->next()))
            ->addSpanQueries(sncf);
        sncf->clear();
        sqi++;
      }
      return make_shared<MatchNoDocsQuery>();
    }

    spanClauses[qi] = sncf->makeSpanClause();
    qi++;
    sqi++;
  }

  return make_shared<SpanNearQuery>(spanClauses, getOpDistance() - 1,
                                    subQueriesOrdered());
}

shared_ptr<Query>
DistanceQuery::makeLuceneQueryFieldNoBoost(const wstring &fieldName,
                                           shared_ptr<BasicQueryFactory> qf)
{
  return make_shared<DistanceRewriteQuery>(shared_from_this(), fieldName, qf);
}
} // namespace org::apache::lucene::queryparser::surround::query