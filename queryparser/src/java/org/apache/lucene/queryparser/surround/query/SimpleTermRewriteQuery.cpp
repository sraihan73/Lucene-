using namespace std;

#include "SimpleTermRewriteQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using IndexReader = org::apache::lucene::index::IndexReader;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Query = org::apache::lucene::search::Query;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using Term = org::apache::lucene::index::Term;

SimpleTermRewriteQuery::SimpleTermRewriteQuery(shared_ptr<SimpleTerm> srndQuery,
                                               const wstring &fieldName,
                                               shared_ptr<BasicQueryFactory> qf)
    : RewriteQuery<SimpleTerm>(srndQuery, fieldName, qf)
{
}

shared_ptr<Query> SimpleTermRewriteQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  const deque<std::shared_ptr<Query>> luceneSubQueries =
      deque<std::shared_ptr<Query>>();
  srndQuery->visitMatchingTerms(
      reader, fieldName,
      make_shared<MatchingTermVisitorAnonymousInnerClass>(shared_from_this(),
                                                          luceneSubQueries));
  return (luceneSubQueries.empty())
             ? make_shared<MatchNoDocsQuery>()
             : (luceneSubQueries.size() == 1)
                   ? luceneSubQueries[0]
                   : SrndBooleanQuery::makeBooleanQuery(
                         luceneSubQueries,
                         BooleanClause::Occur::SHOULD); // OR the subquery terms
}

SimpleTermRewriteQuery::MatchingTermVisitorAnonymousInnerClass::
    MatchingTermVisitorAnonymousInnerClass(
        shared_ptr<SimpleTermRewriteQuery> outerInstance,
        deque<std::shared_ptr<Query>> &luceneSubQueries)
{
  this->outerInstance = outerInstance;
  this->luceneSubQueries = luceneSubQueries;
}

void SimpleTermRewriteQuery::MatchingTermVisitorAnonymousInnerClass::
    visitMatchingTerm(shared_ptr<Term> term) 
{
  luceneSubQueries.push_back(outerInstance->qf->newTermQuery(term));
}
} // namespace org::apache::lucene::queryparser::surround::query