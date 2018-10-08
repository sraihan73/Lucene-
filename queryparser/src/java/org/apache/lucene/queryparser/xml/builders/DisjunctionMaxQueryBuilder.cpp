using namespace std;

#include "DisjunctionMaxQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using Query = org::apache::lucene::search::Query;
using org::w3c::dom::Element;
using org::w3c::dom::Node;
using org::w3c::dom::NodeList;

DisjunctionMaxQueryBuilder::DisjunctionMaxQueryBuilder(
    shared_ptr<QueryBuilder> factory)
    : factory(factory)
{
}

shared_ptr<Query> DisjunctionMaxQueryBuilder::getQuery(
    shared_ptr<Element> e) 
{
  float tieBreaker = DOMUtils::getAttribute(e, L"tieBreaker", 0.0f);

  deque<std::shared_ptr<Query>> disjuncts = deque<std::shared_ptr<Query>>();
  shared_ptr<NodeList> nl = e->getChildNodes();
  constexpr int nlLen = nl->getLength();
  for (int i = 0; i < nlLen; i++) {
    shared_ptr<Node> node = nl->item(i);
    if (std::dynamic_pointer_cast<Element>(node) !=
        nullptr) { // all elements are disjuncts.
      shared_ptr<Element> queryElem = std::static_pointer_cast<Element>(node);
      shared_ptr<Query> q = factory->getQuery(queryElem);
      disjuncts.push_back(q);
    }
  }

  shared_ptr<Query> q = make_shared<DisjunctionMaxQuery>(disjuncts, tieBreaker);
  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  if (boost != 1.0f) {
    q = make_shared<BoostQuery>(q, boost);
  }
  return q;
}
} // namespace org::apache::lucene::queryparser::xml::builders