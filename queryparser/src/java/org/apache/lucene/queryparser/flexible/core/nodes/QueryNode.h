#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/parser/EscapeQuerySyntax.h"

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
namespace org::apache::lucene::queryparser::flexible::core::nodes
{

using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

/**
 * A {@link QueryNode} is a interface implemented by all nodes on a QueryNode
 * tree.
 */
class QueryNode
{
  GET_CLASS_NAME(QueryNode)

  /** convert to a query string understood by the query parser */
  // TODO: this interface might be changed in the future
public:
  virtual std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) = 0;

  /** for printing */
  std::wstring toString();
  override = 0;

  /** get Children nodes */
  virtual std::deque<std::shared_ptr<QueryNode>> getChildren() = 0;

  /** verify if a node is a Leaf node */
  virtual bool isLeaf() = 0;

  /** verify if a node contains a tag */
  virtual bool containsTag(const std::wstring &tagName) = 0;

  /**
   * Returns object stored under that tag name
   */
  virtual std::any getTag(const std::wstring &tagName) = 0;

  virtual std::shared_ptr<QueryNode> getParent() = 0;

  /**
   * Recursive clone the QueryNode tree The tags are not copied to the new tree
   * when you call the cloneTree() method
   *
   * @return the cloned tree
   */
  virtual std::shared_ptr<QueryNode> cloneTree() = 0;

  // Below are the methods that can change state of a QueryNode
  // Write Operations (not Thread Safe)

  // add a new child to a non Leaf node
  virtual void add(std::shared_ptr<QueryNode> child) = 0;

  virtual void add(std::deque<std::shared_ptr<QueryNode>> &children) = 0;

  // reset the children of a node
  virtual void set(std::deque<std::shared_ptr<QueryNode>> &children) = 0;

  /**
   * Associate the specified value with the specified tagName. If the tagName
   * already exists, the old value is replaced. The tagName and value cannot be
   * null. tagName will be converted to lowercase.
   */
  virtual void setTag(const std::wstring &tagName, std::any value) = 0;

  /**
   * Unset a tag. tagName will be converted to lowercase.
   */
  virtual void unsetTag(const std::wstring &tagName) = 0;

  /**
   * Returns a map_obj containing all tags attached to this query node.
   *
   * @return a map_obj containing all tags attached to this query node
   */
  virtual std::unordered_map<std::wstring, std::any> getTagMap() = 0;

  /**
   * Removes this query node from its parent.
   */
  virtual void removeFromParent() = 0;

  /**
   * Remove a child node
   * @param childNode Which child to remove
   */
  virtual void removeChildren(std::shared_ptr<QueryNode> childNode) = 0;
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/
