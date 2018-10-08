using namespace std;

#include "SpanTermBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Term = org::apache::lucene::index::Term;
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;

shared_ptr<SpanQuery>
SpanTermBuilder::getSpanQuery(shared_ptr<Element> e) 
{
  wstring fieldName =
      DOMUtils::getAttributeWithInheritanceOrFail(e, L"fieldName");
  wstring value = DOMUtils::getNonBlankTextOrFail(e);
  shared_ptr<SpanTermQuery> stq =
      make_shared<SpanTermQuery>(make_shared<Term>(fieldName, value));

  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  return make_shared<SpanBoostQuery>(stq, boost);
}
} // namespace org::apache::lucene::queryparser::xml::builders