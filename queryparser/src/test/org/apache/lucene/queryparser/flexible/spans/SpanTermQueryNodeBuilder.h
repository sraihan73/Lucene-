#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanTermQuery.h"

#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/QueryNode.h"

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
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using StandardQueryBuilder = org::apache::lucene::queryparser::flexible::
    standard::builders::StandardQueryBuilder;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;

/**
 * This builder creates {@link SpanTermQuery}s from a {@link FieldQueryNode}
 * object.
 */
class SpanTermQueryNodeBuilder
    : public std::enable_shared_from_this<SpanTermQueryNodeBuilder>,
      public StandardQueryBuilder
{
  GET_CLASS_NAME(SpanTermQueryNodeBuilder)

public:
  std::shared_ptr<SpanTermQuery>
  build(std::shared_ptr<QueryNode> node)  override;
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/spans/
