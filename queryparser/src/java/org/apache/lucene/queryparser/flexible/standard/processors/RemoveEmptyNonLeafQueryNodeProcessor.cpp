using namespace std;

#include "RemoveEmptyNonLeafQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using GroupQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::GroupQueryNode;
using MatchNoDocsQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::MatchNoDocsQueryNode;
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;

RemoveEmptyNonLeafQueryNodeProcessor::RemoveEmptyNonLeafQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> RemoveEmptyNonLeafQueryNodeProcessor::process(
    shared_ptr<QueryNode> queryTree) 
{
  queryTree = QueryNodeProcessorImpl::process(queryTree);

  if (!queryTree->isLeaf()) {

    deque<std::shared_ptr<QueryNode>> children = queryTree->getChildren();

    if (children.empty() || children.empty()) {
      return make_shared<MatchNoDocsQueryNode>();
    }
  }

  return queryTree;
}

shared_ptr<QueryNode> RemoveEmptyNonLeafQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

shared_ptr<QueryNode> RemoveEmptyNonLeafQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>>
RemoveEmptyNonLeafQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  try {

    for (auto child : children) {

      if (!child->isLeaf()) {

        deque<std::shared_ptr<QueryNode>> grandChildren = child->getChildren();

        if (grandChildren.size() > 0 && grandChildren.size() > 0) {
          this->childrenBuffer.push_back(child);
        }

      } else {
        this->childrenBuffer.push_back(child);
      }
    }

    children.clear();
    children.insert(children.end(), this->childrenBuffer.begin(),
                    this->childrenBuffer.end());

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    this->childrenBuffer.clear();
  }

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors