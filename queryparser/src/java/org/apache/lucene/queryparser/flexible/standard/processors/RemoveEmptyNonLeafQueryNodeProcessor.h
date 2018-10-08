#pragma once
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/QueryNode.h"

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
namespace org::apache::lucene::queryparser::flexible::standard::processors
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;

/**
 * This processor removes every {@link QueryNode} that is not a leaf and has not
 * children. If after processing the entire tree the root node is not a leaf and
 * has no children, a {@link MatchNoDocsQueryNode} object is returned.
 * <br>
 * This processor is used at the end of a pipeline to avoid invalid query node
 * tree structures like a {@link GroupQueryNode} or {@link ModifierQueryNode}
 * with no children.
 *
 * @see QueryNode
 * @see MatchNoDocsQueryNode
 */
class RemoveEmptyNonLeafQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(RemoveEmptyNonLeafQueryNodeProcessor)

private:
  std::deque<std::shared_ptr<QueryNode>> childrenBuffer =
      std::deque<std::shared_ptr<QueryNode>>();

public:
  RemoveEmptyNonLeafQueryNodeProcessor();

  std::shared_ptr<QueryNode> process(
      std::shared_ptr<QueryNode> queryTree)  override;

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<RemoveEmptyNonLeafQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<RemoveEmptyNonLeafQueryNodeProcessor>(
        org.apache.lucene.queryparser.flexible.core.processors
            .QueryNodeProcessorImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/processors/
