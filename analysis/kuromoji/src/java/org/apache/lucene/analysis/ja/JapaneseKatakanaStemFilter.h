#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
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
namespace org::apache::lucene::analysis::ja
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

/**
 * A {@link TokenFilter} that normalizes common katakana spelling variations
 * ending in a long sound character by removing this character (U+30FC).  Only
 * katakana words longer than a minimum length are stemmed (default is four).
 * <p>
 * Note that only full-width katakana characters are supported.  Please use a
 * {@link org.apache.lucene.analysis.cjk.CJKWidthFilter} to convert half-width
 * katakana to full-width before using this filter.
 * </p>
 * <p>
 * In order to prevent terms from being stemmed, use an instance of
 * {@link org.apache.lucene.analysis.miscellaneous.SetKeywordMarkerFilter}
 * or a custom {@link TokenFilter} that sets the {@link KeywordAttribute}
 * before this {@link TokenStream}.
 * </p>
 */

class JapaneseKatakanaStemFilter final : public TokenFilter
{
  GET_CLASS_NAME(JapaneseKatakanaStemFilter)
public:
  static constexpr int DEFAULT_MINIMUM_LENGTH = 4;

private:
  static constexpr wchar_t HIRAGANA_KATAKANA_PROLONGED_SOUND_MARK = L'\u30fc';

  const std::shared_ptr<CharTermAttribute> termAttr =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttr =
      addAttribute(KeywordAttribute::typeid);
  const int minimumKatakanaLength;

public:
  JapaneseKatakanaStemFilter(std::shared_ptr<TokenStream> input,
                             int minimumLength);

  JapaneseKatakanaStemFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override;

private:
  int stem(std::deque<wchar_t> &term, int length);

  bool isKatakana(std::deque<wchar_t> &term, int length);

protected:
  std::shared_ptr<JapaneseKatakanaStemFilter> shared_from_this()
  {
    return std::static_pointer_cast<JapaneseKatakanaStemFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ja/
