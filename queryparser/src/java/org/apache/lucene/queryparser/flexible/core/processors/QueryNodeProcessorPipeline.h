#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <deque>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::processors
{
class QueryNodeProcessor;
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
 * A {@link QueryNodeProcessorPipeline} class should be used to build a query
 * node processor pipeline.
 *
 * When a query node tree is processed using this class, it passes the query
 * node tree to each processor on the pipeline and the result from each
 * processor is passed to the next one, always following the order the
 * processors were on the pipeline.
 *
 * When a {@link QueryConfigHandler} object is set on a
 * {@link QueryNodeProcessorPipeline}, it also takes care of setting this
 * {@link QueryConfigHandler} on all processor on pipeline.
 *
 */
class QueryNodeProcessorPipeline
    : public std::enable_shared_from_this<QueryNodeProcessorPipeline>,
      public QueryNodeProcessor,
      public std::deque<std::shared_ptr<QueryNodeProcessor>>
{
  GET_CLASS_NAME(QueryNodeProcessorPipeline)

private:
  std::deque<std::shared_ptr<QueryNodeProcessor>> processors =
      std::deque<std::shared_ptr<QueryNodeProcessor>>();

  std::shared_ptr<QueryConfigHandler> queryConfig;

  /**
   * Constructs an empty query node processor pipeline.
   */
public:
  QueryNodeProcessorPipeline();

  /**
   * Constructs with a {@link QueryConfigHandler} object.
   */
  QueryNodeProcessorPipeline(
      std::shared_ptr<QueryConfigHandler> queryConfigHandler);

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
   * For reference about this method check:
   * {@link QueryNodeProcessor#process(QueryNode)}.
   *
   * @param queryTree the query node tree to be processed
   *
   * @throws QueryNodeException if something goes wrong during the query node
   *         processing
   *
   * @see QueryNode
   */
  std::shared_ptr<QueryNode> process(
      std::shared_ptr<QueryNode> queryTree)  override;

  /**
   * For reference about this method check:
   * {@link QueryNodeProcessor#setQueryConfigHandler(QueryConfigHandler)}.
   *
   * @param queryConfigHandler the query configuration handler to be set.
   *
   * @see QueryNodeProcessor#getQueryConfigHandler()
   * @see QueryConfigHandler
   */
  void setQueryConfigHandler(
      std::shared_ptr<QueryConfigHandler> queryConfigHandler) override;

  /**
   * @see List#add(Object)
   */
  bool add(std::shared_ptr<QueryNodeProcessor> processor) override;

  /**
   * @see List#add(int, Object)
   */
  void add(int index, std::shared_ptr<QueryNodeProcessor> processor) override;

  /**
   * @see List#addAll(std::deque)
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override public bool addAll(std::deque<?
  // extends QueryNodeProcessor> c)
  bool addAll(std::shared_ptr<std::deque<T1>> c);

  /**
   * @see List#addAll(int, std::deque)
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override public bool addAll(int index,
  // std::deque<? extends QueryNodeProcessor> c)
  bool addAll(int index, std::shared_ptr<std::deque<T1>> c);

  /**
   * @see List#clear()
   */
  void clear() override;

  /**
   * @see List#contains(Object)
   */
  bool contains(std::any o) override;

  /**
   * @see List#containsAll(std::deque)
   */
  template <typename T1>
  bool containsAll(std::shared_ptr<std::deque<T1>> c);

  /**
   * @see List#get(int)
   */
  std::shared_ptr<QueryNodeProcessor> get(int index) override;

  /**
   * @see List#indexOf(Object)
   */
  int indexOf(std::any o) override;

  /**
   * @see List#isEmpty()
   */
  bool isEmpty() override;

  /**
   * @see List#iterator()
   */
  std::shared_ptr<Iterator<std::shared_ptr<QueryNodeProcessor>>>
  iterator() override;

  /**
   * @see List#lastIndexOf(Object)
   */
  int lastIndexOf(std::any o) override;

  /**
   * @see List#listIterator()
   */
  std::shared_ptr<ListIterator<std::shared_ptr<QueryNodeProcessor>>>
  listIterator() override;

  /**
   * @see List#listIterator(int)
   */
  std::shared_ptr<ListIterator<std::shared_ptr<QueryNodeProcessor>>>
  listIterator(int index) override;

  /**
   * @see List#remove(Object)
   */
  bool remove(std::any o) override;

  /**
   * @see List#remove(int)
   */
  std::shared_ptr<QueryNodeProcessor> remove(int index) override;

  /**
   * @see List#removeAll(std::deque)
   */
  template <typename T1>
  bool removeAll(std::shared_ptr<std::deque<T1>> c);

  /**
   * @see List#retainAll(std::deque)
   */
  template <typename T1>
  bool retainAll(std::shared_ptr<std::deque<T1>> c);

  /**
   * @see List#set(int, Object)
   */
  std::shared_ptr<QueryNodeProcessor>
  set(int index, std::shared_ptr<QueryNodeProcessor> processor) override;

  /**
   * @see List#size()
   */
  int size() override;

  /**
   * @see List#subList(int, int)
   */
  std::deque<std::shared_ptr<QueryNodeProcessor>>
  subList(int fromIndex, int toIndex) override;

  /**
   * @see List#toArray(Object[])
   */
  template <typename T>
  std::deque<T> toArray(std::deque<T> &array_);

  /**
   * @see List#toArray()
   */
  std::deque<std::any> toArray() override;
};

} // namespace org::apache::lucene::queryparser::flexible::core::processors
