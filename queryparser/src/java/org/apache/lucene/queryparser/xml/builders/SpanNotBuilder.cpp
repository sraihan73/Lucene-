using namespace std;

#include "SpanNotBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanNotQuery = org::apache::lucene::search::spans::SpanNotQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;

SpanNotBuilder::SpanNotBuilder(shared_ptr<SpanQueryBuilder> factory)
    : factory(factory)
{
}

shared_ptr<SpanQuery>
SpanNotBuilder::getSpanQuery(shared_ptr<Element> e) 
{
  shared_ptr<Element> includeElem =
      DOMUtils::getChildByTagOrFail(e, L"Include");
  includeElem = DOMUtils::getFirstChildOrFail(includeElem);

  shared_ptr<Element> excludeElem =
      DOMUtils::getChildByTagOrFail(e, L"Exclude");
  excludeElem = DOMUtils::getFirstChildOrFail(excludeElem);

  shared_ptr<SpanQuery> include = factory->getSpanQuery(includeElem);
  shared_ptr<SpanQuery> exclude = factory->getSpanQuery(excludeElem);

  shared_ptr<SpanNotQuery> snq = make_shared<SpanNotQuery>(include, exclude);

  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  return make_shared<SpanBoostQuery>(snq, boost);
}
} // namespace org::apache::lucene::queryparser::xml::builders