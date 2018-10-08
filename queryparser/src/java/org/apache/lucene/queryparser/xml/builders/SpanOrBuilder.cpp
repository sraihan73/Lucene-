using namespace std;

#include "SpanOrBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;
using org::w3c::dom::Node;

SpanOrBuilder::SpanOrBuilder(shared_ptr<SpanQueryBuilder> factory)
    : factory(factory)
{
}

shared_ptr<SpanQuery>
SpanOrBuilder::getSpanQuery(shared_ptr<Element> e) 
{
  deque<std::shared_ptr<SpanQuery>> clausesList =
      deque<std::shared_ptr<SpanQuery>>();
  for (shared_ptr<Node> kid = e->getFirstChild(); kid != nullptr;
       kid = kid->getNextSibling()) {
    if (kid->getNodeType() == Node::ELEMENT_NODE) {
      shared_ptr<SpanQuery> clause =
          factory->getSpanQuery(std::static_pointer_cast<Element>(kid));
      clausesList.push_back(clause);
    }
  }
  std::deque<std::shared_ptr<SpanQuery>> clauses = clausesList.toArray(
      std::deque<std::shared_ptr<SpanQuery>>(clausesList.size()));
  shared_ptr<SpanOrQuery> soq = make_shared<SpanOrQuery>(clauses);
  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  return make_shared<SpanBoostQuery>(soq, boost);
}
} // namespace org::apache::lucene::queryparser::xml::builders