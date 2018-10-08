using namespace std;

#include "QueryNodeProcessorImpl.h"

namespace org::apache::lucene::queryparser::flexible::core::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

QueryNodeProcessorImpl::QueryNodeProcessorImpl()
{
  // empty constructor
}

QueryNodeProcessorImpl::QueryNodeProcessorImpl(
    shared_ptr<QueryConfigHandler> queryConfigHandler)
{
  this->queryConfig = queryConfigHandler;
}

shared_ptr<QueryNode> QueryNodeProcessorImpl::process(
    shared_ptr<QueryNode> queryTree) 
{
  return processIteration(queryTree);
}

shared_ptr<QueryNode> QueryNodeProcessorImpl::processIteration(
    shared_ptr<QueryNode> queryTree) 
{
  queryTree = preProcessNode(queryTree);

  processChildren(queryTree);

  queryTree = postProcessNode(queryTree);

  return queryTree;
}

void QueryNodeProcessorImpl::processChildren(
    shared_ptr<QueryNode> queryTree) 
{

  deque<std::shared_ptr<QueryNode>> children = queryTree->getChildren();
  shared_ptr<ChildrenList> newChildren;

  if (children.size() > 0 && children.size() > 0) {

    newChildren = allocateChildrenList();

    try {

      for (auto child : children) {
        child = processIteration(child);

        if (child == nullptr) {
          throw make_shared<NullPointerException>();
        }

        newChildren->push_back(child);
      }

      deque<std::shared_ptr<QueryNode>> orderedChildrenList =
          setChildrenOrder(newChildren);

      queryTree->set(orderedChildrenList);

    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      newChildren->beingUsed = false;
    }
  }
}

shared_ptr<ChildrenList> QueryNodeProcessorImpl::allocateChildrenList()
{
  shared_ptr<ChildrenList> deque;

  for (auto auxList : this->childrenListPool) {

    if (!auxList->beingUsed) {
      deque = auxList;
      deque->clear();

      break;
    }
  }

  if (deque->empty()) {
    deque = make_shared<ChildrenList>();
    this->childrenListPool.push_back(deque);
  }

  deque->beingUsed = true;

  return deque;
}

void QueryNodeProcessorImpl::setQueryConfigHandler(
    shared_ptr<QueryConfigHandler> queryConfigHandler)
{
  this->queryConfig = queryConfigHandler;
}

shared_ptr<QueryConfigHandler> QueryNodeProcessorImpl::getQueryConfigHandler()
{
  return this->queryConfig;
}
} // namespace org::apache::lucene::queryparser::flexible::core::processors