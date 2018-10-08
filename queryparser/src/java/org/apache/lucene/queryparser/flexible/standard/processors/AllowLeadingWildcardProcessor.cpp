using namespace std;

#include "AllowLeadingWildcardProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using UnescapedCharSequence = org::apache::lucene::queryparser::flexible::core::
    util::UnescapedCharSequence;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using WildcardQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::WildcardQueryNode;
using EscapeQuerySyntaxImpl = org::apache::lucene::queryparser::flexible::
    standard::parser::EscapeQuerySyntaxImpl;

AllowLeadingWildcardProcessor::AllowLeadingWildcardProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> AllowLeadingWildcardProcessor::process(
    shared_ptr<QueryNode> queryTree) 
{
  optional<bool> allowsLeadingWildcard =
      getQueryConfigHandler()->get(ConfigurationKeys::ALLOW_LEADING_WILDCARD);

  if (allowsLeadingWildcard) {

    if (!allowsLeadingWildcard) {
      return QueryNodeProcessorImpl::process(queryTree);
    }
  }

  return queryTree;
}

shared_ptr<QueryNode> AllowLeadingWildcardProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<WildcardQueryNode>(node) != nullptr) {
    shared_ptr<WildcardQueryNode> wildcardNode =
        std::static_pointer_cast<WildcardQueryNode>(node);

    if (wildcardNode->getText()->length() > 0) {

      // Validate if the wildcard was escaped
      if (UnescapedCharSequence::wasEscaped(wildcardNode->getText(), 0)) {
        return node;
      }

      switch (wildcardNode->getText()->charAt(0)) {
      case L'*':
      case L'?':
        throw make_shared<QueryNodeException>(make_shared<MessageImpl>(
            QueryParserMessages::LEADING_WILDCARD_NOT_ALLOWED,
            node->toQueryString(make_shared<EscapeQuerySyntaxImpl>())));
      }
    }
  }

  return node;
}

shared_ptr<QueryNode> AllowLeadingWildcardProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>>
AllowLeadingWildcardProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors