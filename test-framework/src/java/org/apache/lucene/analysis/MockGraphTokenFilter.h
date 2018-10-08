#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Position;
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

// TODO: sometimes remove tokens too...?

/** Randomly inserts overlapped (posInc=0) tokens with
 *  posLength sometimes &gt; 1.  The chain must have
 *  an OffsetAttribute.  */

class MockGraphTokenFilter final
    : public LookaheadTokenFilter<
          std::shared_ptr<LookaheadTokenFilter::Position>>
{
  GET_CLASS_NAME(MockGraphTokenFilter)

private:
  static bool DEBUG;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  const int64_t seed;
  std::shared_ptr<Random> random;

public:
  MockGraphTokenFilter(std::shared_ptr<Random> random,
                       std::shared_ptr<TokenStream> input);

protected:
  std::shared_ptr<Position> newPosition() override;

  void afterPosition()  override;

public:
  void reset()  override;

  virtual ~MockGraphTokenFilter();

  bool incrementToken()  override;

protected:
  std::shared_ptr<MockGraphTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<MockGraphTokenFilter>(
        LookaheadTokenFilter<
            LookaheadTokenFilter.Position>::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
