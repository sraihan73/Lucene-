#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/nodes/PointQueryNode.h"

#include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/config/PointsConfig.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"

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

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using PointsConfig =
    org::apache::lucene::queryparser::flexible::standard::config::PointsConfig;

/**
 * This query node represents a range query composed by {@link PointQueryNode}
 * bounds, which means the bound values are {@link Number}s.
 *
 * @see PointQueryNode
 * @see AbstractRangeQueryNode
 */
class PointRangeQueryNode
    : public AbstractRangeQueryNode<std::shared_ptr<PointQueryNode>>
{
  GET_CLASS_NAME(PointRangeQueryNode)

public:
  std::shared_ptr<PointsConfig> numericConfig;

  /**
   * Constructs a {@link PointRangeQueryNode} object using the given
   * {@link PointQueryNode} as its bounds and {@link PointsConfig}.
   *
   * @param lower the lower bound
   * @param upper the upper bound
   * @param lowerInclusive <code>true</code> if the lower bound is inclusive,
   * otherwise, <code>false</code>
   * @param upperInclusive <code>true</code> if the upper bound is inclusive,
   * otherwise, <code>false</code>
   * @param numericConfig the {@link PointsConfig} that represents associated
   * with the upper and lower bounds
   *
   * @see #setBounds(PointQueryNode, PointQueryNode, bool, bool,
   * PointsConfig)
   */
  PointRangeQueryNode(
      std::shared_ptr<PointQueryNode> lower,
      std::shared_ptr<PointQueryNode> upper, bool lowerInclusive,
      bool upperInclusive,
      std::shared_ptr<PointsConfig> numericConfig) ;

  /**
   * Sets the upper and lower bounds of this range query node and the
   * {@link PointsConfig} associated with these bounds.
   *
   * @param lower the lower bound
   * @param upper the upper bound
   * @param lowerInclusive <code>true</code> if the lower bound is inclusive,
   * otherwise, <code>false</code>
   * @param upperInclusive <code>true</code> if the upper bound is inclusive,
   * otherwise, <code>false</code>
   * @param pointsConfig the {@link PointsConfig} that represents associated
   * with the upper and lower bounds
   *
   */
  virtual void setBounds(
      std::shared_ptr<PointQueryNode> lower,
      std::shared_ptr<PointQueryNode> upper, bool lowerInclusive,
      bool upperInclusive,
      std::shared_ptr<PointsConfig> pointsConfig) ;

  /**
   * Returns the {@link PointsConfig} associated with the lower and upper
   * bounds.
   *
   * @return the {@link PointsConfig} associated with the lower and upper bounds
   */
  virtual std::shared_ptr<PointsConfig> getPointsConfig();

  virtual std::wstring toString();

protected:
  std::shared_ptr<PointRangeQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<PointRangeQueryNode>(
        AbstractRangeQueryNode<PointQueryNode>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/nodes/
