using namespace std;

#include "WildcardQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using Term = org::apache::lucene::index::Term;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using WildcardQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::WildcardQueryNode;
using MultiTermRewriteMethodProcessor = org::apache::lucene::queryparser::
    flexible::standard::processors::MultiTermRewriteMethodProcessor;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;

WildcardQueryNodeBuilder::WildcardQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<WildcardQuery> WildcardQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<WildcardQueryNode> wildcardNode =
      std::static_pointer_cast<WildcardQueryNode>(queryNode);

  shared_ptr<WildcardQuery> q = make_shared<WildcardQuery>(make_shared<Term>(
      wildcardNode->getFieldAsString(), wildcardNode->getTextAsString()));

  shared_ptr<MultiTermQuery::RewriteMethod> method =
      any_cast<std::shared_ptr<MultiTermQuery::RewriteMethod>>(
          queryNode->getTag(MultiTermRewriteMethodProcessor::TAG_ID));
  if (method != nullptr) {
    q->setRewriteMethod(method);
  }

  return q;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders