#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
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
namespace org::apache::lucene::analysis::payloads
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;

/**
 * Adds the {@link OffsetAttribute#startOffset()}
 * and {@link OffsetAttribute#endOffset()}
 * First 4 bytes are the start
 *
 **/
class TokenOffsetPayloadTokenFilter : public TokenFilter
{
  GET_CLASS_NAME(TokenOffsetPayloadTokenFilter)
private:
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PayloadAttribute> payAtt =
      addAttribute(PayloadAttribute::typeid);

public:
  TokenOffsetPayloadTokenFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override final;

protected:
  std::shared_ptr<TokenOffsetPayloadTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TokenOffsetPayloadTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};
} // namespace org::apache::lucene::analysis::payloads
