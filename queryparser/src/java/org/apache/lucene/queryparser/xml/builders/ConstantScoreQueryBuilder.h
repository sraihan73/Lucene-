#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::xml
{
class QueryBuilderFactory;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::queryparser::xml
{
class ParserException;
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
namespace org::apache::lucene::queryparser::xml::builders
{

using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using QueryBuilderFactory =
    org::apache::lucene::queryparser::xml::QueryBuilderFactory;
using Query = org::apache::lucene::search::Query;
using org::w3c::dom::Element;
/**
 * Builder for {@link ConstantScoreQuery}
 */
class ConstantScoreQueryBuilder
    : public std::enable_shared_from_this<ConstantScoreQueryBuilder>,
      public QueryBuilder
{
  GET_CLASS_NAME(ConstantScoreQueryBuilder)

private:
  const std::shared_ptr<QueryBuilderFactory> queryFactory;

public:
  ConstantScoreQueryBuilder(std::shared_ptr<QueryBuilderFactory> queryFactory);

  std::shared_ptr<Query>
  getQuery(std::shared_ptr<Element> e)  override;
};

} // namespace org::apache::lucene::queryparser::xml::builders