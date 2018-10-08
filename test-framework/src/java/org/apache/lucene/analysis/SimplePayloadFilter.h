#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
}

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
namespace org::apache::lucene::analysis
{

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;

/** Simple payload filter that sets the payload as pos: XXXX */
class SimplePayloadFilter final : public TokenFilter
{
  GET_CLASS_NAME(SimplePayloadFilter)
public:
  int pos = 0;
  const std::shared_ptr<PayloadAttribute> payloadAttr;
  const std::shared_ptr<CharTermAttribute> termAttr;

  SimplePayloadFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<SimplePayloadFilter> shared_from_this()
  {
    return std::static_pointer_cast<SimplePayloadFilter>(
        TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
