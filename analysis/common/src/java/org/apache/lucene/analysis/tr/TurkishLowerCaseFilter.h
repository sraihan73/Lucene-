#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::tr
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Normalizes Turkish token text to lower case.
 * <p>
 * Turkish and Azeri have unique casing behavior for some characters. This
 * filter applies Turkish lowercase rules. For more information, see <a
 * href="http://en.wikipedia.org/wiki/Turkish_dotted_and_dotless_I"
 * >http://en.wikipedia.org/wiki/Turkish_dotted_and_dotless_I</a>
 * </p>
 */
class TurkishLowerCaseFilter final : public TokenFilter
{
  GET_CLASS_NAME(TurkishLowerCaseFilter)
private:
  static constexpr int LATIN_CAPITAL_LETTER_I = L'\u0049';
  static constexpr int LATIN_SMALL_LETTER_I = L'\u0069';
  static constexpr int LATIN_SMALL_LETTER_DOTLESS_I = L'\u0131';
  static constexpr int COMBINING_DOT_ABOVE = L'\u0307';
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /**
   * Create a new TurkishLowerCaseFilter, that normalizes Turkish token text
   * to lower case.
   *
   * @param in TokenStream to filter
   */
public:
  TurkishLowerCaseFilter(std::shared_ptr<TokenStream> in_);

  bool incrementToken()  override final;

  /**
   * lookahead for a combining dot above.
   * other NSMs may be in between.
   */
private:
  bool isBeforeDot(std::deque<wchar_t> &s, int pos, int len);

  /**
   * delete a character in-place.
   * rarely happens, only if COMBINING_DOT_ABOVE is found after an i
   */
  int delete_(std::deque<wchar_t> &s, int pos, int len);

protected:
  std::shared_ptr<TurkishLowerCaseFilter> shared_from_this()
  {
    return std::static_pointer_cast<TurkishLowerCaseFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/tr/
