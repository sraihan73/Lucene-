#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
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
using QueryBuilder =
    org::apache::lucene::queryparser::flexible::core::builders::QueryBuilder;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using Query = org::apache::lucene::search::Query;

/**
 * This interface should be implemented by every class that wants to build
 * {@link Query} objects from {@link QueryNode} objects.
GET_CLASS_NAME(that)
 *
 * @see QueryBuilder
 * @see QueryTreeBuilder
 */
class StandardQueryBuilder : public QueryBuilder
{
  GET_CLASS_NAME(StandardQueryBuilder)

public:
  std::shared_ptr<Query> build(std::shared_ptr<QueryNode> queryNode);
  override = 0;
};

} // namespace org::apache::lucene::queryparser::flexible::standard::builders