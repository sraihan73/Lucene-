#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::ja::util
{

/**
 * Utility class for parsing CSV text
 */
class CSVUtil final : public std::enable_shared_from_this<CSVUtil>
{
  GET_CLASS_NAME(CSVUtil)
private:
  static constexpr wchar_t QUOTE = L'"';

  static constexpr wchar_t COMMA = L',';

  static const std::shared_ptr<Pattern> QUOTE_REPLACE_PATTERN;

  static const std::wstring ESCAPED_QUOTE;

  CSVUtil();

  /**
   * Parse CSV line
   * @param line line containing csv-encoded data
   * @return Array of values
   */
public:
  static std::deque<std::wstring> parse(const std::wstring &line);

private:
  static std::wstring unQuoteUnEscape(const std::wstring &original);

  /**
   * Quote and escape input value for CSV
   */
public:
  static std::wstring quoteEscape(const std::wstring &original);
};

} // namespace org::apache::lucene::analysis::ja::util
