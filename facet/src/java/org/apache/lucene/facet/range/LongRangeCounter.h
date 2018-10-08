#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet::range
{
class LongRangeNode;
}

namespace org::apache::lucene::facet::range
{
class LongRange;
}
namespace org::apache::lucene::facet::range
{
class InclusiveRange;
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
namespace org::apache::lucene::facet::range
{

/** Counts how many times each range was seen;
 *  per-hit it's just a binary search ({@link #add})
 *  against the elementary intervals, and in the end we
 *  rollup back to the original ranges. */

class LongRangeCounter final
    : public std::enable_shared_from_this<LongRangeCounter>
{
  GET_CLASS_NAME(LongRangeCounter)

public:
  const std::shared_ptr<LongRangeNode> root;
  std::deque<int64_t> const boundaries;
  std::deque<int> const leafCounts;

  // Used during rollup
private:
  int leafUpto = 0;
  int missingCount = 0;

public:
  LongRangeCounter(std::deque<std::shared_ptr<LongRange>> &ranges);

  void add(int64_t v);

  /** Fills counts corresponding to the original input
   *  ranges, returning the missing count (how many hits
   *  didn't match any ranges). */
  int fillCounts(std::deque<int> &counts);

private:
  int rollup(std::shared_ptr<LongRangeNode> node, std::deque<int> &counts,
             bool sawOutputs);

  static std::shared_ptr<LongRangeNode>
  split(int start, int end,
        std::deque<std::shared_ptr<InclusiveRange>> &elementaryIntervals);

private:
  class InclusiveRange final
      : public std::enable_shared_from_this<InclusiveRange>
  {
    GET_CLASS_NAME(InclusiveRange)
  public:
    const int64_t start;
    const int64_t end;

    InclusiveRange(int64_t start, int64_t end);

    virtual std::wstring toString();
  };

  /** Holds one node of the segment tree. */
public:
  class LongRangeNode final : public std::enable_shared_from_this<LongRangeNode>
  {
    GET_CLASS_NAME(LongRangeNode)
  public:
    const std::shared_ptr<LongRangeNode> left;
    const std::shared_ptr<LongRangeNode> right;

    // Our range, inclusive:
    const int64_t start;
    const int64_t end;

    // If we are a leaf, the index into elementary ranges that
    // we point to:
    const int leafIndex;

    // Which range indices to output when a query goes
    // through this node:
    std::deque<int> outputs;

    LongRangeNode(int64_t start, int64_t end,
                  std::shared_ptr<LongRangeNode> left,
                  std::shared_ptr<LongRangeNode> right, int leafIndex);

    virtual std::wstring toString();

    static void indent(std::shared_ptr<StringBuilder> sb, int depth);

    /** Recursively assigns range outputs to each node. */
    void addOutputs(int index, std::shared_ptr<LongRange> range);

    void toString(std::shared_ptr<StringBuilder> sb, int depth);
  };
};

} // namespace org::apache::lucene::facet::range
