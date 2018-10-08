#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class BytesTermAttribute;
}
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

using BytesTermAttribute =
    org::apache::lucene::analysis::tokenattributes::BytesTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * TokenStream from a canned deque of binary (BytesRef-based)
 * tokens.
 */
class CannedBinaryTokenStream final : public TokenStream
{
  GET_CLASS_NAME(CannedBinaryTokenStream)

  /** Represents a binary token. */
public:
  class BinaryToken final : public std::enable_shared_from_this<BinaryToken>
  {
    GET_CLASS_NAME(BinaryToken)
  public:
    std::shared_ptr<BytesRef> term;
    int posInc = 0;
    int posLen = 0;
    int startOffset = 0;
    int endOffset = 0;

    BinaryToken(std::shared_ptr<BytesRef> term);

    BinaryToken(std::shared_ptr<BytesRef> term, int posInc, int posLen);
  };

private:
  std::deque<std::shared_ptr<BinaryToken>> const tokens;
  int upto = 0;
  const std::shared_ptr<BytesTermAttribute> termAtt =
      addAttribute(BytesTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLengthAtt =
      addAttribute(PositionLengthAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

public:
  CannedBinaryTokenStream(std::deque<BinaryToken> &tokens);

  bool incrementToken() override;

protected:
  std::shared_ptr<CannedBinaryTokenStream> shared_from_this()
  {
    return std::static_pointer_cast<CannedBinaryTokenStream>(
        TokenStream::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
