#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class TermQuery;
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
using TermQuery = org::apache::lucene::search::TermQuery;

/**
 * Builds a {@link TermQuery} object from a {@link FieldQueryNode} object.
 */
class FieldQueryNodeBuilder
    : public std::enable_shared_from_this<FieldQueryNodeBuilder>,
      public StandardQueryBuilder
{
  GET_CLASS_NAME(FieldQueryNodeBuilder)

public:
  FieldQueryNodeBuilder();

  std::shared_ptr<TermQuery> build(std::shared_ptr<QueryNode> queryNode) throw(
      QueryNodeException) override;
};

} // namespace org::apache::lucene::queryparser::flexible::standard::builders
