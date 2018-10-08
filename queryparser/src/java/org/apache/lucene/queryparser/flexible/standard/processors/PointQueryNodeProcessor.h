#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::queryparser::flexible::standard::processors
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;

/**
 * This processor is used to convert {@link FieldQueryNode}s to
 * {@link PointRangeQueryNode}s. It looks for
 * {@link ConfigurationKeys#POINTS_CONFIG} set in the {@link FieldConfig} of
 * every {@link FieldQueryNode} found. If
 * {@link ConfigurationKeys#POINTS_CONFIG} is found, it considers that
 * {@link FieldQueryNode} to be a numeric query and convert it to
 * {@link PointRangeQueryNode} with upper and lower inclusive and lower and
 * upper equals to the value represented by the {@link FieldQueryNode} converted
 * to {@link Number}. It means that <b>field:1</b> is converted to <b>field:[1
 * TO 1]</b>. <br>
 * <br>
 * Note that {@link FieldQueryNode}s children of a
 * {@link RangeQueryNode} are ignored.
 *
 * @see ConfigurationKeys#POINTS_CONFIG
 * @see FieldQueryNode
 * @see PointsConfig
 * @see PointQueryNode
 */
class PointQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(PointQueryNodeProcessor)

  /**
   * Constructs a {@link PointQueryNodeProcessor} object.
   */
public:
  PointQueryNodeProcessor();

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<PointQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<PointQueryNodeProcessor>(
        org.apache.lucene.queryparser.flexible.core.processors
            .QueryNodeProcessorImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::standard::processors
