#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
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
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;

/**
 * Lets all tokens pass through until it sees one with a start offset &lt;= a
 * configured limit, which won't pass and ends the stream.  This can be useful
 * to limit highlighting, for example. <p> By default, this filter ignores any
 * tokens in the wrapped {@code TokenStream} once the limit has been exceeded,
 * which can result in {@code reset()} being called prior to {@code
 * incrementToken()} returning {@code false}.  For most
 * {@code TokenStream} implementations this should be acceptable, and faster
 * then consuming the full stream. If you are wrapping a {@code TokenStream}
 * which requires that the full stream of tokens be exhausted in order to
 * function properly, use the
 * {@link #LimitTokenOffsetFilter(TokenStream, int, bool)} option.
 */
class LimitTokenOffsetFilter final : public TokenFilter
{
  GET_CLASS_NAME(LimitTokenOffsetFilter)

private:
  const std::shared_ptr<OffsetAttribute> offsetAttrib =
      addAttribute(OffsetAttribute::typeid);
  int maxStartOffset = 0;
  const bool consumeAllTokens;

  // some day we may limit by end offset too but no need right now

  /**
   * Lets all tokens pass through until it sees one with a start offset &lt;=
   * {@code maxStartOffset} which won't pass and ends the stream. It won't
   * consume any tokens afterwards.
   *
   * @param maxStartOffset the maximum start offset allowed
   */
public:
  LimitTokenOffsetFilter(std::shared_ptr<TokenStream> input,
                         int maxStartOffset);

  LimitTokenOffsetFilter(std::shared_ptr<TokenStream> input, int maxStartOffset,
                         bool consumeAllTokens);

  bool incrementToken()  override;

protected:
  std::shared_ptr<LimitTokenOffsetFilter> shared_from_this()
  {
    return std::static_pointer_cast<LimitTokenOffsetFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
