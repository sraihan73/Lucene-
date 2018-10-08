using namespace std;

#include "NoChildOptimizationQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::core::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using namespace org::apache::lucene::queryparser::flexible::core::nodes;

NoChildOptimizationQueryNodeProcessor::NoChildOptimizationQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> NoChildOptimizationQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<BooleanQueryNode>(node) != nullptr ||
      std::dynamic_pointer_cast<BoostQueryNode>(node) != nullptr ||
      std::dynamic_pointer_cast<TokenizedPhraseQueryNode>(node) != nullptr ||
      std::dynamic_pointer_cast<ModifierQueryNode>(node) != nullptr) {

    deque<std::shared_ptr<QueryNode>> children = node->getChildren();

    if (children.size() > 0 && children.size() > 0) {

      for (auto child : children) {

        if (!(std::dynamic_pointer_cast<DeletedQueryNode>(child) != nullptr)) {
          return node;
        }
      }
    }

    return make_shared<MatchNoDocsQueryNode>();
  }

  return node;
}

shared_ptr<QueryNode> NoChildOptimizationQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>>
NoChildOptimizationQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::core::processors