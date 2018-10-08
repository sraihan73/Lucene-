#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/stempel/StempelStemmer.h"
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
namespace org::apache::lucene::analysis::stempel
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

/**
 * Transforms the token stream as per the stemming algorithm.
 * <p>
 * Note: the input to the stemming filter must already be in lower case, so you
 * will need to use LowerCaseFilter or LowerCaseTokenizer farther down the
 * Tokenizer chain in order for this to work properly!
 */
class StempelFilter final : public TokenFilter
{
  GET_CLASS_NAME(StempelFilter)
private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAtt =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<StempelStemmer> stemmer;
  const int minLength;

  /**
   * Minimum length of input words to be processed. Shorter words are returned
   * unchanged.
   */
public:
  static constexpr int DEFAULT_MIN_LENGTH = 3;

  /**
   * Create filter using the supplied stemming table.
   *
   * @param in input token stream
   * @param stemmer stemmer
   */
  StempelFilter(std::shared_ptr<TokenStream> in_,
                std::shared_ptr<StempelStemmer> stemmer);

  /**
   * Create filter using the supplied stemming table.
   *
   * @param in input token stream
   * @param stemmer stemmer
   * @param minLength For performance reasons words shorter than minLength
   * characters are not processed, but simply returned.
   */
  StempelFilter(std::shared_ptr<TokenStream> in_,
                std::shared_ptr<StempelStemmer> stemmer, int minLength);

  /** Returns the next input Token, after being stemmed */
  bool incrementToken()  override;

protected:
  std::shared_ptr<StempelFilter> shared_from_this()
  {
    return std::static_pointer_cast<StempelFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/stempel/
