#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::queryparser::ext
{

using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;

/**
 * {@link ExtensionQuery} holds all query components extracted from the original
 * query string like the query field and the extension query string.
 *
 * @see Extensions
 * @see ExtendableQueryParser
 * @see ParserExtension
 */
class ExtensionQuery : public std::enable_shared_from_this<ExtensionQuery>
{
  GET_CLASS_NAME(ExtensionQuery)

private:
  const std::wstring field;
  const std::wstring rawQueryString;
  const std::shared_ptr<QueryParser> topLevelParser;

  /**
   * Creates a new {@link ExtensionQuery}
   *
   * @param field
   *          the query field
   * @param rawQueryString
   *          the raw extension query string
   */
public:
  ExtensionQuery(std::shared_ptr<QueryParser> topLevelParser,
                 const std::wstring &field, const std::wstring &rawQueryString);

  /**
   * Returns the query field
   *
   * @return the query field
   */
  virtual std::wstring getField();

  /**
   * Returns the raw extension query string
   *
   * @return the raw extension query string
   */
  virtual std::wstring getRawQueryString();

  /**
   * Returns the top level parser which created this {@link ExtensionQuery}
   * @return the top level parser which created this {@link ExtensionQuery}
   */
  virtual std::shared_ptr<QueryParser> getTopLevelParser();
};

} // #include  "core/src/java/org/apache/lucene/queryparser/ext/
