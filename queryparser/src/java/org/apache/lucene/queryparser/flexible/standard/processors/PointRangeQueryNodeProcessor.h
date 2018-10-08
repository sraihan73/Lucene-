#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"

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
namespace org::apache::lucene::queryparser::flexible::standard::processors
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;

/**
 * This processor is used to convert {@link TermRangeQueryNode}s to
 * {@link PointRangeQueryNode}s. It looks for
 * {@link ConfigurationKeys#POINTS_CONFIG} set in the {@link FieldConfig} of
 * every {@link TermRangeQueryNode} found. If
 * {@link ConfigurationKeys#POINTS_CONFIG} is found, it considers that
 * {@link TermRangeQueryNode} to be a numeric range query and convert it to
 * {@link PointRangeQueryNode}.
 *
 * @see ConfigurationKeys#POINTS_CONFIG
 * @see TermRangeQueryNode
 * @see PointsConfig
 * @see PointRangeQueryNode
 */
class PointRangeQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(PointRangeQueryNodeProcessor)

  /**
   * Constructs an empty {@link PointRangeQueryNodeProcessor} object.
   */
public:
  PointRangeQueryNodeProcessor();

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<PointRangeQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<PointRangeQueryNodeProcessor>(
        org.apache.lucene.queryparser.flexible.core.processors
            .QueryNodeProcessorImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/processors/
