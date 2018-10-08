#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::search::highlight
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;

/**
 * One, or several overlapping tokens, along with the score(s) and the scope of
 * the original text.
 */
class TokenGroup : public std::enable_shared_from_this<TokenGroup>
{
  GET_CLASS_NAME(TokenGroup)

private:
  static constexpr int MAX_NUM_TOKENS_PER_GROUP = 50;

  std::deque<float> scores = std::deque<float>(MAX_NUM_TOKENS_PER_GROUP);
  int numTokens = 0;
  int startOffset = 0;
  int endOffset = 0;
  float tot = 0;
  int matchStartOffset = 0;
  int matchEndOffset = 0;

  std::shared_ptr<OffsetAttribute> offsetAtt;
  std::shared_ptr<CharTermAttribute> termAtt;

public:
  TokenGroup(std::shared_ptr<TokenStream> tokenStream);

  virtual void addToken(float score);

  virtual bool isDistinct();

  virtual void clear();

  /**
   *
   * @param index a value between 0 and numTokens -1
   * @return the "n"th score
   */
  virtual float getScore(int index);

  /**
   * @return the earliest start offset in the original text of a matching token
   * in this group (score &gt; 0), or if there are none then the earliest offset
   * of any token in the group.
   */
  virtual int getStartOffset();

  /**
   * @return the latest end offset in the original text of a matching token in
   * this group (score &gt; 0), or if there are none then {@link
   * #getEndOffset()}.
   */
  virtual int getEndOffset();

  /**
   * @return the number of tokens in this group
   */
  virtual int getNumTokens();

  /**
   * @return all tokens' scores summed up
   */
  virtual float getTotalScore();
};

} // namespace org::apache::lucene::search::highlight
