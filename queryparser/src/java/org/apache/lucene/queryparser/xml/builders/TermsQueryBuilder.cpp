using namespace std;

#include "TermsQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;

TermsQueryBuilder::TermsQueryBuilder(shared_ptr<Analyzer> analyzer)
    : analyzer(analyzer)
{
}

shared_ptr<Query>
TermsQueryBuilder::getQuery(shared_ptr<Element> e) 
{
  wstring fieldName =
      DOMUtils::getAttributeWithInheritanceOrFail(e, L"fieldName");
  wstring text = DOMUtils::getNonBlankTextOrFail(e);

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->setMinimumNumberShouldMatch(
      DOMUtils::getAttribute(e, L"minimumNumberShouldMatch", 0));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream(fieldName, text))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(fieldName, text);
    try {
      shared_ptr<TermToBytesRefAttribute> termAtt =
          ts->addAttribute(TermToBytesRefAttribute::typeid);
      shared_ptr<Term> term = nullptr;
      ts->reset();
      while (ts->incrementToken()) {
        term = make_shared<Term>(fieldName,
                                 BytesRef::deepCopyOf(termAtt->getBytesRef()));
        bq->add(make_shared<BooleanClause>(make_shared<TermQuery>(term),
                                           BooleanClause::Occur::SHOULD));
      }
      ts->end();
    } catch (const IOException &ioe) {
      throw runtime_error(L"Error constructing terms from index:" + ioe);
    }
  }

  shared_ptr<Query> q = bq->build();
  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  return make_shared<BoostQuery>(q, boost);
}
} // namespace org::apache::lucene::queryparser::xml::builders