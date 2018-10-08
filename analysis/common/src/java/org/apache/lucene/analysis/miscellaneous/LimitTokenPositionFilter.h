#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::analysis::miscellaneous
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * This TokenFilter limits its emitted tokens to those with positions that
 * are not greater than the configured limit.
 * <p>
 * By default, this filter ignores any tokens in the wrapped {@code TokenStream}
 * once the limit has been exceeded, which can result in {@code reset()} being
 * called prior to {@code incrementToken()} returning {@code false}.  For most
 * {@code TokenStream} implementations this should be acceptable, and faster
 * then consuming the full stream. If you are wrapping a {@code TokenStream}
 * which requires that the full stream of tokens be exhausted in order to
 * function properly, use the
 * {@link #LimitTokenPositionFilter(TokenStream,int,bool) consumeAllTokens}
 * option.
 */
class LimitTokenPositionFilter final : public TokenFilter
{
  GET_CLASS_NAME(LimitTokenPositionFilter)

private:
  const int maxTokenPosition;
  const bool consumeAllTokens;
  int tokenPosition = 0;
  bool exhausted = false;
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);

  /**
   * Build a filter that only accepts tokens up to and including the given
   maximum position.
   * This filter will not consume any tokens with position greater than the
   maxTokenPosition limit.

   * @param in the stream to wrap
   * @param maxTokenPosition max position of tokens to produce (1st token always
   has position 1)
   *
   * @see #LimitTokenPositionFilter(TokenStream,int,bool)
   */
public:
  LimitTokenPositionFilter(std::shared_ptr<TokenStream> in_,
                           int maxTokenPosition);

  /**
   * Build a filter that limits the maximum position of tokens to emit.
   *
   * @param in the stream to wrap
   * @param maxTokenPosition max position of tokens to produce (1st token always
   * has position 1)
   * @param consumeAllTokens whether all tokens from the wrapped input stream
   * must be consumed even if maxTokenPosition is exceeded.
   */
  LimitTokenPositionFilter(std::shared_ptr<TokenStream> in_,
                           int maxTokenPosition, bool consumeAllTokens);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<LimitTokenPositionFilter> shared_from_this()
  {
    return std::static_pointer_cast<LimitTokenPositionFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
