#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class TermRangeQuery;
}

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
namespace org::apache::lucene::queryparser::flexible::standard::builders
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;

/**
 * Builds a {@link TermRangeQuery} object from a {@link TermRangeQueryNode}
 * object.
 */
class TermRangeQueryNodeBuilder
    : public std::enable_shared_from_this<TermRangeQueryNodeBuilder>,
      public StandardQueryBuilder
{
  GET_CLASS_NAME(TermRangeQueryNodeBuilder)

public:
  TermRangeQueryNodeBuilder();

  std::shared_ptr<TermRangeQuery> build(
      std::shared_ptr<QueryNode> queryNode)  override;
};

} // namespace org::apache::lucene::queryparser::flexible::standard::builders
