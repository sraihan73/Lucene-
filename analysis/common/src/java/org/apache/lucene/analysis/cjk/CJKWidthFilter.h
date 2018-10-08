#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
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
namespace org::apache::lucene::analysis::cjk
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * A {@link TokenFilter} that normalizes CJK width differences:
 * <ul>
 *   <li>Folds fullwidth ASCII variants into the equivalent basic latin
 *   <li>Folds halfwidth Katakana variants into the equivalent kana
 * </ul>
 * <p>
 * NOTE: this filter can be viewed as a (practical) subset of NFKC/NFKD
 * Unicode normalization. See the normalization support in the ICU package
 * for full normalization.
 */
class CJKWidthFilter final : public TokenFilter
{
  GET_CLASS_NAME(CJKWidthFilter)
private:
  std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /* halfwidth kana mappings: 0xFF65-0xFF9D
   *
   * note: 0xFF9C and 0xFF9D are only mapped to 0x3099 and 0x309A
   * as a fallback when they cannot properly combine with a preceding
   * character into a composed form.
   */
  static std::deque<wchar_t> const KANA_NORM;

public:
  CJKWidthFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override;

  /* kana combining diffs: 0x30A6-0x30FD */
private:
  static std::deque<char> const KANA_COMBINE_VOICED;

  static std::deque<char> const KANA_COMBINE_HALF_VOICED;

  /** returns true if we successfully combined the voice mark */
  static bool combine(std::deque<wchar_t> &text, int pos, wchar_t ch);

protected:
  std::shared_ptr<CJKWidthFilter> shared_from_this()
  {
    return std::static_pointer_cast<CJKWidthFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/cjk/
