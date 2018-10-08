#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
 * This processors process {@link TermRangeQueryNode}s. It reads the lower and
 * upper bounds value from the {@link TermRangeQueryNode} object and try
 * to parse their values using a {@link DateFormat}. If the values cannot be
 * parsed to a date value, it will only create the {@link TermRangeQueryNode}
 * using the non-parsed values. <br>
 * <br>
 * If a {@link ConfigurationKeys#LOCALE} is defined in the
 * {@link QueryConfigHandler} it will be used to parse the date, otherwise
 * {@link Locale#getDefault()} will be used. <br>
 * <br>
 * If a {@link ConfigurationKeys#DATE_RESOLUTION} is defined and the
 * {@link Resolution} is not <code>null</code> it will also be used to parse the
 * date value.
 *
 * @see ConfigurationKeys#DATE_RESOLUTION
 * @see ConfigurationKeys#LOCALE
 * @see TermRangeQueryNode
 */
class TermRangeQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(TermRangeQueryNodeProcessor)

public:
  TermRangeQueryNodeProcessor();

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<TermRangeQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<TermRangeQueryNodeProcessor>(
        org.apache.lucene.queryparser.flexible.core.processors
            .QueryNodeProcessorImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::standard::processors
