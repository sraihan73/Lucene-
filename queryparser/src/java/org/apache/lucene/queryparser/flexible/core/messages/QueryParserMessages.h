#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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
namespace org::apache::lucene::queryparser::flexible::core::messages
{

using NLS = org::apache::lucene::queryparser::flexible::messages::NLS;

/**
 * Flexible Query Parser message bundle class
 */
class QueryParserMessages : public NLS
{
  GET_CLASS_NAME(QueryParserMessages)

private:
  static const std::wstring BUNDLE_NAME;

  QueryParserMessages();

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static QueryParserMessages::StaticConstructor staticConstructor;

  // static string must match the strings in the property files.
public:
  static std::wstring INVALID_SYNTAX;
  static std::wstring INVALID_SYNTAX_CANNOT_PARSE;
  static std::wstring INVALID_SYNTAX_FUZZY_LIMITS;
  static std::wstring INVALID_SYNTAX_FUZZY_EDITS;
  static std::wstring INVALID_SYNTAX_ESCAPE_UNICODE_TRUNCATION;
  static std::wstring INVALID_SYNTAX_ESCAPE_CHARACTER;
  static std::wstring INVALID_SYNTAX_ESCAPE_NONE_HEX_UNICODE;
  static std::wstring NODE_ACTION_NOT_SUPPORTED;
  static std::wstring PARAMETER_VALUE_NOT_SUPPORTED;
  static std::wstring LUCENE_QUERY_CONVERSION_ERROR;
  static std::wstring EMPTY_MESSAGE;
  static std::wstring WILDCARD_NOT_SUPPORTED;
  static std::wstring TOO_MANY_BOOLEAN_CLAUSES;
  static std::wstring LEADING_WILDCARD_NOT_ALLOWED;
  static std::wstring COULD_NOT_PARSE_NUMBER;
  static std::wstring NUMBER_CLASS_NOT_SUPPORTED_BY_NUMERIC_RANGE_QUERY;
  static std::wstring UNSUPPORTED_NUMERIC_DATA_TYPE;
  static std::wstring NUMERIC_CANNOT_BE_EMPTY;

protected:
  std::shared_ptr<QueryParserMessages> shared_from_this()
  {
    return std::static_pointer_cast<QueryParserMessages>(
        org.apache.lucene.queryparser.flexible.messages
            .NLS::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::core::messages
