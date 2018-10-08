using namespace std;

#include "ConstantScoreQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using QueryBuilderFactory =
    org::apache::lucene::queryparser::xml::QueryBuilderFactory;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using Query = org::apache::lucene::search::Query;
using org::w3c::dom::Element;

ConstantScoreQueryBuilder::ConstantScoreQueryBuilder(
    shared_ptr<QueryBuilderFactory> queryFactory)
    : queryFactory(queryFactory)
{
}

shared_ptr<Query> ConstantScoreQueryBuilder::getQuery(
    shared_ptr<Element> e) 
{
  shared_ptr<Element> queryElem = DOMUtils::getFirstChildOrFail(e);

  shared_ptr<Query> q =
      make_shared<ConstantScoreQuery>(queryFactory->getQuery(queryElem));
  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  if (boost != 1.0f) {
    q = make_shared<BoostQuery>(q, boost);
  }
  return q;
}
} // namespace org::apache::lucene::queryparser::xml::builders