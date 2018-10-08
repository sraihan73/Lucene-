#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

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
namespace org::apache::lucene::analysis::miscellaneous
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * This filter normalize use of the interchangeable Scandinavian characters
 * æÆäÄöÖøØ and folded variants (aa, ao, ae, oe and oo) by transforming them to
 * åÅæÆøØ. <p> It's a semantically less destructive solution than {@link
 * ScandinavianFoldingFilter}, most useful when a person with a Norwegian or
 * Danish keyboard queries a Swedish index and vice versa. This filter does
 * <b>not</b>  the common Swedish folds of å and ä to a nor ö to o. <p>
 * blåbærsyltetøj == blåbärsyltetöj == blaabaarsyltetoej but not blabarsyltetoj
 * räksmörgås == ræksmørgås == ræksmörgaos == raeksmoergaas but not raksmorgas
 * @see ScandinavianFoldingFilter
 */
class ScandinavianNormalizationFilter final : public TokenFilter
{
  GET_CLASS_NAME(ScandinavianNormalizationFilter)

public:
  ScandinavianNormalizationFilter(std::shared_ptr<TokenStream> input);

private:
  const std::shared_ptr<CharTermAttribute> charTermAttribute =
      addAttribute(CharTermAttribute::typeid);

  static constexpr wchar_t AA = L'\u00C5';    // Å
  static constexpr wchar_t aa = L'\u00E5';    // å
  static constexpr wchar_t AE = L'\u00C6';    // Æ
  static constexpr wchar_t ae = L'\u00E6';    // æ
  static constexpr wchar_t AE_se = L'\u00C4'; // Ä
  static constexpr wchar_t ae_se = L'\u00E4'; // ä
  static constexpr wchar_t OE = L'\u00D8';    // Ø
  static constexpr wchar_t oe = L'\u00F8';    // ø
  static constexpr wchar_t OE_se = L'\u00D6'; // Ö
  static constexpr wchar_t oe_se = L'\u00F6'; //ö

public:
  bool incrementToken()  override;

protected:
  std::shared_ptr<ScandinavianNormalizationFilter> shared_from_this()
  {
    return std::static_pointer_cast<ScandinavianNormalizationFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
