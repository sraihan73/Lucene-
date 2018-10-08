#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
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
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;

// TODO: rename to OffsetsXXXTF?  ie we only validate
// offsets (now anyway...)

// TODO: also make a DebuggingTokenFilter, that just prints
// all att values that come through it...

// TODO: BTSTC should just append this to the chain
// instead of checking itself:

/** A TokenFilter that checks consistency of the tokens (eg
 *  offsets are consistent with one another). */
class ValidatingTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(ValidatingTokenFilter)

private:
  int pos = 0;
  int lastStartOffset = 0;

  // Maps position to the start/end offset:
  const std::unordered_map<int, int> posToStartOffset =
      std::unordered_map<int, int>();
  const std::unordered_map<int, int> posToEndOffset =
      std::unordered_map<int, int>();

  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      getAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAtt =
      getAttribute(PositionLengthAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      getAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<CharTermAttribute> termAtt =
      getAttribute(CharTermAttribute::typeid);

  const std::wstring name;

  /** The name arg is used to identify this stage when
   *  throwing exceptions (useful if you have more than one
   *  instance in your chain). */
public:
  ValidatingTokenFilter(std::shared_ptr<TokenStream> in_,
                        const std::wstring &name);

  bool incrementToken()  override;

  void end()  override;

  void reset()  override;

protected:
  std::shared_ptr<ValidatingTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<ValidatingTokenFilter>(
        TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
