#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
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
namespace org::apache::lucene::analysis::phonetic
{

using org::apache::commons::codec::language::bm::PhoneticEngine;
using org::apache::commons::codec::language::bm::Languages::LanguageSet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * TokenFilter for Beider-Morse phonetic encoding.
 * @see BeiderMorseEncoder
 * @lucene.experimental
 */
class BeiderMorseFilter final : public TokenFilter
{
  GET_CLASS_NAME(BeiderMorseFilter)
private:
  const std::shared_ptr<PhoneticEngine> engine;
  const std::shared_ptr<LanguageSet> languages;

  // output is a string such as ab|ac|...
  // in complex cases like d'angelo it's (anZelo|andZelo|...)-(danZelo|...)
  // if there are multiple 's, it starts to nest...
  static const std::shared_ptr<Pattern> pattern;

  // matcher over any buffered output
  const std::shared_ptr<Matcher> matcher = pattern->matcher(L"");
  // encoded representation
  std::wstring encoded;
  // preserves all attributes for any buffered outputs
  std::shared_ptr<State> state;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);

  /**
   * Calls
   * {@link #BeiderMorseFilter(TokenStream, PhoneticEngine,
   * org.apache.commons.codec.language.bm.Languages.LanguageSet)}
   *
   * @param input TokenStream to filter
   * @param engine configured PhoneticEngine with BM settings.
   */
public:
  BeiderMorseFilter(std::shared_ptr<TokenStream> input,
                    std::shared_ptr<PhoneticEngine> engine);

  /**
   * Create a new BeiderMorseFilter
   * @param input TokenStream to filter
   * @param engine configured PhoneticEngine with BM settings.
   * @param languages optional Set of original languages. Can be null (which
   * means it will be guessed).
   */
  BeiderMorseFilter(std::shared_ptr<TokenStream> input,
                    std::shared_ptr<PhoneticEngine> engine,
                    std::shared_ptr<LanguageSet> languages);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<BeiderMorseFilter> shared_from_this()
  {
    return std::static_pointer_cast<BeiderMorseFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::phonetic
