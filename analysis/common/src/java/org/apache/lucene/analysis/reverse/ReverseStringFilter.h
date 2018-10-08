#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis::reverse
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Reverse token string, for example "country" =&gt; "yrtnuoc".
 * <p>
 * If <code>marker</code> is supplied, then tokens will be also prepended by
 * that character. For example, with a marker of &#x5C;u0001, "country" =&gt;
 * "&#x5C;u0001yrtnuoc". This is useful when implementing efficient leading
 * wildcards search.
 */
class ReverseStringFilter final : public TokenFilter
{
  GET_CLASS_NAME(ReverseStringFilter)

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const wchar_t marker;
  static constexpr wchar_t NOMARKER = L'\uFFFF';

  /**
   * Example marker character: U+0001 (START OF HEADING)
   */
public:
  static constexpr wchar_t START_OF_HEADING_MARKER = L'\u0001';

  /**
   * Example marker character: U+001F (INFORMATION SEPARATOR ONE)
   */
  static constexpr wchar_t INFORMATION_SEPARATOR_MARKER = L'\u001F';

  /**
   * Example marker character: U+EC00 (PRIVATE USE AREA: EC00)
   */
  static constexpr wchar_t PUA_EC00_MARKER = L'\uEC00';

  /**
   * Example marker character: U+200F (RIGHT-TO-LEFT MARK)
   */
  static constexpr wchar_t RTL_DIRECTION_MARKER = L'\u200F';

  /**
   * Create a new ReverseStringFilter that reverses all tokens in the
   * supplied {@link TokenStream}.
   * <p>
   * The reversed tokens will not be marked.
   * </p>
   *
   * @param in {@link TokenStream} to filter
   */
  ReverseStringFilter(std::shared_ptr<TokenStream> in_);

  /**
   * Create a new ReverseStringFilter that reverses and marks all tokens in the
   * supplied {@link TokenStream}.
   * <p>
   * The reversed tokens will be prepended (marked) by the <code>marker</code>
   * character.
   * </p>
   *
   * @param in {@link TokenStream} to filter
   * @param marker A character used to mark reversed tokens
   */
  ReverseStringFilter(std::shared_ptr<TokenStream> in_, wchar_t marker);

  bool incrementToken()  override;

  /**
   * Reverses the given input string
   *
   * @param input the string to reverse
   * @return the given input string in reversed order
   */
  static std::wstring reverse(const std::wstring &input);

  /**
   * Reverses the given input buffer in-place
   * @param buffer the input char array to reverse
   */
  static void reverse(std::deque<wchar_t> &buffer);

  /**
   * Partially reverses the given input buffer in-place from offset 0
   * up to the given length.
   * @param buffer the input char array to reverse
   * @param len the length in the buffer up to where the
   *        buffer should be reversed
   */
  static void reverse(std::deque<wchar_t> &buffer, int const len);

  /**
   * Partially reverses the given input buffer in-place from the given offset
   * up to the given length.
   * @param buffer the input char array to reverse
   * @param start the offset from where to reverse the buffer
   * @param len the length in the buffer up to where the
   *        buffer should be reversed
   */
  static void reverse(std::deque<wchar_t> &buffer, int const start,
                      int const len);

protected:
  std::shared_ptr<ReverseStringFilter> shared_from_this()
  {
    return std::static_pointer_cast<ReverseStringFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::reverse
