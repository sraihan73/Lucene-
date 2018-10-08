#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core
{
class QueryNodeException;
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
namespace org::apache::lucene::queryparser::flexible::core::processors
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using namespace org::apache::lucene::queryparser::flexible::core::nodes;

/**
 * <p>
 * A {@link NoChildOptimizationQueryNodeProcessor} removes every
 * BooleanQueryNode, BoostQueryNode, TokenizedPhraseQueryNode or
 * ModifierQueryNode that do not have a valid children.
 * </p>
 * <p>
 * Example: When the children of these nodes are removed for any reason then the
 * nodes may become invalid.
 * </p>
 */
class NoChildOptimizationQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(NoChildOptimizationQueryNodeProcessor)

public:
  NoChildOptimizationQueryNodeProcessor();

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<NoChildOptimizationQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<NoChildOptimizationQueryNodeProcessor>(
        QueryNodeProcessorImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::core::processors
