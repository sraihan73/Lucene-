#pragma once
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/queryparser/xml/ParserException.h"

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
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using Query = org::apache::lucene::search::Query;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;

/**
 * UserInputQueryBuilder uses 1 of 2 strategies for thread-safe parsing:
 * 1) Synchronizing access to "parse" calls on a previously supplied QueryParser
 * or..
 * 2) creating a new QueryParser object for each parse request
 */
class UserInputQueryBuilder
    : public std::enable_shared_from_this<UserInputQueryBuilder>,
      public QueryBuilder
{
  GET_CLASS_NAME(UserInputQueryBuilder)

private:
  std::shared_ptr<QueryParser> unSafeParser;
  std::shared_ptr<Analyzer> analyzer;
  std::wstring defaultField;

  /**
   * This constructor has the disadvantage of not being able to change choice of
   * default field name
   *
   * @param parser thread un-safe query parser
   */
public:
  UserInputQueryBuilder(std::shared_ptr<QueryParser> parser);

  UserInputQueryBuilder(const std::wstring &defaultField,
                        std::shared_ptr<Analyzer> analyzer);

  /* (non-Javadoc)
   * @see
   * org.apache.lucene.xmlparser.QueryObjectBuilder#process(org.w3c.dom.Element)
   */

  std::shared_ptr<Query>
  getQuery(std::shared_ptr<Element> e)  override;

  /**
   * Method to create a QueryParser - designed to be overridden
   *
   * @return QueryParser
   */
protected:
  virtual std::shared_ptr<QueryParser>
  createQueryParser(const std::wstring &fieldName,
                    std::shared_ptr<Analyzer> analyzer);
};

} // #include  "core/src/java/org/apache/lucene/queryparser/xml/builders/
