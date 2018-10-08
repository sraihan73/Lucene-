#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::no
{
class NorwegianMinimalStemmer;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class KeywordAttribute;
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
namespace org::apache::lucene::analysis::no
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

/**
 * A {@link TokenFilter} that applies {@link NorwegianMinimalStemmer} to stem
 * Norwegian words. <p> To prevent terms from being stemmed use an instance of
 * {@link SetKeywordMarkerFilter} or a custom {@link TokenFilter} that sets
 * the {@link KeywordAttribute} before this {@link TokenStream}.
 * </p>
 */
class NorwegianMinimalStemFilter final : public TokenFilter
{
  GET_CLASS_NAME(NorwegianMinimalStemFilter)
private:
  const std::shared_ptr<NorwegianMinimalStemmer> stemmer;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttr =
      addAttribute(KeywordAttribute::typeid);

  /**
   * Calls {@link #NorwegianMinimalStemFilter(TokenStream, int)
   * NorwegianMinimalStemFilter(input, BOKMAAL)}
   */
public:
  NorwegianMinimalStemFilter(std::shared_ptr<TokenStream> input);

  /**
   * Creates a new NorwegianLightStemFilter
   * @param flags set to {@link NorwegianLightStemmer#BOKMAAL},
   *                     {@link NorwegianLightStemmer#NYNORSK}, or both.
   */
  NorwegianMinimalStemFilter(std::shared_ptr<TokenStream> input, int flags);

  bool incrementToken()  override;

protected:
  std::shared_ptr<NorwegianMinimalStemFilter> shared_from_this()
  {
    return std::static_pointer_cast<NorwegianMinimalStemFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::no