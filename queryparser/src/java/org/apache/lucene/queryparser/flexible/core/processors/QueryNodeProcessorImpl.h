#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::processors
{
class ChildrenList;
}

namespace org::apache::lucene::queryparser::flexible::core::config
{
class QueryConfigHandler;
}
namespace org::apache::lucene::queryparser::flexible::core
{
class QueryNodeException;
}
namespace org::apache::lucene::queryparser::flexible::core::nodes
{
class QueryNode;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::queryparser::flexible::core::processors
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

/**
 * <p>
 * This is a default implementation for the {@link QueryNodeProcessor}
 * interface, it's an abstract class, so it should be extended by classes that
 * want to process a {@link QueryNode} tree.
GET_CLASS_NAME(,)
 * </p>
 * <p>
 * This class process {@link QueryNode}s from left to right in the tree. While
 * it's walking down the tree, for every node,
 * {@link #preProcessNode(QueryNode)} is invoked. After a node's children are
 * processed, {@link #postProcessNode(QueryNode)} is invoked for that node.
 * {@link #setChildrenOrder(List)} is invoked before
 * {@link #postProcessNode(QueryNode)} only if the node has at least one child,
 * in {@link #setChildrenOrder(List)} the implementor might redefine the
 * children order or remove any children from the children deque.
 * </p>
 * <p>
 * Here is an example about how it process the nodes:
 * </p>
 *
 * <pre>
 *      a
 *     / \
 *    b   e
 *   / \
 *  c   d
 * </pre>
 *
 * Here is the order the methods would be invoked for the tree described above:
 *
 * <pre>
 *      preProcessNode( a );
 *      preProcessNode( b );
 *      preProcessNode( c );
 *      postProcessNode( c );
 *      preProcessNode( d );
 *      postProcessNode( d );
 *      setChildrenOrder( bChildrenList );
 *      postProcessNode( b );
 *      preProcessNode( e );
 *      postProcessNode( e );
 *      setChildrenOrder( aChildrenList );
 *      postProcessNode( a )
 * </pre>
 *
 * @see
org.apache.lucene.queryparser.flexible.core.processors.QueryNodeProcessor
 */
class QueryNodeProcessorImpl
    : public std::enable_shared_from_this<QueryNodeProcessorImpl>,
      public QueryNodeProcessor
{
  GET_CLASS_NAME(QueryNodeProcessorImpl)

private:
  std::deque<std::shared_ptr<ChildrenList>> childrenListPool =
      std::deque<std::shared_ptr<ChildrenList>>();

  std::shared_ptr<QueryConfigHandler> queryConfig;

public:
  QueryNodeProcessorImpl();

  QueryNodeProcessorImpl(
      std::shared_ptr<QueryConfigHandler> queryConfigHandler);

  std::shared_ptr<QueryNode> process(
      std::shared_ptr<QueryNode> queryTree)  override;

private:
  std::shared_ptr<QueryNode> processIteration(
      std::shared_ptr<QueryNode> queryTree) ;

  /**
   * This method is called every time a child is processed.
   *
   * @param queryTree
   *          the query node child to be processed
   * @throws QueryNodeException
   *           if something goes wrong during the query node processing
   */
protected:
  virtual void processChildren(std::shared_ptr<QueryNode> queryTree) throw(
      QueryNodeException);

private:
  std::shared_ptr<ChildrenList> allocateChildrenList();

  /**
   * For reference about this method check:
   * {@link QueryNodeProcessor#setQueryConfigHandler(QueryConfigHandler)}.
   *
   * @param queryConfigHandler
   *          the query configuration handler to be set.
   *
   * @see QueryNodeProcessor#getQueryConfigHandler()
   * @see QueryConfigHandler
   */
public:
  void setQueryConfigHandler(
      std::shared_ptr<QueryConfigHandler> queryConfigHandler) override;

  /**
   * For reference about this method check:
   * {@link QueryNodeProcessor#getQueryConfigHandler()}.
   *
   * @return QueryConfigHandler the query configuration handler to be set.
   *
   * @see QueryNodeProcessor#setQueryConfigHandler(QueryConfigHandler)
   * @see QueryConfigHandler
   */
  std::shared_ptr<QueryConfigHandler> getQueryConfigHandler() override;

  /**
   * This method is invoked for every node when walking down the tree.
   *
   * @param node
   *          the query node to be pre-processed
   *
   * @return a query node
   *
   * @throws QueryNodeException
   *           if something goes wrong during the query node processing
   */
protected:
  virtual std::shared_ptr<QueryNode>
  preProcessNode(std::shared_ptr<QueryNode> node) = 0;

  /**
   * This method is invoked for every node when walking up the tree.
   *
   * @param node
   *          node the query node to be post-processed
   *
   * @return a query node
   *
   * @throws QueryNodeException
   *           if something goes wrong during the query node processing
   */
  virtual std::shared_ptr<QueryNode>
  postProcessNode(std::shared_ptr<QueryNode> node) = 0;

  /**
   * This method is invoked for every node that has at least on child. It's
   * invoked right before {@link #postProcessNode(QueryNode)} is invoked.
   *
   * @param children
   *          the deque containing all current node's children
   *
   * @return a new deque containing all children that should be set to the
   *         current node
   *
   * @throws QueryNodeException
   *           if something goes wrong during the query node processing
   */
  virtual std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) = 0;

private:
  class ChildrenList : public std::deque<std::shared_ptr<QueryNode>>
  {
    GET_CLASS_NAME(ChildrenList)

  public:
    bool beingUsed = false;

  protected:
    std::shared_ptr<ChildrenList> shared_from_this()
    {
      return std::static_pointer_cast<ChildrenList>(
          java.util.ArrayList<org.apache.lucene.queryparser.flexible.core.nodes
                                  .QueryNode>::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::queryparser::flexible::core::processors
