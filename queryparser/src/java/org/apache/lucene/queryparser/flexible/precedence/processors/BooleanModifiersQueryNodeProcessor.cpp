using namespace std;

#include "BooleanModifiersQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::precedence::processors
{
using PrecedenceQueryParser = org::apache::lucene::queryparser::flexible::
    precedence::PrecedenceQueryParser;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using Operator = org::apache::lucene::queryparser::flexible::standard::config::
    StandardQueryConfigHandler::Operator;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using namespace org::apache::lucene::queryparser::flexible::core::nodes;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using Modifier = org::apache::lucene::queryparser::flexible::core::nodes::
    ModifierQueryNode::Modifier;

BooleanModifiersQueryNodeProcessor::BooleanModifiersQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> BooleanModifiersQueryNodeProcessor::process(
    shared_ptr<QueryNode> queryTree) 
{
  StandardQueryConfigHandler::Operator op = getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::DEFAULT_OPERATOR);

  if (op == nullptr) {
    throw invalid_argument(
        L"StandardQueryConfigHandler.ConfigurationKeys.DEFAULT_OPERATOR should "
        L"be set on the QueryConfigHandler");
  }

  this->usingAnd = StandardQueryConfigHandler::Operator::AND == op;

  return QueryNodeProcessorImpl::process(queryTree);
}

shared_ptr<QueryNode> BooleanModifiersQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<AndQueryNode>(node) != nullptr) {
    this->childrenBuffer.clear();
    deque<std::shared_ptr<QueryNode>> children = node->getChildren();

    for (auto child : children) {
      this->childrenBuffer.push_back(
          applyModifier(child, ModifierQueryNode::Modifier::MOD_REQ));
    }

    node->set(this->childrenBuffer);

  } else if (this->usingAnd &&
             std::dynamic_pointer_cast<BooleanQueryNode>(node) != nullptr &&
             !(std::dynamic_pointer_cast<OrQueryNode>(node) != nullptr)) {

    this->childrenBuffer.clear();
    deque<std::shared_ptr<QueryNode>> children = node->getChildren();

    for (auto child : children) {
      this->childrenBuffer.push_back(
          applyModifier(child, ModifierQueryNode::Modifier::MOD_REQ));
    }

    node->set(this->childrenBuffer);
  }

  return node;
}

shared_ptr<QueryNode> BooleanModifiersQueryNodeProcessor::applyModifier(
    shared_ptr<QueryNode> node, ModifierQueryNode::Modifier mod)
{

  // check if modifier is not already defined and is default
  if (!(std::dynamic_pointer_cast<ModifierQueryNode>(node) != nullptr)) {
    return make_shared<ModifierQueryNode>(node, mod);

  } else {
    shared_ptr<ModifierQueryNode> modNode =
        std::static_pointer_cast<ModifierQueryNode>(node);

    if (modNode->getModifier() == ModifierQueryNode::Modifier::MOD_NONE) {
      return make_shared<ModifierQueryNode>(modNode->getChild(), mod);
    }
  }

  return node;
}

shared_ptr<QueryNode> BooleanModifiersQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{
  return node;
}

deque<std::shared_ptr<QueryNode>>
BooleanModifiersQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace
  // org::apache::lucene::queryparser::flexible::precedence::processors