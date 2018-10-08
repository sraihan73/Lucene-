#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::xml
{
class QueryBuilder;
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
namespace org::apache::lucene::queryparser::xml
{

using Query = org::apache::lucene::search::Query;
using org::w3c::dom::Element;

/**
 * Factory for {@link QueryBuilder}
 */
class QueryBuilderFactory
    : public std::enable_shared_from_this<QueryBuilderFactory>,
      public QueryBuilder
{
  GET_CLASS_NAME(QueryBuilderFactory)

public:
  std::unordered_map<std::wstring, std::shared_ptr<QueryBuilder>> builders =
      std::unordered_map<std::wstring, std::shared_ptr<QueryBuilder>>();

  std::shared_ptr<Query>
  getQuery(std::shared_ptr<Element> n)  override;

  virtual void addBuilder(const std::wstring &nodeName,
                          std::shared_ptr<QueryBuilder> builder);

  virtual std::shared_ptr<QueryBuilder>
  getQueryBuilder(const std::wstring &nodeName);
};

} // namespace org::apache::lucene::queryparser::xml