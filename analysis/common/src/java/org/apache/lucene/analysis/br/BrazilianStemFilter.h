#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::br
{
class BrazilianStemmer;
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
namespace org::apache::lucene::analysis::br
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

/**
 * A {@link TokenFilter} that applies {@link BrazilianStemmer}.
 * <p>
 * To prevent terms from being stemmed use an instance of
 * {@link SetKeywordMarkerFilter} or a custom {@link TokenFilter} that sets
 * the {@link KeywordAttribute} before this {@link TokenStream}.
 * </p>
 * @see SetKeywordMarkerFilter
 *
 */
class BrazilianStemFilter final : public TokenFilter
{
  GET_CLASS_NAME(BrazilianStemFilter)

  /**
   * {@link BrazilianStemmer} in use by this filter.
   */
private:
  std::shared_ptr<BrazilianStemmer> stemmer =
      std::make_shared<BrazilianStemmer>();
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private std::unordered_set<?> exclusions = null;
  std::shared_ptr < Set < ? >> exclusions = nullptr;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttr =
      addAttribute(KeywordAttribute::typeid);

  /**
   * Creates a new BrazilianStemFilter
   *
   * @param in the source {@link TokenStream}
   */
public:
  BrazilianStemFilter(std::shared_ptr<TokenStream> in_);

  bool incrementToken()  override;

protected:
  std::shared_ptr<BrazilianStemFilter> shared_from_this()
  {
    return std::static_pointer_cast<BrazilianStemFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::br
