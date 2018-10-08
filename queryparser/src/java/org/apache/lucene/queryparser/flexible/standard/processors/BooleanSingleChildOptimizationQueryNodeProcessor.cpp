using namespace std;

#include "BooleanSingleChildOptimizationQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using Modifier = org::apache::lucene::queryparser::flexible::core::nodes::
    ModifierQueryNode::Modifier;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using BooleanModifierNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::BooleanModifierNode;

BooleanSingleChildOptimizationQueryNodeProcessor::
    BooleanSingleChildOptimizationQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode>
BooleanSingleChildOptimizationQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<BooleanQueryNode>(node) != nullptr) {
    deque<std::shared_ptr<QueryNode>> children = node->getChildren();

    if (children.size() > 0 && children.size() == 1) {
      shared_ptr<QueryNode> child = children[0];

      if (std::dynamic_pointer_cast<ModifierQueryNode>(child) != nullptr) {
        shared_ptr<ModifierQueryNode> modNode =
            std::static_pointer_cast<ModifierQueryNode>(child);

        if (std::dynamic_pointer_cast<BooleanModifierNode>(modNode) !=
                nullptr ||
            modNode->getModifier() == ModifierQueryNode::Modifier::MOD_NONE) {

          return child;
        }

      } else {
        return child;
      }
    }
  }

  return node;
}

shared_ptr<QueryNode>
BooleanSingleChildOptimizationQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>>
BooleanSingleChildOptimizationQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors