using namespace std;

#include "RemoveDeletedQueryNodesProcessor.h"

namespace org::apache::lucene::queryparser::flexible::core::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using DeletedQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::DeletedQueryNode;
using MatchNoDocsQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::MatchNoDocsQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

RemoveDeletedQueryNodesProcessor::RemoveDeletedQueryNodesProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> RemoveDeletedQueryNodesProcessor::process(
    shared_ptr<QueryNode> queryTree) 
{
  queryTree = QueryNodeProcessorImpl::process(queryTree);

  if (std::dynamic_pointer_cast<DeletedQueryNode>(queryTree) != nullptr &&
      !(std::dynamic_pointer_cast<MatchNoDocsQueryNode>(queryTree) !=
        nullptr)) {

    return make_shared<MatchNoDocsQueryNode>();
  }

  return queryTree;
}

shared_ptr<QueryNode> RemoveDeletedQueryNodesProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (!node->isLeaf()) {
    deque<std::shared_ptr<QueryNode>> children = node->getChildren();
    bool removeBoolean = false;

    if (children.empty() || children.empty()) {
      removeBoolean = true;

    } else {
      removeBoolean = true;

      for (deque<std::shared_ptr<QueryNode>>::const_iterator it =
               children.begin();
           it != children.end(); ++it) {

        if (!(*std::dynamic_pointer_cast<DeletedQueryNode>(it) != nullptr)) {
          removeBoolean = false;
          break;
        }
      }
    }

    if (removeBoolean) {
      return make_shared<DeletedQueryNode>();
    }
  }

  return node;
}

deque<std::shared_ptr<QueryNode>>
RemoveDeletedQueryNodesProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  for (int i = 0; i < children.size(); i++) {

    if (std::dynamic_pointer_cast<DeletedQueryNode>(children[i]) != nullptr) {
      children.erase(children.begin() + i--);
    }
  }

  return children;
}

shared_ptr<QueryNode> RemoveDeletedQueryNodesProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}
} // namespace org::apache::lucene::queryparser::flexible::core::processors