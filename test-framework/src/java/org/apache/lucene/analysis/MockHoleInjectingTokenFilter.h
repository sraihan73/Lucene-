#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
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

using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;

// TODO: maybe, instead to be more "natural", we should make
// a MockRemovesTokensTF, ideally subclassing FilteringTF
// (in modules/analysis)

/**
 * Randomly injects holes (similar to what a stopfilter would do)
 */
class MockHoleInjectingTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(MockHoleInjectingTokenFilter)

private:
  const int64_t randomSeed;
  std::shared_ptr<Random> random;
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAtt =
      addAttribute(PositionLengthAttribute::typeid);
  int maxPos = 0;
  int pos = 0;

public:
  MockHoleInjectingTokenFilter(std::shared_ptr<Random> random,
                               std::shared_ptr<TokenStream> in_);

  void reset()  override;

  bool incrementToken()  override;

  // TODO: end?

protected:
  std::shared_ptr<MockHoleInjectingTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<MockHoleInjectingTokenFilter>(
        TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
