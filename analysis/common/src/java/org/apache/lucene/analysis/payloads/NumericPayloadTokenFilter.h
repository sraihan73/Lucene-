#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
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
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Assigns a payload to a token based on the {@link
 *org.apache.lucene.analysis.tokenattributes.TypeAttribute}
 **/
class NumericPayloadTokenFilter : public TokenFilter
{
  GET_CLASS_NAME(NumericPayloadTokenFilter)

private:
  std::wstring typeMatch;
  std::shared_ptr<BytesRef> thePayload;

  const std::shared_ptr<PayloadAttribute> payloadAtt =
      addAttribute(PayloadAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);

public:
  NumericPayloadTokenFilter(std::shared_ptr<TokenStream> input, float payload,
                            const std::wstring &typeMatch);

  bool incrementToken()  override final;

protected:
  std::shared_ptr<NumericPayloadTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<NumericPayloadTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::payloads
