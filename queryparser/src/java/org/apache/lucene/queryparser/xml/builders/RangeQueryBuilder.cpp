using namespace std;

#include "RangeQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using Query = org::apache::lucene::search::Query;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using org::w3c::dom::Element;

shared_ptr<Query>
RangeQueryBuilder::getQuery(shared_ptr<Element> e) 
{
  wstring fieldName = DOMUtils::getAttributeWithInheritance(e, L"fieldName");

  wstring lowerTerm = e->getAttribute(L"lowerTerm");
  wstring upperTerm = e->getAttribute(L"upperTerm");
  bool includeLower = DOMUtils::getAttribute(e, L"includeLower", true);
  bool includeUpper = DOMUtils::getAttribute(e, L"includeUpper", true);
  return TermRangeQuery::newStringRange(fieldName, lowerTerm, upperTerm,
                                        includeLower, includeUpper);
}
} // namespace org::apache::lucene::queryparser::xml::builders