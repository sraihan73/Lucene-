#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::bn
{
class BengaliNormalizer;
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
namespace org::apache::lucene::analysis::bn
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

/**
 * A {@link TokenFilter} that applies {@link BengaliNormalizer} to normalize the
 * orthography.
 * <p>
 * In some cases the normalization may cause unrelated terms to conflate, so
 * to prevent terms from being normalized use an instance of
 * {@link SetKeywordMarkerFilter} or a custom {@link TokenFilter} that sets
 * the {@link KeywordAttribute} before this {@link TokenStream}.
 * </p>
 * @see BengaliNormalizer
 */
class BengaliNormalizationFilter final : public TokenFilter
{
  GET_CLASS_NAME(BengaliNormalizationFilter)

private:
  const std::shared_ptr<BengaliNormalizer> normalizer =
      std::make_shared<BengaliNormalizer>();
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAtt =
      addAttribute(KeywordAttribute::typeid);

public:
  BengaliNormalizationFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override;

protected:
  std::shared_ptr<BengaliNormalizationFilter> shared_from_this()
  {
    return std::static_pointer_cast<BengaliNormalizationFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::bn
