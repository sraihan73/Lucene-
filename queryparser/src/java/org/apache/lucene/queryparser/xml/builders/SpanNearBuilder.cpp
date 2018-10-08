using namespace std;

#include "SpanNearBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;
using org::w3c::dom::Node;

SpanNearBuilder::SpanNearBuilder(shared_ptr<SpanQueryBuilder> factory)
    : factory(factory)
{
}

shared_ptr<SpanQuery>
SpanNearBuilder::getSpanQuery(shared_ptr<Element> e) 
{
  wstring slopString = DOMUtils::getAttributeOrFail(e, L"slop");
  int slop = stoi(slopString);
  bool inOrder = DOMUtils::getAttribute(e, L"inOrder", false);
  deque<std::shared_ptr<SpanQuery>> spans =
      deque<std::shared_ptr<SpanQuery>>();
  for (shared_ptr<Node> kid = e->getFirstChild(); kid != nullptr;
       kid = kid->getNextSibling()) {
    if (kid->getNodeType() == Node::ELEMENT_NODE) {
      spans.push_back(
          factory->getSpanQuery(std::static_pointer_cast<Element>(kid)));
    }
  }
  std::deque<std::shared_ptr<SpanQuery>> spanQueries =
      spans.toArray(std::deque<std::shared_ptr<SpanQuery>>(spans.size()));
  shared_ptr<SpanQuery> snq =
      make_shared<SpanNearQuery>(spanQueries, slop, inOrder);
  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  return make_shared<SpanBoostQuery>(snq, boost);
}
} // namespace org::apache::lucene::queryparser::xml::builders