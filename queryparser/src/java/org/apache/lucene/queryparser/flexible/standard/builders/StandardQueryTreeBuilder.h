#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
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
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using Query = org::apache::lucene::search::Query;

/**
 * This query tree builder only defines the necessary map_obj to build a
 * {@link Query} tree object. It should be used to generate a {@link Query} tree
 * object from a query node tree processed by a
 * {@link StandardQueryNodeProcessorPipeline}.
 *
 * @see QueryTreeBuilder
 * @see StandardQueryNodeProcessorPipeline
 */
class StandardQueryTreeBuilder : public QueryTreeBuilder,
                                 public StandardQueryBuilder
{
  GET_CLASS_NAME(StandardQueryTreeBuilder)

public:
  StandardQueryTreeBuilder();

  std::shared_ptr<Query> build(std::shared_ptr<QueryNode> queryNode) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<StandardQueryTreeBuilder> shared_from_this()
  {
    return std::static_pointer_cast<StandardQueryTreeBuilder>(
        org.apache.lucene.queryparser.flexible.core.builders
            .QueryTreeBuilder::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::standard::builders
