#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
 * This processor is used to expand terms so the query looks for the same term
 * in different fields. It also boosts a query based on its field. <br>
 * <br>
 * This processor looks for every {@link FieldableNode} contained in the query
 * node tree. If a {@link FieldableNode} is found, it checks if there is a
 * {@link ConfigurationKeys#MULTI_FIELDS} defined in the {@link
 * QueryConfigHandler}. If there is, the {@link FieldableNode} is cloned N times
 * and the clones are added to a {@link BooleanQueryNode} together with the
 * original node. N is defined by the number of fields that it will be expanded
 * to. The
 * {@link BooleanQueryNode} is returned.
 *
 * @see ConfigurationKeys#MULTI_FIELDS
 */
class MultiFieldQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(MultiFieldQueryNodeProcessor)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool processChildren_ = true;

public:
  MultiFieldQueryNodeProcessor();

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  void processChildren(std::shared_ptr<QueryNode> queryTree) throw(
      QueryNodeException) override;

  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<MultiFieldQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<MultiFieldQueryNodeProcessor>(
        org.apache.lucene.queryparser.flexible.core.processors
            .QueryNodeProcessorImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::standard::processors