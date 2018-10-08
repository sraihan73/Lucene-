#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::cn::smart::hhmm
{
class SegToken;
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
namespace org::apache::lucene::analysis::cn::smart::hhmm
{

/**
 * Graph representing possible tokens at each start offset in the sentence.
 * <p>
 * For each start offset, a deque of possible tokens is stored.
 * </p>
 * @lucene.experimental
 */
class SegGraph : public std::enable_shared_from_this<SegGraph>
{
  GET_CLASS_NAME(SegGraph)

  /**
   * Map of start offsets to ArrayList of tokens at that position
   */
private:
  std::unordered_map<int, std::deque<std::shared_ptr<SegToken>>>
      tokenListTable =
          std::unordered_map<int, std::deque<std::shared_ptr<SegToken>>>();

  int maxStart = -1;

  /**
   * Returns true if a mapping for the specified start offset exists
   *
   * @param s startOffset
   * @return true if there are tokens for the startOffset
   */
public:
  virtual bool isStartExist(int s);

  /**
   * Get the deque of tokens at the specified start offset
   *
   * @param s startOffset
   * @return List of tokens at the specified start offset.
   */
  virtual std::deque<std::shared_ptr<SegToken>> getStartList(int s);

  /**
   * Get the highest start offset in the map_obj
   *
   * @return maximum start offset, or -1 if the map_obj is empty.
   */
  virtual int getMaxStart();

  /**
   * Set the {@link SegToken#index} for each token, based upon its order by
   * startOffset.
   * @return a {@link List} of these ordered tokens.
   */
  virtual std::deque<std::shared_ptr<SegToken>> makeIndex();

  /**
   * Add a {@link SegToken} to the mapping, creating a new mapping at the
   * token's startOffset if one does not exist.
   * @param token {@link SegToken}
   */
  virtual void addToken(std::shared_ptr<SegToken> token);

  /**
   * Return a {@link List} of all tokens in the map_obj, ordered by startOffset.
   *
   * @return {@link List} of all tokens in the map_obj.
   */
  virtual std::deque<std::shared_ptr<SegToken>> toTokenList();

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::analysis::cn::smart::hhmm
