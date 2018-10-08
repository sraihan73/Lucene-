#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::xml::builders
{
class SpanQueryBuilder;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::queryparser::xml
{
class ParserException;
}
namespace org::apache::lucene::search::spans
{
class SpanQuery;
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

using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;

/**
 * Factory for {@link SpanQueryBuilder}s
 */
class SpanQueryBuilderFactory
    : public std::enable_shared_from_this<SpanQueryBuilderFactory>,
      public SpanQueryBuilder
{
  GET_CLASS_NAME(SpanQueryBuilderFactory)

private:
  const std::unordered_map<std::wstring, std::shared_ptr<SpanQueryBuilder>>
      builders =
          std::unordered_map<std::wstring, std::shared_ptr<SpanQueryBuilder>>();

public:
  std::shared_ptr<Query>
  getQuery(std::shared_ptr<Element> e)  override;

  virtual void addBuilder(const std::wstring &nodeName,
                          std::shared_ptr<SpanQueryBuilder> builder);

  std::shared_ptr<SpanQuery>
  getSpanQuery(std::shared_ptr<Element> e)  override;
};

} // namespace org::apache::lucene::queryparser::xml::builders
