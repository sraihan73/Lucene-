using namespace std;

#include "BooleanQuery2ModifierNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using AndQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::AndQueryNode;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using Modifier = org::apache::lucene::queryparser::flexible::core::nodes::
    ModifierQueryNode::Modifier;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessor = org::apache::lucene::queryparser::flexible::core::
    processors::QueryNodeProcessor;
using BooleanModifiersQueryNodeProcessor = org::apache::lucene::queryparser::
    flexible::precedence::processors::BooleanModifiersQueryNodeProcessor;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using Operator = org::apache::lucene::queryparser::flexible::standard::config::
    StandardQueryConfigHandler::Operator;
using BooleanModifierNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::BooleanModifierNode;
using StandardSyntaxParser = org::apache::lucene::queryparser::flexible::
    standard::parser::StandardSyntaxParser;
const wstring BooleanQuery2ModifierNodeProcessor::TAG_REMOVE = L"remove";
const wstring BooleanQuery2ModifierNodeProcessor::TAG_MODIFIER =
    L"wrapWithModifier";
const wstring BooleanQuery2ModifierNodeProcessor::TAG_BOOLEAN_ROOT =
    L"booleanRoot";

BooleanQuery2ModifierNodeProcessor::BooleanQuery2ModifierNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> BooleanQuery2ModifierNodeProcessor::process(
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

  return processIteration(queryTree);
}

void BooleanQuery2ModifierNodeProcessor::processChildren(
    shared_ptr<QueryNode> queryTree) 
{
  deque<std::shared_ptr<QueryNode>> children = queryTree->getChildren();
  if (children.size() > 0 && children.size() > 0) {
    for (auto child : children) {
      child = processIteration(child);
    }
  }
}

shared_ptr<QueryNode> BooleanQuery2ModifierNodeProcessor::processIteration(
    shared_ptr<QueryNode> queryTree) 
{
  queryTree = preProcessNode(queryTree);

  processChildren(queryTree);

  queryTree = postProcessNode(queryTree);

  return queryTree;
}

void BooleanQuery2ModifierNodeProcessor::fillChildrenBufferAndApplyModifiery(
    shared_ptr<QueryNode> parent)
{
  for (auto node : parent->getChildren()) {
    if (node->containsTag(TAG_REMOVE)) {
      fillChildrenBufferAndApplyModifiery(node);
    } else if (node->containsTag(TAG_MODIFIER)) {
      childrenBuffer.push_back(applyModifier(
          node,
          any_cast<ModifierQueryNode::Modifier>(node->getTag(TAG_MODIFIER))));
    } else {
      childrenBuffer.push_back(node);
    }
  }
}

shared_ptr<QueryNode> BooleanQuery2ModifierNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{
  if (node->containsTag(TAG_BOOLEAN_ROOT)) {
    this->childrenBuffer.clear();
    fillChildrenBufferAndApplyModifiery(node);
    node->set(childrenBuffer);
  }
  return node;
}

shared_ptr<QueryNode> BooleanQuery2ModifierNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{
  shared_ptr<QueryNode> parent = node->getParent();
  if (std::dynamic_pointer_cast<BooleanQueryNode>(node) != nullptr) {
    if (std::dynamic_pointer_cast<BooleanQueryNode>(parent) != nullptr) {
      node->setTag(TAG_REMOVE, Boolean::TRUE); // no precedence
    } else {
      node->setTag(TAG_BOOLEAN_ROOT, Boolean::TRUE);
    }
  } else if (std::dynamic_pointer_cast<BooleanQueryNode>(parent) != nullptr) {
    if ((std::dynamic_pointer_cast<AndQueryNode>(parent) != nullptr) ||
        (usingAnd && isDefaultBooleanQueryNode(parent))) {
      tagModifierButDoNotOverride(node, ModifierQueryNode::Modifier::MOD_REQ);
    }
  }
  return node;
}

bool BooleanQuery2ModifierNodeProcessor::isDefaultBooleanQueryNode(
    shared_ptr<QueryNode> toTest)
{
  return toTest != nullptr &&
         BooleanQueryNode::typeid->equals(toTest->getClass());
}

shared_ptr<QueryNode> BooleanQuery2ModifierNodeProcessor::applyModifier(
    shared_ptr<QueryNode> node, ModifierQueryNode::Modifier mod)
{

  // check if modifier is not already defined and is default
  if (!(std::dynamic_pointer_cast<ModifierQueryNode>(node) != nullptr)) {
    return make_shared<BooleanModifierNode>(node, mod);

  } else {
    shared_ptr<ModifierQueryNode> modNode =
        std::static_pointer_cast<ModifierQueryNode>(node);

    if (modNode->getModifier() == ModifierQueryNode::Modifier::MOD_NONE) {
      return make_shared<ModifierQueryNode>(modNode->getChild(), mod);
    }
  }

  return node;
}

void BooleanQuery2ModifierNodeProcessor::tagModifierButDoNotOverride(
    shared_ptr<QueryNode> node, ModifierQueryNode::Modifier mod)
{
  if (std::dynamic_pointer_cast<ModifierQueryNode>(node) != nullptr) {
    shared_ptr<ModifierQueryNode> modNode =
        std::static_pointer_cast<ModifierQueryNode>(node);
    if (modNode->getModifier() == ModifierQueryNode::Modifier::MOD_NONE) {
      node->setTag(TAG_MODIFIER, mod);
    }
  } else {
    node->setTag(TAG_MODIFIER, ModifierQueryNode::Modifier::MOD_REQ);
  }
}

void BooleanQuery2ModifierNodeProcessor::setQueryConfigHandler(
    shared_ptr<QueryConfigHandler> queryConfigHandler)
{
  this->queryConfigHandler = queryConfigHandler;
}

shared_ptr<QueryConfigHandler>
BooleanQuery2ModifierNodeProcessor::getQueryConfigHandler()
{
  return queryConfigHandler;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors