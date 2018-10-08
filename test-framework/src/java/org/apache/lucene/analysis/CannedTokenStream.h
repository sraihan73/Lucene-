#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Token.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

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

using Token = org::apache::lucene::analysis::Token;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * TokenStream from a canned deque of Tokens.
 */
class CannedTokenStream final : public TokenStream
{
  GET_CLASS_NAME(CannedTokenStream)
private:
  std::deque<std::shared_ptr<Token>> const tokens;
  int upto = 0;
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const int finalOffset;
  const int finalPosInc;

public:
  CannedTokenStream(std::deque<Token> &tokens);

  /** If you want trailing holes, pass a non-zero
   *  finalPosInc. */
  CannedTokenStream(int finalPosInc, int finalOffset,
                    std::deque<Token> &tokens);

  void end()  override;

  bool incrementToken() override;

protected:
  std::shared_ptr<CannedTokenStream> shared_from_this()
  {
    return std::static_pointer_cast<CannedTokenStream>(
        TokenStream::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
