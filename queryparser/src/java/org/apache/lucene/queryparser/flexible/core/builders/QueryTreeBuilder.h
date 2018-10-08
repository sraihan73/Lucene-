#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::builders
{
class QueryBuilder;
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
namespace org::apache::lucene::queryparser::flexible::core::builders
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

/**
 * This class should be used when there is a builder for each type of node.
 *
 * The type of node may be defined in 2 different ways: - by the field name,
 * when the node implements the {@link FieldableNode} interface - by its class,
 * it keeps checking the class and all the interfaces and classes this class
 * implements/extends until it finds a builder for that class/interface
 *
 * This class always check if there is a builder for the field name before it
 * checks for the node class. So, field name builders have precedence over class
 * builders.
 *
 * When a builder is found for a node, it's called and the node is passed to the
 * builder. If the returned built object is not <code>null</code>, it's tagged
 * on the node using the tag {@link QueryTreeBuilder#QUERY_TREE_BUILDER_TAGID}.
 *
 * The children are usually built before the parent node. However, if a builder
 * associated to a node is an instance of {@link QueryTreeBuilder}, the node is
 * delegated to this builder and it's responsible to build the node and its
 * children.
 *
 * @see QueryBuilder
 */
class QueryTreeBuilder : public std::enable_shared_from_this<QueryTreeBuilder>,
                         public QueryBuilder
{
  GET_CLASS_NAME(QueryTreeBuilder)

  /**
   * This tag is used to tag the nodes in a query tree with the built objects
   * produced from their own associated builder.
   */
public:
  static const std::wstring QUERY_TREE_BUILDER_TAGID;

private:
  std::unordered_map<std::type_info, std::shared_ptr<QueryBuilder>>
      queryNodeBuilders;

  std::unordered_map<std::wstring, std::shared_ptr<QueryBuilder>>
      fieldNameBuilders;

  /**
   * {@link QueryTreeBuilder} constructor.
   */
public:
  QueryTreeBuilder();

  /**
   * Associates a field name with a builder.
   *
   * @param fieldName the field name
   * @param builder the builder to be associated
   */
  virtual void setBuilder(std::shared_ptr<std::wstring> fieldName,
                          std::shared_ptr<QueryBuilder> builder);

  /**
   * Associates a class with a builder
   *
   * @param queryNodeClass the class
   * @param builder the builder to be associated
   */
  virtual void setBuilder(std::type_info queryNodeClass,
                          std::shared_ptr<QueryBuilder> builder);

private:
  void process(std::shared_ptr<QueryNode> node) ;

  std::shared_ptr<QueryBuilder> getBuilder(std::shared_ptr<QueryNode> node);

  void
  processNode(std::shared_ptr<QueryNode> node,
              std::shared_ptr<QueryBuilder> builder) ;

  std::shared_ptr<QueryBuilder> getQueryBuilder(std::type_info clazz);

  /**
   * Builds some kind of object from a query tree. Each node in the query tree
   * is built using an specific builder associated to it.
   *
   * @param queryNode the query tree root node
   *
   * @return the built object
   *
   * @throws QueryNodeException if some node builder throws a
   *         {@link QueryNodeException} or if there is a node which had no
   *         builder associated to it
   */
public:
  std::any build(std::shared_ptr<QueryNode> queryNode) throw(
      QueryNodeException) override;
};

} // namespace org::apache::lucene::queryparser::flexible::core::builders
