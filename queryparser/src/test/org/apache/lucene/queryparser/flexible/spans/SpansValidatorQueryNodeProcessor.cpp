using namespace std;

#include "SpansValidatorQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::spans
{
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using AndQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::AndQueryNode;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using OrQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::OrQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;

shared_ptr<QueryNode> SpansValidatorQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

shared_ptr<QueryNode> SpansValidatorQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (!((std::dynamic_pointer_cast<BooleanQueryNode>(node) != nullptr &&
         !(std::dynamic_pointer_cast<AndQueryNode>(node) != nullptr)) ||
        node->getClass() == FieldQueryNode::typeid)) {
    throw make_shared<QueryNodeException>(make_shared<MessageImpl>(
        QueryParserMessages::NODE_ACTION_NOT_SUPPORTED));
  }

  return node;
}

deque<std::shared_ptr<QueryNode>>
SpansValidatorQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::spans