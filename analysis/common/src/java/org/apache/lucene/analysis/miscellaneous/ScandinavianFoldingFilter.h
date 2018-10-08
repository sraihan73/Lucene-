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
 * This filter folds Scandinavian characters åÅäæÄÆ-&gt;a and öÖøØ-&gt;o.
 * It also discriminate against use of double vowels aa, ae, ao, oe and oo,
 * leaving just the first one. <p> It's a semantically more destructive solution
 * than {@link ScandinavianNormalizationFilter} but can in addition help with
 * matching raksmorgas as räksmörgås. <p> blåbærsyltetøj == blåbärsyltetöj ==
 * blaabaarsyltetoej == blabarsyltetoj räksmörgås == ræksmørgås == ræksmörgaos
 * == raeksmoergaas == raksmorgas <p> Background: Swedish åäö are in fact the
 * same letters as Norwegian and Danish åæø and thus interchangeable when used
 * between these languages. They are however folded differently when people type
 * them on a keyboard lacking these characters.
 * <p>
 * In that situation almost all Swedish people use a, a, o instead of å, ä, ö.
 * <p>
 * Norwegians and Danes on the other hand usually type aa, ae and oe instead of
 * å, æ and ø. Some do however use a, a, o, oo, ao and sometimes permutations of
 * everything above. <p> This filter solves that mismatch problem, but might
 * also cause new.
 * @see ScandinavianNormalizationFilter
 */
class ScandinavianFoldingFilter final : public TokenFilter
{
  GET_CLASS_NAME(ScandinavianFoldingFilter)

public:
  ScandinavianFoldingFilter(std::shared_ptr<TokenStream> input);

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
  std::shared_ptr<ScandinavianFoldingFilter> shared_from_this()
  {
    return std::static_pointer_cast<ScandinavianFoldingFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
