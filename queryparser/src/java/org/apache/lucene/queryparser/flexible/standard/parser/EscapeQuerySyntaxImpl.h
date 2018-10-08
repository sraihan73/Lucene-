#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::util
{
class UnescapedCharSequence;
}

namespace org::apache::lucene::queryparser::flexible::standard::parser
{
class ParseException;
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
namespace org::apache::lucene::queryparser::flexible::standard::parser
{

using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using UnescapedCharSequence = org::apache::lucene::queryparser::flexible::core::
    util::UnescapedCharSequence;

/**
 * Implementation of {@link EscapeQuerySyntax} for the standard lucene
 * syntax.
 */
class EscapeQuerySyntaxImpl
    : public std::enable_shared_from_this<EscapeQuerySyntaxImpl>,
      public EscapeQuerySyntax
{
  GET_CLASS_NAME(EscapeQuerySyntaxImpl)

private:
  static std::deque<wchar_t> const wildcardChars;

  static std::deque<std::wstring> const escapableTermExtraFirstChars;

  static std::deque<std::wstring> const escapableTermChars;

  // TODO: check what to do with these "*", "?", "\\"
  static std::deque<std::wstring> const escapableQuotedChars;
  static std::deque<std::wstring> const escapableWhiteChars;
  static std::deque<std::wstring> const escapableWordTokens;

  static std::shared_ptr<std::wstring>
  escapeChar(std::shared_ptr<std::wstring> str, std::shared_ptr<Locale> locale);

  std::shared_ptr<std::wstring> escapeQuoted(std::shared_ptr<std::wstring> str,
                                             std::shared_ptr<Locale> locale);

  static std::shared_ptr<std::wstring>
  escapeTerm(std::shared_ptr<std::wstring> term,
             std::shared_ptr<Locale> locale);

  /**
   * replace with ignore case
   *
   * @param string
   *          string to get replaced
   * @param sequence1
   *          the old character sequence in lowercase
   * @param escapeChar
   *          the new character to prefix sequence1 in return string.
   * @return the new std::wstring
   */
  static std::shared_ptr<std::wstring>
  replaceIgnoreCase(std::shared_ptr<std::wstring> string,
                    std::shared_ptr<std::wstring> sequence1,
                    std::shared_ptr<std::wstring> escapeChar,
                    std::shared_ptr<Locale> locale);

  /**
   * escape all tokens that are part of the parser syntax on a given string
   *
   * @param str
   *          string to get replaced
   * @param locale
   *          locale to be used when performing string compares
   * @return the new std::wstring
   */
  static std::shared_ptr<std::wstring>
  escapeWhiteChar(std::shared_ptr<std::wstring> str,
                  std::shared_ptr<Locale> locale);

public:
  std::shared_ptr<std::wstring> escape(std::shared_ptr<std::wstring> text,
                                       std::shared_ptr<Locale> locale,
                                       Type type) override;

  /**
   * Returns a std::wstring where the escape char has been removed, or kept only once
   * if there was a double escape.
   *
   * Supports escaped unicode characters, e. g. translates <code>A</code> to
   * <code>A</code>.
   *
   */
  static std::shared_ptr<UnescapedCharSequence>
  discardEscapeChar(std::shared_ptr<std::wstring> input) ;

  /** Returns the numeric value of the hexadecimal character */
private:
  static int hexToInt(wchar_t c) ;
};

} // namespace org::apache::lucene::queryparser::flexible::standard::parser
