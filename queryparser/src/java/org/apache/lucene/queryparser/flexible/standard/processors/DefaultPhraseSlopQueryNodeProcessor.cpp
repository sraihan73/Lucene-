using namespace std;

#include "DefaultPhraseSlopQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using SlopQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::SlopQueryNode;
using TokenizedPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    core::nodes::TokenizedPhraseQueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using MultiPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::MultiPhraseQueryNode;

DefaultPhraseSlopQueryNodeProcessor::DefaultPhraseSlopQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> DefaultPhraseSlopQueryNodeProcessor::process(
    shared_ptr<QueryNode> queryTree) 
{
  shared_ptr<QueryConfigHandler> queryConfig = getQueryConfigHandler();

  if (queryConfig != nullptr) {
    optional<int> defaultPhraseSlop =
        queryConfig->get(ConfigurationKeys::PHRASE_SLOP);

    if (defaultPhraseSlop) {
      this->defaultPhraseSlop = defaultPhraseSlop;

      return QueryNodeProcessorImpl::process(queryTree);
    }
  }

  return queryTree;
}

shared_ptr<QueryNode> DefaultPhraseSlopQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<TokenizedPhraseQueryNode>(node) != nullptr ||
      std::dynamic_pointer_cast<MultiPhraseQueryNode>(node) != nullptr) {

    return make_shared<SlopQueryNode>(node, this->defaultPhraseSlop);
  }

  return node;
}

shared_ptr<QueryNode> DefaultPhraseSlopQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<SlopQueryNode>(node) != nullptr) {
    this->processChildren_ = false;
  }

  return node;
}

void DefaultPhraseSlopQueryNodeProcessor::processChildren(
    shared_ptr<QueryNode> queryTree) 
{

  if (this->processChildren_) {
    QueryNodeProcessorImpl::processChildren(queryTree);

  } else {
    this->processChildren_ = true;
  }
}

deque<std::shared_ptr<QueryNode>>
DefaultPhraseSlopQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors