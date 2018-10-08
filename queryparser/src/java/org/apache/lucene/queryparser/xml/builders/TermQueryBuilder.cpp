using namespace std;

#include "TermQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Term = org::apache::lucene::index::Term;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;

shared_ptr<Query>
TermQueryBuilder::getQuery(shared_ptr<Element> e) 
{
  wstring field = DOMUtils::getAttributeWithInheritanceOrFail(e, L"fieldName");
  wstring value = DOMUtils::getNonBlankTextOrFail(e);
  shared_ptr<Query> tq =
      make_shared<TermQuery>(make_shared<Term>(field, value));
  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  if (boost != 1.0f) {
    tq = make_shared<BoostQuery>(tq, boost);
  }
  return tq;
}
} // namespace org::apache::lucene::queryparser::xml::builders