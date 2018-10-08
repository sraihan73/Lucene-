using namespace std;

#include "SpanFirstBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanFirstQuery = org::apache::lucene::search::spans::SpanFirstQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;

SpanFirstBuilder::SpanFirstBuilder(shared_ptr<SpanQueryBuilder> factory)
    : factory(factory)
{
}

shared_ptr<SpanQuery>
SpanFirstBuilder::getSpanQuery(shared_ptr<Element> e) 
{
  int end = DOMUtils::getAttribute(e, L"end", 1);
  shared_ptr<Element> child = DOMUtils::getFirstChildElement(e);
  shared_ptr<SpanQuery> q = factory->getSpanQuery(child);

  shared_ptr<SpanFirstQuery> sfq = make_shared<SpanFirstQuery>(q, end);

  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  return make_shared<SpanBoostQuery>(sfq, boost);
}
} // namespace org::apache::lucene::queryparser::xml::builders