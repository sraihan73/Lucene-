#pragma once
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::queryparser::flexible::core::parser
{

/**
 * A parser needs to implement {@link EscapeQuerySyntax} to allow the QueryNode
 * to escape the queries, when the toQueryString method is called.
 */
class EscapeQuerySyntax
{
  GET_CLASS_NAME(EscapeQuerySyntax)
  /**
   * Type of escaping: std::wstring for escaping syntax,
   * NORMAL for escaping reserved words (like AND) in terms
   */
public:
  enum class Type { GET_CLASS_NAME(Type) STRING, NORMAL };

  /**
   * @param text
   *          - text to be escaped
   * @param locale
   *          - locale for the current query
   * @param type
   *          - select the type of escape operation to use
   * @return escaped text
   */
public:
  virtual std::shared_ptr<std::wstring>
  escape(std::shared_ptr<std::wstring> text, std::shared_ptr<Locale> locale,
         Type type) = 0;
};

} // namespace org::apache::lucene::queryparser::flexible::core::parser
