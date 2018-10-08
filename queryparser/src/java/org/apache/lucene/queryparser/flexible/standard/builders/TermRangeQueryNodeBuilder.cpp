using namespace std;

#include "TermRangeQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using StringUtils =
    org::apache::lucene::queryparser::flexible::core::util::StringUtils;
using TermRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::TermRangeQueryNode;
using MultiTermRewriteMethodProcessor = org::apache::lucene::queryparser::
    flexible::standard::processors::MultiTermRewriteMethodProcessor;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;

TermRangeQueryNodeBuilder::TermRangeQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<TermRangeQuery> TermRangeQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<TermRangeQueryNode> rangeNode =
      std::static_pointer_cast<TermRangeQueryNode>(queryNode);
  shared_ptr<FieldQueryNode> upper = rangeNode->getUpperBound();
  shared_ptr<FieldQueryNode> lower = rangeNode->getLowerBound();

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring field = StringUtils::toString(rangeNode->getField());
  wstring lowerText = lower->getTextAsString();
  wstring upperText = upper->getTextAsString();

  if (lowerText.length() == 0) {
    lowerText = L"";
  }

  if (upperText.length() == 0) {
    upperText = L"";
  }

  shared_ptr<TermRangeQuery> rangeQuery = TermRangeQuery::newStringRange(
      field, lowerText, upperText, rangeNode->isLowerInclusive(),
      rangeNode->isUpperInclusive());

  shared_ptr<MultiTermQuery::RewriteMethod> method =
      any_cast<std::shared_ptr<MultiTermQuery::RewriteMethod>>(
          queryNode->getTag(MultiTermRewriteMethodProcessor::TAG_ID));
  if (method != nullptr) {
    rangeQuery->setRewriteMethod(method);
  }

  return rangeQuery;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders