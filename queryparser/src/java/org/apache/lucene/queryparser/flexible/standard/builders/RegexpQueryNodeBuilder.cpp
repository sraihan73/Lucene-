using namespace std;

#include "RegexpQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using Term = org::apache::lucene::index::Term;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using RegexpQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::RegexpQueryNode;
using MultiTermRewriteMethodProcessor = org::apache::lucene::queryparser::
    flexible::standard::processors::MultiTermRewriteMethodProcessor;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using RegexpQuery = org::apache::lucene::search::RegexpQuery;

RegexpQueryNodeBuilder::RegexpQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<RegexpQuery> RegexpQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<RegexpQueryNode> regexpNode =
      std::static_pointer_cast<RegexpQueryNode>(queryNode);

  // TODO: make the maxStates configurable w/ a reasonable default
  // (QueryParserBase uses 10000)
  shared_ptr<RegexpQuery> q = make_shared<RegexpQuery>(make_shared<Term>(
      regexpNode->getFieldAsString(), regexpNode->textToBytesRef()));

  shared_ptr<MultiTermQuery::RewriteMethod> method =
      any_cast<std::shared_ptr<MultiTermQuery::RewriteMethod>>(
          queryNode->getTag(MultiTermRewriteMethodProcessor::TAG_ID));
  if (method != nullptr) {
    q->setRewriteMethod(method);
  }

  return q;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders