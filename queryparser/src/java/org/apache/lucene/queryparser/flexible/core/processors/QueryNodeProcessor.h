#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::nodes
{
class QueryNode;
}

namespace org::apache::lucene::queryparser::flexible::core::config
{
class QueryConfigHandler;
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
 * A {@link QueryNodeProcessor} is an interface for classes that process a
 * {@link QueryNode} tree.
GET_CLASS_NAME(es)
 * <p>
 * The implementor of this class should perform some operation on a query node
 * tree and return the same or another query node tree.
 * <p>
 * It also may carry a {@link QueryConfigHandler} object that contains
 * configuration about the query represented by the query tree or the
 * collection/index where it's intended to be executed.
 * <p>
 * In case there is any {@link QueryConfigHandler} associated to the query tree
 * to be processed, it should be set using
 * {@link QueryNodeProcessor#setQueryConfigHandler(QueryConfigHandler)} before
 * {@link QueryNodeProcessor#process(QueryNode)} is invoked.
 *
 * @see QueryNode
 * @see QueryNodeProcessor
 * @see QueryConfigHandler
 */
class QueryNodeProcessor
{
  GET_CLASS_NAME(QueryNodeProcessor)

  /**
   * Processes a query node tree. It may return the same or another query tree.
   * I should never return <code>null</code>.
   *
   * @param queryTree
   *          tree root node
   *
   * @return the processed query tree
   */
public:
  virtual std::shared_ptr<QueryNode>
  process(std::shared_ptr<QueryNode> queryTree) = 0;

  /**
   * Sets the {@link QueryConfigHandler} associated to the query tree.
   */
  virtual void setQueryConfigHandler(
      std::shared_ptr<QueryConfigHandler> queryConfigHandler) = 0;

  /**
   * Returns the {@link QueryConfigHandler} associated to the query tree if any,
   * otherwise it returns <code>null</code>
   *
   * @return the {@link QueryConfigHandler} associated to the query tree if any,
   *         otherwise it returns <code>null</code>
   */
  virtual std::shared_ptr<QueryConfigHandler> getQueryConfigHandler() = 0;
};

} // namespace org::apache::lucene::queryparser::flexible::core::processors
