using namespace std;

#include "MultiTermRewriteMethodProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using AbstractRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::AbstractRangeQueryNode;
using RegexpQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::RegexpQueryNode;
using WildcardQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::WildcardQueryNode;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
const wstring MultiTermRewriteMethodProcessor::TAG_ID =
    L"MultiTermRewriteMethodConfiguration";

shared_ptr<QueryNode>
MultiTermRewriteMethodProcessor::postProcessNode(shared_ptr<QueryNode> node)
{

  // set setMultiTermRewriteMethod for WildcardQueryNode and
  // PrefixWildcardQueryNode
  if (std::dynamic_pointer_cast<WildcardQueryNode>(node) != nullptr ||
      std::dynamic_pointer_cast<AbstractRangeQueryNode>(node) != nullptr ||
      std::dynamic_pointer_cast<RegexpQueryNode>(node) != nullptr) {

    shared_ptr<MultiTermQuery::RewriteMethod> rewriteMethod =
        getQueryConfigHandler()->get(
            ConfigurationKeys::MULTI_TERM_REWRITE_METHOD);

    if (rewriteMethod == nullptr) {
      // This should not happen, this configuration is set in the
      // StandardQueryConfigHandler
      throw invalid_argument(
          L"StandardQueryConfigHandler.ConfigurationKeys.MULTI_TERM_REWRITE_"
          L"METHOD should be set on the QueryConfigHandler");
    }

    // use a TAG to take the value to the Builder
    node->setTag(MultiTermRewriteMethodProcessor::TAG_ID, rewriteMethod);
  }

  return node;
}

shared_ptr<QueryNode>
MultiTermRewriteMethodProcessor::preProcessNode(shared_ptr<QueryNode> node)
{
  return node;
}

deque<std::shared_ptr<QueryNode>>
MultiTermRewriteMethodProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children)
{
  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors