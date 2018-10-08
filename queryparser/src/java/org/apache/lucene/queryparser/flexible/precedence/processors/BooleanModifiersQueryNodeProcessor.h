#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
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
namespace org::apache::lucene::queryparser::flexible::precedence::processors
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using namespace org::apache::lucene::queryparser::flexible::core::nodes;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;

/**
 * <p>
 * This processor is used to apply the correct {@link ModifierQueryNode} to
 * {@link BooleanQueryNode}s children.
 * </p>
 * <p>
 * It walks through the query node tree looking for {@link BooleanQueryNode}s.
 * If an {@link AndQueryNode} is found, every child, which is not a {@link
 * ModifierQueryNode} or the {@link ModifierQueryNode} is {@link
 * Modifier#MOD_NONE}, becomes a {@link Modifier#MOD_REQ}. For any other
 * {@link BooleanQueryNode} which is not an {@link OrQueryNode}, it checks the
 * default operator is {@link Operator#AND}, if it is, the same operation when
 * an {@link AndQueryNode} is found is applied to it.
 * </p>
 *
 * @see ConfigurationKeys#DEFAULT_OPERATOR
 * @see PrecedenceQueryParser#setDefaultOperator
 */
class BooleanModifiersQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(BooleanModifiersQueryNodeProcessor)

private:
  std::deque<std::shared_ptr<QueryNode>> childrenBuffer =
      std::deque<std::shared_ptr<QueryNode>>();

  std::optional<bool> usingAnd = false;

public:
  BooleanModifiersQueryNodeProcessor();

  std::shared_ptr<QueryNode> process(
      std::shared_ptr<QueryNode> queryTree)  override;

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

private:
  std::shared_ptr<QueryNode> applyModifier(std::shared_ptr<QueryNode> node,
                                           ModifierQueryNode::Modifier mod);

protected:
  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<BooleanModifiersQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<BooleanModifiersQueryNodeProcessor>(
        org.apache.lucene.queryparser.flexible.core.processors
            .QueryNodeProcessorImpl::shared_from_this());
  }
};

} // namespace
  // org::apache::lucene::queryparser::flexible::precedence::processors
