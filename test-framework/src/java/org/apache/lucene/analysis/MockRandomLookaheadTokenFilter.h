#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Position.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
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

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Uses {@link LookaheadTokenFilter} to randomly peek at future tokens.
 */

class MockRandomLookaheadTokenFilter final
    : public LookaheadTokenFilter<
          std::shared_ptr<LookaheadTokenFilter::Position>>
{
  GET_CLASS_NAME(MockRandomLookaheadTokenFilter)
private:
  static constexpr bool DEBUG = false;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<Random> random;
  const int64_t seed;

public:
  MockRandomLookaheadTokenFilter(std::shared_ptr<Random> random,
                                 std::shared_ptr<TokenStream> in_);

  std::shared_ptr<Position> newPosition() override;

protected:
  void afterPosition()  override;

public:
  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<MockRandomLookaheadTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<MockRandomLookaheadTokenFilter>(
        LookaheadTokenFilter<
            LookaheadTokenFilter.Position>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
