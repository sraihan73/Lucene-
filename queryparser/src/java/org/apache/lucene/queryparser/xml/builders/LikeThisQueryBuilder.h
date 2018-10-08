#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using Query = org::apache::lucene::search::Query;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;

/**
 * Builder for {@link MoreLikeThisQuery}
 */
class LikeThisQueryBuilder
    : public std::enable_shared_from_this<LikeThisQueryBuilder>,
      public QueryBuilder
{
  GET_CLASS_NAME(LikeThisQueryBuilder)

private:
  static constexpr int DEFAULT_MAX_QUERY_TERMS = 20;
  static constexpr int DEFAULT_MIN_TERM_FREQUENCY = 1;
  static constexpr float DEFAULT_PERCENT_TERMS_TO_MATCH =
      30; // default is a 3rd of selected terms must match

  const std::shared_ptr<Analyzer> analyzer;
  std::deque<std::wstring> const defaultFieldNames;

public:
  LikeThisQueryBuilder(std::shared_ptr<Analyzer> analyzer,
                       std::deque<std::wstring> &defaultFieldNames);

  /* (non-Javadoc)
   * @see
   * org.apache.lucene.xmlparser.QueryObjectBuilder#process(org.w3c.dom.Element)
   */
  std::shared_ptr<Query>
  getQuery(std::shared_ptr<Element> e)  override;
};

} // namespace org::apache::lucene::queryparser::xml::builders
