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
 * A {@link BooleanQueryNode} represents a deque of elements which do not have an
 * explicit bool operator defined between them. It can be used to express a
 * bool query that intends to use the default bool operator.
 */
class BooleanQueryNode : public QueryNodeImpl
{
  GET_CLASS_NAME(BooleanQueryNode)

  /**
   * @param clauses
   *          - the query nodes to be and'ed
   */
public:
  BooleanQueryNode(std::deque<std::shared_ptr<QueryNode>> &clauses);

  virtual std::wstring toString();

  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

  std::shared_ptr<QueryNode>
  cloneTree()  override;

protected:
  std::shared_ptr<BooleanQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<BooleanQueryNode>(
        QueryNodeImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::core::nodes
