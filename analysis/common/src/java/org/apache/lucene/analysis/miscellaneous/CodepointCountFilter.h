#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/FilteringTokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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
namespace org::apache::lucene::analysis::miscellaneous
{

using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Removes words that are too long or too short from the stream.
 * <p>
 * Note: Length is calculated as the number of Unicode codepoints.
 * </p>
 */
class CodepointCountFilter final : public FilteringTokenFilter
{
  GET_CLASS_NAME(CodepointCountFilter)

private:
  const int min;
  const int max;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /**
   * Create a new {@link CodepointCountFilter}. This will filter out tokens
   * whose
   * {@link CharTermAttribute} is either too short ({@link
   * Character#codePointCount(char[], int, int)} &lt; min) or too long ({@link
   * Character#codePointCount(char[], int, int)} &gt; max).
   * @param in      the {@link TokenStream} to consume
   * @param min     the minimum length
   * @param max     the maximum length
   */
public:
  CodepointCountFilter(std::shared_ptr<TokenStream> in_, int min, int max);

  bool accept() override;

protected:
  std::shared_ptr<CodepointCountFilter> shared_from_this()
  {
    return std::static_pointer_cast<CodepointCountFilter>(
        org.apache.lucene.analysis.FilteringTokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
