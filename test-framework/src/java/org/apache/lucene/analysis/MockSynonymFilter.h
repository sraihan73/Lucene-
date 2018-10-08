#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
}
namespace org::apache::lucene::util
{
class AttributeSource;
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
using AttributeSource = org::apache::lucene::util::AttributeSource;

/** adds synonym of "dog" for "dogs", and synonym of "cavy" for "guinea pig". */
class MockSynonymFilter : public TokenFilter
{
  GET_CLASS_NAME(MockSynonymFilter)
public:
  std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  std::shared_ptr<PositionLengthAttribute> posLenAtt =
      addAttribute(PositionLengthAttribute::typeid);
  std::deque<std::shared_ptr<AttributeSource>> tokenQueue =
      std::deque<std::shared_ptr<AttributeSource>>();
  bool endOfInput = false;

  MockSynonymFilter(std::shared_ptr<TokenStream> input);

  void reset()  override;

  bool incrementToken()  override final;

private:
  void addSynonym(const std::wstring &synonymText, int posLen, int endOffset);
  void addSynonymAndRestoreOrigToken(const std::wstring &synonymText,
                                     int posLen, int endOffset);

protected:
  std::shared_ptr<MockSynonymFilter> shared_from_this()
  {
    return std::static_pointer_cast<MockSynonymFilter>(
        TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
