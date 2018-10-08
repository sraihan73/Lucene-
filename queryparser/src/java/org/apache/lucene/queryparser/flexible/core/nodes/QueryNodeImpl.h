#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/QueryNode.h"

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

/**
 * A {@link QueryNodeImpl} is the default implementation of the interface
 * {@link QueryNode}
 */
class QueryNodeImpl : public std::enable_shared_from_this<QueryNodeImpl>,
                      public QueryNode,
                      public Cloneable
{
  GET_CLASS_NAME(QueryNodeImpl)

  /* index default field */
  // TODO remove PLAINTEXT_FIELD_NAME replacing it with configuration APIs
public:
  static const std::wstring PLAINTEXT_FIELD_NAME;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool isLeaf_ = true;

  std::unordered_map<std::wstring, std::any> tags =
      std::unordered_map<std::wstring, std::any>();

  std::deque<std::shared_ptr<QueryNode>> clauses;

protected:
  virtual void allocate();

public:
  void add(std::shared_ptr<QueryNode> child) override;

  void add(std::deque<std::shared_ptr<QueryNode>> &children) override;

  bool isLeaf() override;

  void set(std::deque<std::shared_ptr<QueryNode>> &children) override;

  std::shared_ptr<QueryNode>
  cloneTree()  override;

  std::shared_ptr<QueryNode> clone()  override;

protected:
  virtual void setLeaf(bool isLeaf);

  /**
   * @return a List for QueryNode object. Returns null, for nodes that do not
   *         contain children. All leaf Nodes return null.
   */
public:
  std::deque<std::shared_ptr<QueryNode>> getChildren() override;

  void setTag(const std::wstring &tagName, std::any value) override;

  void unsetTag(const std::wstring &tagName) override;

  /** verify if a node contains a tag */
  bool containsTag(const std::wstring &tagName) override;

  std::any getTag(const std::wstring &tagName) override;

private:
  std::shared_ptr<QueryNode> parent = nullptr;

  void setParent(std::shared_ptr<QueryNode> parent);

public:
  std::shared_ptr<QueryNode> getParent() override;

protected:
  virtual bool isRoot();

  /**
   * If set to true the the method toQueryString will not write field names
   */
  bool toQueryStringIgnoreFields = false;

  /**
   * This method is use toQueryString to detect if fld is the default field
   *
   * @param fld - field name
   * @return true if fld is the default field
   */
  // TODO: remove this method, it's commonly used by {@link
  // #toQueryString(org.apache.lucene.queryParser.core.parser.EscapeQuerySyntax)}
  // to figure out what is the default field, however, {@link
  // #toQueryString(org.apache.lucene.queryParser.core.parser.EscapeQuerySyntax)}
  // should receive the default field value directly by parameter
  virtual bool isDefaultField(std::shared_ptr<std::wstring> fld);

  /**
   * Every implementation of this class should return pseudo xml like this:
   *
   * For FieldQueryNode: &lt;field start='1' end='2' field='subject'
   * text='foo'/&gt;
   *
   * @see org.apache.lucene.queryparser.flexible.core.nodes.QueryNode#toString()
   */
public:
  virtual std::wstring toString();

  /**
   * Returns a map_obj containing all tags attached to this query node.
   *
   * @return a map_obj containing all tags attached to this query node
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @SuppressWarnings("unchecked") public
  // java.util.Map<std::wstring, Object> getTagMap()
  std::unordered_map<std::wstring, std::any> getTagMap() override;

  void removeChildren(std::shared_ptr<QueryNode> childNode) override;

  void removeFromParent() override;

}; // end class QueryNodeImpl

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/
