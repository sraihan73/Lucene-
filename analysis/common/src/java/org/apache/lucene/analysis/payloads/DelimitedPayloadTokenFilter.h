#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/payloads/PayloadEncoder.h"
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
namespace org::apache::lucene::analysis::payloads
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Characters before the delimiter are the "token", those after are the payload.
 * <p>
 * For example, if the delimiter is '|', then for the string "foo|bar", foo is
 * the token and "bar" is a payload. <p> Note, you can also include a {@link
 * org.apache.lucene.analysis.payloads.PayloadEncoder} to convert the payload in
 * an appropriate way (from characters to bytes). <p> Note make sure your
 * Tokenizer doesn't split on the delimiter, or this won't work
 *
 * @see PayloadEncoder
 */
class DelimitedPayloadTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(DelimitedPayloadTokenFilter)
public:
  static constexpr wchar_t DEFAULT_DELIMITER = L'|';

private:
  const wchar_t delimiter;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PayloadAttribute> payAtt =
      addAttribute(PayloadAttribute::typeid);
  const std::shared_ptr<PayloadEncoder> encoder;

public:
  DelimitedPayloadTokenFilter(std::shared_ptr<TokenStream> input,
                              wchar_t delimiter,
                              std::shared_ptr<PayloadEncoder> encoder);

  bool incrementToken()  override;

protected:
  std::shared_ptr<DelimitedPayloadTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<DelimitedPayloadTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/payloads/
