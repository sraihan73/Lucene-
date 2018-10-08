#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"

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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;

/**
 * Assembles a QueryBuilder which uses Query objects from
 * Lucene's <code>sandbox</code> and <code>queries</code>
 * modules in addition to core queries.
 */
class CorePlusExtensionsParser : public CorePlusQueriesParser
{
  GET_CLASS_NAME(CorePlusExtensionsParser)

  /**
   * Construct an XML parser that uses a single instance QueryParser for
   * handling UserQuery tags - all parse operations are synchronized on this
   * parser
   *
   * @param parser A QueryParser which will be synchronized on during parse
   * calls.
   */
public:
  CorePlusExtensionsParser(std::shared_ptr<Analyzer> analyzer,
                           std::shared_ptr<QueryParser> parser);

  /**
   * Constructs an XML parser that creates a QueryParser for each UserQuery
   * request.
   *
   * @param defaultField The default field name used by QueryParsers constructed
   * for UserQuery tags
   */
  CorePlusExtensionsParser(const std::wstring &defaultField,
                           std::shared_ptr<Analyzer> analyzer);

private:
  CorePlusExtensionsParser(const std::wstring &defaultField,
                           std::shared_ptr<Analyzer> analyzer,
                           std::shared_ptr<QueryParser> parser);

protected:
  std::shared_ptr<CorePlusExtensionsParser> shared_from_this()
  {
    return std::static_pointer_cast<CorePlusExtensionsParser>(
        CorePlusQueriesParser::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/xml/
