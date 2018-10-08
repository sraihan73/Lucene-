using namespace std;

#include "BooleanQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;
using org::w3c::dom::Node;
using org::w3c::dom::NodeList;

BooleanQueryBuilder::BooleanQueryBuilder(shared_ptr<QueryBuilder> factory)
    : factory(factory)
{
}

shared_ptr<Query>
BooleanQueryBuilder::getQuery(shared_ptr<Element> e) 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->setMinimumNumberShouldMatch(
      DOMUtils::getAttribute(e, L"minimumNumberShouldMatch", 0));

  shared_ptr<NodeList> nl = e->getChildNodes();
  constexpr int nlLen = nl->getLength();
  for (int i = 0; i < nlLen; i++) {
    shared_ptr<Node> node = nl->item(i);
    if (node->getNodeName().equals(L"Clause")) {
      shared_ptr<Element> clauseElem = std::static_pointer_cast<Element>(node);
      BooleanClause::Occur occurs = getOccursValue(clauseElem);

      shared_ptr<Element> clauseQuery =
          DOMUtils::getFirstChildOrFail(clauseElem);
      shared_ptr<Query> q = factory->getQuery(clauseQuery);
      bq->add(make_shared<BooleanClause>(q, occurs));
    }
  }

  shared_ptr<Query> q = bq->build();
  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  if (boost != 1.0f) {
    q = make_shared<BoostQuery>(q, boost);
  }
  return q;
}

BooleanClause::Occur BooleanQueryBuilder::getOccursValue(
    shared_ptr<Element> clauseElem) 
{
  wstring occs = clauseElem->getAttribute(L"occurs");
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  if (occs == L"" || (wstring(L"should")).equalsIgnoreCase(occs)) {
    return BooleanClause::Occur::SHOULD;
  }
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  else if ((wstring(L"must")).equalsIgnoreCase(occs)) {
    return BooleanClause::Occur::MUST;
  }
  // C++ TODO: The following Java case-insensitive std::wstring method call is not
  // converted:
  else if ((wstring(L"mustNot")).equalsIgnoreCase(occs)) {
    return BooleanClause::Occur::MUST_NOT;
  } else if (L"filter" == occs) {
    return BooleanClause::Occur::FILTER;
  }
  throw make_shared<ParserException>(
      L"Invalid value for \"occurs\" attribute of clause:" + occs);
}
} // namespace org::apache::lucene::queryparser::xml::builders