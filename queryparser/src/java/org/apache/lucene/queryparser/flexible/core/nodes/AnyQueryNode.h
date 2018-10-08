#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::nodes
{
class QueryNode;
}

namespace org::apache::lucene::queryparser::flexible::core::parser
{
class EscapeQuerySyntax;
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
namespace org::apache::lucene::queryparser::flexible::core::nodes
{

using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

/**
 * A {@link AnyQueryNode} represents an ANY operator performed on a deque of
 * nodes.
 */
class AnyQueryNode : public AndQueryNode
{
  GET_CLASS_NAME(AnyQueryNode)
private:
  std::shared_ptr<std::wstring> field = nullptr;
  int minimumMatchingmElements = 0;

  /**
   * @param clauses
   *          - the query nodes to be or'ed
   */
public:
  AnyQueryNode(std::deque<std::shared_ptr<QueryNode>> &clauses,
               std::shared_ptr<std::wstring> field,
               int minimumMatchingElements);

  virtual int getMinimumMatchingElements();

  /**
   * returns null if the field was not specified
   *
   * @return the field
   */
  virtual std::shared_ptr<std::wstring> getField();

  /**
   * returns - null if the field was not specified
   *
   * @return the field as a std::wstring
   */
  virtual std::wstring getFieldAsString();

  /**
   * @param field
   *          - the field to set
   */
  virtual void setField(std::shared_ptr<std::wstring> field);

  std::shared_ptr<QueryNode>
  cloneTree()  override;

  virtual std::wstring toString();

  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

protected:
  std::shared_ptr<AnyQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<AnyQueryNode>(
        AndQueryNode::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::core::nodes
