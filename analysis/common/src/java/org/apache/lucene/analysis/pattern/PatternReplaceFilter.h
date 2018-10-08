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
namespace org::apache::lucene::analysis::pattern
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * A TokenFilter which applies a Pattern to each token in the stream,
 * replacing match occurances with the specified replacement string.
 *
 * <p>
 * <b>Note:</b> Depending on the input and the pattern used and the input
 * TokenStream, this TokenFilter may produce Tokens whose text is the empty
 * string.
 * </p>
 *
 * @see Pattern
 */
class PatternReplaceFilter final : public TokenFilter
{
  GET_CLASS_NAME(PatternReplaceFilter)
private:
  const std::wstring replacement;
  const bool all;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<Matcher> m;

  /**
   * Constructs an instance to replace either the first, or all occurances
   *
   * @param in the TokenStream to process
   * @param p the patterm to apply to each Token
   * @param replacement the "replacement string" to substitute, if null a
   *        blank string will be used. Note that this is not the literal
   *        string that will be used, '$' and '\' have special meaning.
   * @param all if true, all matches will be replaced otherwise just the first
   * match.
   * @see Matcher#quoteReplacement
   */
public:
  PatternReplaceFilter(std::shared_ptr<TokenStream> in_,
                       std::shared_ptr<Pattern> p,
                       const std::wstring &replacement, bool all);

  bool incrementToken()  override;

protected:
  std::shared_ptr<PatternReplaceFilter> shared_from_this()
  {
    return std::static_pointer_cast<PatternReplaceFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::pattern
