#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class KeywordAttribute;
}
namespace org::apache::lucene::analysis::id
{
class IndonesianStemmer;
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
namespace org::apache::lucene::analysis::id
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

/**
 * A {@link TokenFilter} that applies {@link IndonesianStemmer} to stem
 * Indonesian words.
 */
class IndonesianStemFilter final : public TokenFilter
{
  GET_CLASS_NAME(IndonesianStemFilter)
private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAtt =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<IndonesianStemmer> stemmer =
      std::make_shared<IndonesianStemmer>();
  const bool stemDerivational;

  /**
   * Calls {@link #IndonesianStemFilter(TokenStream, bool)
   * IndonesianStemFilter(input, true)}
   */
public:
  IndonesianStemFilter(std::shared_ptr<TokenStream> input);

  /**
   * Create a new IndonesianStemFilter.
   * <p>
   * If <code>stemDerivational</code> is false,
   * only inflectional suffixes (particles and possessive pronouns) are stemmed.
   */
  IndonesianStemFilter(std::shared_ptr<TokenStream> input,
                       bool stemDerivational);

  bool incrementToken()  override;

protected:
  std::shared_ptr<IndonesianStemFilter> shared_from_this()
  {
    return std::static_pointer_cast<IndonesianStemFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::id
