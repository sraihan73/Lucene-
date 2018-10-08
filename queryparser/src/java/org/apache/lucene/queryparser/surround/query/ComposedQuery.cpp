using namespace std;

#include "ComposedQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Query = org::apache::lucene::search::Query;

ComposedQuery::ComposedQuery(deque<std::shared_ptr<SrndQuery>> &qs,
                             bool operatorInfix, const wstring &opName)
{
  recompose(qs);
  this->operatorInfix = operatorInfix;
  this->opName = opName;
}

void ComposedQuery::recompose(deque<std::shared_ptr<SrndQuery>> &queries)
{
  if (queries.size() < 2) {
    throw make_shared<AssertionError>(L"Too few subqueries");
  }
  this->queries = queries;
}

wstring ComposedQuery::getOperatorName() { return opName; }

shared_ptr<Iterator<std::shared_ptr<SrndQuery>>>
ComposedQuery::getSubQueriesIterator()
{
  return queries.begin();
}

int ComposedQuery::getNrSubQueries() { return queries.size(); }

shared_ptr<SrndQuery> ComposedQuery::getSubQuery(int qn) { return queries[qn]; }

bool ComposedQuery::isOperatorInfix()
{
  return operatorInfix;
} // else prefix operator

deque<std::shared_ptr<Query>>
ComposedQuery::makeLuceneSubQueriesField(const wstring &fn,
                                         shared_ptr<BasicQueryFactory> qf)
{
  deque<std::shared_ptr<Query>> luceneSubQueries =
      deque<std::shared_ptr<Query>>();
  Iterator<std::shared_ptr<SrndQuery>> sqi = getSubQueriesIterator();
  while (sqi->hasNext()) {
    luceneSubQueries.push_back((sqi->next()).makeLuceneQueryField(fn, qf));
    sqi++;
  }
  return luceneSubQueries;
}

wstring ComposedQuery::toString()
{
  shared_ptr<StringBuilder> r = make_shared<StringBuilder>();
  if (isOperatorInfix()) {
    infixToString(r);
  } else {
    prefixToString(r);
  }
  weightToString(r);
  return r->toString();
}

wstring ComposedQuery::getPrefixSeparator() { return L", "; }

wstring ComposedQuery::getBracketOpen() { return L"("; }

wstring ComposedQuery::getBracketClose() { return L")"; }

void ComposedQuery::infixToString(shared_ptr<StringBuilder> r)
{
  /* Brackets are possibly redundant in the result. */
  Iterator<std::shared_ptr<SrndQuery>> sqi = getSubQueriesIterator();
  r->append(getBracketOpen());
  if (sqi->hasNext()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    r->append(sqi->next()->toString());
    while (sqi->hasNext()) {
      r->append(L" ");
      r->append(getOperatorName()); // infix operator
      r->append(L" ");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      r->append(sqi->next()->toString());
      sqi++;
    }
  }
  r->append(getBracketClose());
}

void ComposedQuery::prefixToString(shared_ptr<StringBuilder> r)
{
  Iterator<std::shared_ptr<SrndQuery>> sqi = getSubQueriesIterator();
  r->append(getOperatorName()); // prefix operator
  r->append(getBracketOpen());
  if (sqi->hasNext()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    r->append(sqi->next()->toString());
    while (sqi->hasNext()) {
      r->append(getPrefixSeparator());
      // C++ TODO: There is no native C++ equivalent to 'toString':
      r->append(sqi->next()->toString());
      sqi++;
    }
  }
  r->append(getBracketClose());
}

bool ComposedQuery::isFieldsSubQueryAcceptable()
{
  /* at least one subquery should be acceptable */
  Iterator<std::shared_ptr<SrndQuery>> sqi = getSubQueriesIterator();
  while (sqi->hasNext()) {
    if ((sqi->next()).isFieldsSubQueryAcceptable()) {
      return true;
    }
    sqi++;
  }
  return false;
}
} // namespace org::apache::lucene::queryparser::surround::query