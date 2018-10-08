#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"

#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/QueryNode.h"
#include  "core/src/java/org/apache/lucene/search/BooleanClause.h"

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
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;

/**
 * Builds a {@link BooleanQuery} object from a {@link BooleanQueryNode} object.
 * Every children in the {@link BooleanQueryNode} object must be already tagged
 * using {@link QueryTreeBuilder#QUERY_TREE_BUILDER_TAGID} with a {@link Query}
 * object. <br>
 * <br>
 * It takes in consideration if the children is a {@link ModifierQueryNode} to
 * define the {@link BooleanClause}.
 */
class BooleanQueryNodeBuilder
    : public std::enable_shared_from_this<BooleanQueryNodeBuilder>,
      public StandardQueryBuilder
{
  GET_CLASS_NAME(BooleanQueryNodeBuilder)

public:
  BooleanQueryNodeBuilder();

  std::shared_ptr<BooleanQuery> build(
      std::shared_ptr<QueryNode> queryNode)  override;

private:
  static BooleanClause::Occur getModifierValue(std::shared_ptr<QueryNode> node);
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/builders/
