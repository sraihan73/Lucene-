#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::analysis::miscellaneous
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * This TokenFilter limits the number of tokens while indexing. It is
 * a replacement for the maximum field length setting inside {@link
 * org.apache.lucene.index.IndexWriter}. <p> By default, this filter ignores any
 * tokens in the wrapped {@code TokenStream} once the limit has been reached,
 * which can result in {@code reset()} being called prior to {@code
 * incrementToken()} returning {@code false}.  For most
 * {@code TokenStream} implementations this should be acceptable, and faster
 * then consuming the full stream. If you are wrapping a {@code TokenStream}
 * which requires that the full stream of tokens be exhausted in order to
 * function properly, use the
 * {@link #LimitTokenCountFilter(TokenStream,int,bool) consumeAllTokens}
 * option.
 */
class LimitTokenCountFilter final : public TokenFilter
{
  GET_CLASS_NAME(LimitTokenCountFilter)

private:
  const int maxTokenCount;
  const bool consumeAllTokens;
  int tokenCount = 0;
  bool exhausted = false;

  /**
   * Build a filter that only accepts tokens up to a maximum number.
   * This filter will not consume any tokens beyond the maxTokenCount limit
   *
   * @see #LimitTokenCountFilter(TokenStream,int,bool)
   */
public:
  LimitTokenCountFilter(std::shared_ptr<TokenStream> in_, int maxTokenCount);

  /**
   * Build an filter that limits the maximum number of tokens per field.
   * @param in the stream to wrap
   * @param maxTokenCount max number of tokens to produce
   * @param consumeAllTokens whether all tokens from the input must be consumed
   * even if maxTokenCount is reached.
   */
  LimitTokenCountFilter(std::shared_ptr<TokenStream> in_, int maxTokenCount,
                        bool consumeAllTokens);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<LimitTokenCountFilter> shared_from_this()
  {
    return std::static_pointer_cast<LimitTokenCountFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
