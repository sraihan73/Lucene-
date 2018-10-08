#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/SegTokenPair.h"

#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/SegToken.h"
#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/BigramDictionary.h"
#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/SegGraph.h"

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
 * Graph representing possible token pairs (bigrams) at each start offset in the
 * sentence. <p> For each start offset, a deque of possible token pairs is
 * stored.
 * </p>
 * @lucene.experimental
 */
class BiSegGraph : public std::enable_shared_from_this<BiSegGraph>
{
  GET_CLASS_NAME(BiSegGraph)

private:
  std::unordered_map<int, std::deque<std::shared_ptr<SegTokenPair>>>
      tokenPairListTable =
          std::unordered_map<int, std::deque<std::shared_ptr<SegTokenPair>>>();

  std::deque<std::shared_ptr<SegToken>> segTokenList;

  static std::shared_ptr<BigramDictionary> bigramDict;

public:
  BiSegGraph(std::shared_ptr<SegGraph> segGraph);

  /*
   * Generate a BiSegGraph based upon a SegGraph
   */
private:
  void generateBiSegGraph(std::shared_ptr<SegGraph> segGraph);

  /**
   * Returns true if their is a deque of token pairs at this offset (index of the
   * second token)
   *
   * @param to index of the second token in the token pair
   * @return true if a token pair exists
   */
public:
  virtual bool isToExist(int to);

  /**
   * Return a {@link List} of all token pairs at this offset (index of the
   * second token)
   *
   * @param to index of the second token in the token pair
   * @return {@link List} of token pairs.
   */
  virtual std::deque<std::shared_ptr<SegTokenPair>> getToList(int to);

  /**
   * Add a {@link SegTokenPair}
   *
   * @param tokenPair {@link SegTokenPair}
   */
  virtual void addSegTokenPair(std::shared_ptr<SegTokenPair> tokenPair);

  /**
   * Get the number of {@link SegTokenPair} entries in the table.
   * @return number of {@link SegTokenPair} entries
   */
  virtual int getToCount();

  /**
   * Find the shortest path with the Viterbi algorithm.
   * @return {@link List}
   */
  virtual std::deque<std::shared_ptr<SegToken>> getShortPath();

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/
