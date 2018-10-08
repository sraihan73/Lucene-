#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::nodes
{
class FieldQueryNode;
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
namespace org::apache::lucene::queryparser::flexible::standard::nodes
{

using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;

/**
 * This query node represents a range query composed by {@link FieldQueryNode}
 * bounds, which means the bound values are strings.
 *
 * @see FieldQueryNode
 * @see AbstractRangeQueryNode
 */
class TermRangeQueryNode
    : public AbstractRangeQueryNode<std::shared_ptr<FieldQueryNode>>
{
  GET_CLASS_NAME(TermRangeQueryNode)

  /**
   * Constructs a {@link TermRangeQueryNode} object using the given
   * {@link FieldQueryNode} as its bounds.
   *
   * @param lower the lower bound
   * @param upper the upper bound
   * @param lowerInclusive <code>true</code> if the lower bound is inclusive,
   * otherwise, <code>false</code>
   * @param upperInclusive <code>true</code> if the upper bound is inclusive,
   * otherwise, <code>false</code>
   */
public:
  TermRangeQueryNode(std::shared_ptr<FieldQueryNode> lower,
                     std::shared_ptr<FieldQueryNode> upper, bool lowerInclusive,
                     bool upperInclusive);

protected:
  std::shared_ptr<TermRangeQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<TermRangeQueryNode>(
        AbstractRangeQueryNode<org.apache.lucene.queryparser.flexible.core.nodes
                                   .FieldQueryNode>::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::standard::nodes
