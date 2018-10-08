#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spans
{
class SpanQuery;
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
namespace org::apache::lucene::queryparser::flexible::spans
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using StandardQueryBuilder = org::apache::lucene::queryparser::flexible::
    standard::builders::StandardQueryBuilder;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

/**
 * Sets up a query tree builder to build a span query tree from a query node
 * tree.<br>
 * <br>
 *
 * The defined map_obj is:<br>
 * - every BooleanQueryNode instance is delegated to the
 * SpanOrQueryNodeBuilder<br>
 * - every FieldQueryNode instance is delegated to the SpanTermQueryNodeBuilder
 * <br>
 *
 */
class SpansQueryTreeBuilder : public QueryTreeBuilder,
                              public StandardQueryBuilder
{
  GET_CLASS_NAME(SpansQueryTreeBuilder)

public:
  SpansQueryTreeBuilder();

  std::shared_ptr<SpanQuery> build(std::shared_ptr<QueryNode> queryTree) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<SpansQueryTreeBuilder> shared_from_this()
  {
    return std::static_pointer_cast<SpansQueryTreeBuilder>(
        org.apache.lucene.queryparser.flexible.core.builders
            .QueryTreeBuilder::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::spans
