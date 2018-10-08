#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
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
namespace org::apache::lucene::analysis
{

using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * TokenFilter that adds random variable-length payloads.
 */
class MockVariableLengthPayloadFilter final : public TokenFilter
{
  GET_CLASS_NAME(MockVariableLengthPayloadFilter)
private:
  static constexpr int MAXLENGTH = 129;

  const std::shared_ptr<PayloadAttribute> payloadAtt =
      addAttribute(PayloadAttribute::typeid);
  const std::shared_ptr<Random> random;
  std::deque<char> const bytes = std::deque<char>(MAXLENGTH);
  const std::shared_ptr<BytesRef> payload;

public:
  MockVariableLengthPayloadFilter(std::shared_ptr<Random> random,
                                  std::shared_ptr<TokenStream> in_);

  bool incrementToken()  override;

protected:
  std::shared_ptr<MockVariableLengthPayloadFilter> shared_from_this()
  {
    return std::static_pointer_cast<MockVariableLengthPayloadFilter>(
        TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
