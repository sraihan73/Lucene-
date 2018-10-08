using namespace std;

#include "PhraseSlopQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using SlopQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::SlopQueryNode;
using TokenizedPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    core::nodes::TokenizedPhraseQueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using MultiPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::MultiPhraseQueryNode;

PhraseSlopQueryNodeProcessor::PhraseSlopQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> PhraseSlopQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<SlopQueryNode>(node) != nullptr) {
    shared_ptr<SlopQueryNode> phraseSlopNode =
        std::static_pointer_cast<SlopQueryNode>(node);

    if (!(std::dynamic_pointer_cast<TokenizedPhraseQueryNode>(
              phraseSlopNode->getChild()) != nullptr) &&
        !(std::dynamic_pointer_cast<MultiPhraseQueryNode>(
              phraseSlopNode->getChild()) != nullptr)) {
      return phraseSlopNode->getChild();
    }
  }

  return node;
}

shared_ptr<QueryNode> PhraseSlopQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>>
PhraseSlopQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors