using namespace std;

#include "BoostingTermBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Term = org::apache::lucene::index::Term;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using AveragePayloadFunction =
    org::apache::lucene::queries::payloads::AveragePayloadFunction;
using PayloadScoreQuery =
    org::apache::lucene::queries::payloads::PayloadScoreQuery;
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using org::w3c::dom::Element;

shared_ptr<SpanQuery>
BoostingTermBuilder::getSpanQuery(shared_ptr<Element> e) 
{
  wstring fieldName =
      DOMUtils::getAttributeWithInheritanceOrFail(e, L"fieldName");
  wstring value = DOMUtils::getNonBlankTextOrFail(e);

  shared_ptr<SpanQuery> btq = make_shared<PayloadScoreQuery>(
      make_shared<SpanTermQuery>(make_shared<Term>(fieldName, value)),
      make_shared<AveragePayloadFunction>());
  btq = make_shared<SpanBoostQuery>(btq,
                                    DOMUtils::getAttribute(e, L"boost", 1.0f));
  return btq;
}
} // namespace org::apache::lucene::queryparser::xml::builders