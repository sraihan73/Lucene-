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
namespace org::apache::lucene::analysis::de
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Normalizes German characters according to the heuristics
 * of the <a
 * href="http://snowball.tartarus.org/algorithms/german2/stemmer.html"> German2
 * snowball algorithm</a>. It allows for the fact that ä, ö and ü are sometimes
 * written as ae, oe and ue. <ul> <li> 'ß' is replaced by 'ss' <li> 'ä', 'ö',
 * 'ü' are replaced by 'a', 'o', 'u', respectively. <li> 'ae' and 'oe' are
 * replaced by 'a', and 'o', respectively. <li> 'ue' is replaced by 'u', when
 * not following a vowel or q.
 * </ul>
 * This is useful if you want this normalization without using
 * the German2 stemmer, or perhaps no stemming at all.
 */
class GermanNormalizationFilter final : public TokenFilter
{
  GET_CLASS_NAME(GermanNormalizationFilter)
  // FSM with 3 states:
private:
  static constexpr int N = 0; // ordinary state
  static constexpr int V = 1; // stops 'u' from entering umlaut state
  static constexpr int U = 2; // umlaut state, allows e-deletion

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

public:
  GermanNormalizationFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override;

protected:
  std::shared_ptr<GermanNormalizationFilter> shared_from_this()
  {
    return std::static_pointer_cast<GermanNormalizationFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/de/
