using namespace std;

#include "BoostingQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using BoostingQuery = org::apache::lucene::queries::BoostingQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;
float BoostingQueryBuilder::DEFAULT_BOOST = 0.01f;

BoostingQueryBuilder::BoostingQueryBuilder(shared_ptr<QueryBuilder> factory)
    : factory(factory)
{
}

shared_ptr<Query>
BoostingQueryBuilder::getQuery(shared_ptr<Element> e) 
{
  shared_ptr<Element> mainQueryElem =
      DOMUtils::getChildByTagOrFail(e, L"Query");
  mainQueryElem = DOMUtils::getFirstChildOrFail(mainQueryElem);
  shared_ptr<Query> mainQuery = factory->getQuery(mainQueryElem);

  shared_ptr<Element> boostQueryElem =
      DOMUtils::getChildByTagOrFail(e, L"BoostQuery");
  float boost = DOMUtils::getAttribute(boostQueryElem, L"boost", DEFAULT_BOOST);
  boostQueryElem = DOMUtils::getFirstChildOrFail(boostQueryElem);
  shared_ptr<Query> boostQuery = factory->getQuery(boostQueryElem);

  shared_ptr<Query> bq =
      make_shared<BoostingQuery>(mainQuery, boostQuery, boost);

  boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  if (boost != 1.0f) {
    return make_shared<BoostQuery>(bq, boost);
  }
  return bq;
}
} // namespace org::apache::lucene::queryparser::xml::builders