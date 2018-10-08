using namespace std;

#include "PrefixWildcardQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using Term = org::apache::lucene::index::Term;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using PrefixWildcardQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::PrefixWildcardQueryNode;
using MultiTermRewriteMethodProcessor = org::apache::lucene::queryparser::
    flexible::standard::processors::MultiTermRewriteMethodProcessor;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;

PrefixWildcardQueryNodeBuilder::PrefixWildcardQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<PrefixQuery> PrefixWildcardQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{

  shared_ptr<PrefixWildcardQueryNode> wildcardNode =
      std::static_pointer_cast<PrefixWildcardQueryNode>(queryNode);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring text = wildcardNode->getText()
                     ->substr(0, wildcardNode->getText()->length() - 1)
                     ->toString();
  shared_ptr<PrefixQuery> q = make_shared<PrefixQuery>(
      make_shared<Term>(wildcardNode->getFieldAsString(), text));

  shared_ptr<MultiTermQuery::RewriteMethod> method =
      any_cast<std::shared_ptr<MultiTermQuery::RewriteMethod>>(
          queryNode->getTag(MultiTermRewriteMethodProcessor::TAG_ID));
  if (method != nullptr) {
    q->setRewriteMethod(method);
  }

  return q;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders