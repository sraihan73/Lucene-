using namespace std;

#include "FuzzyLikeThisQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using FuzzyLikeThisQuery =
    org::apache::lucene::sandbox::queries::FuzzyLikeThisQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using Query = org::apache::lucene::search::Query;
using org::w3c::dom::Element;
using org::w3c::dom::NodeList;
const float FuzzyLikeThisQueryBuilder::DEFAULT_MIN_SIMILARITY =
    org::apache::lucene::search::FuzzyQuery::defaultMinSimilarity;

FuzzyLikeThisQueryBuilder::FuzzyLikeThisQueryBuilder(
    shared_ptr<Analyzer> analyzer)
    : analyzer(analyzer)
{
}

shared_ptr<Query> FuzzyLikeThisQueryBuilder::getQuery(
    shared_ptr<Element> e) 
{
  shared_ptr<NodeList> nl = e->getElementsByTagName(L"Field");
  int maxNumTerms =
      DOMUtils::getAttribute(e, L"maxNumTerms", DEFAULT_MAX_NUM_TERMS);
  shared_ptr<FuzzyLikeThisQuery> fbq =
      make_shared<FuzzyLikeThisQuery>(maxNumTerms, analyzer);
  fbq->setIgnoreTF(DOMUtils::getAttribute(e, L"ignoreTF", DEFAULT_IGNORE_TF));

  constexpr int nlLen = nl->getLength();
  for (int i = 0; i < nlLen; i++) {
    shared_ptr<Element> fieldElem =
        std::static_pointer_cast<Element>(nl->item(i));
    float minSimilarity = DOMUtils::getAttribute(fieldElem, L"minSimilarity",
                                                 DEFAULT_MIN_SIMILARITY);
    int prefixLength = DOMUtils::getAttribute(fieldElem, L"prefixLength",
                                              DEFAULT_PREFIX_LENGTH);
    wstring fieldName =
        DOMUtils::getAttributeWithInheritance(fieldElem, L"fieldName");

    wstring value = DOMUtils::getText(fieldElem);
    fbq->addTerms(value, fieldName, minSimilarity, prefixLength);
  }

  shared_ptr<Query> q = fbq;
  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  if (boost != 1.0f) {
    q = make_shared<BoostQuery>(fbq, boost);
  }
  return q;
}
} // namespace org::apache::lucene::queryparser::xml::builders