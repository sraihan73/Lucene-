#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <typeinfo>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::util
{
class FrequencyTrackingRingBuffer;
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
namespace org::apache::lucene::search
{

using FrequencyTrackingRingBuffer =
    org::apache::lucene::util::FrequencyTrackingRingBuffer;

/**
 * A {@link QueryCachingPolicy} that tracks usage statistics of recently-used
 * filters in order to decide on which filters are worth caching.
 *
 * @lucene.experimental
 */
class UsageTrackingQueryCachingPolicy
    : public std::enable_shared_from_this<UsageTrackingQueryCachingPolicy>,
      public QueryCachingPolicy
{
  GET_CLASS_NAME(UsageTrackingQueryCachingPolicy)

  // the hash code that we use as a sentinel in the ring buffer.
private:
  static const int SENTINEL = std::numeric_limits<int>::min();

  static bool isPointQuery(std::shared_ptr<Query> query);

public:
  static bool isCostly(std::shared_ptr<Query> query);

private:
  static bool shouldNeverCache(std::shared_ptr<Query> query);

  const std::shared_ptr<FrequencyTrackingRingBuffer> recentlyUsedFilters;

  /**
   * Expert: Create a new instance with a configurable history size. Beware of
   * passing too large values for the size of the history, either
   * {@link #minFrequencyToCache} returns low values and this means some filters
   * that are rarely used will be cached, which would hurt performance. Or
   * {@link #minFrequencyToCache} returns high values that are function of the
   * size of the history but then filters will be slow to make it to the cache.
   *
   * @param historySize               the number of recently used filters to
   * track
   */
public:
  UsageTrackingQueryCachingPolicy(int historySize);

  /** Create a new instance with an history size of 256. This should be a good
   *  default for most cases. */
  UsageTrackingQueryCachingPolicy();

  /**
   * For a given filter, return how many times it should appear in the history
   * before being cached. The default implementation returns 2 for filters that
   * need to evaluate against the entire index to build a {@link
   * DocIdSetIterator}, like {@link MultiTermQuery}, point-based queries or
   * {@link TermInSetQuery}, and 5 for other filters.
   */
protected:
  virtual int minFrequencyToCache(std::shared_ptr<Query> query);

public:
  void onUse(std::shared_ptr<Query> query) override;

  virtual int frequency(std::shared_ptr<Query> query);

  bool shouldCache(std::shared_ptr<Query> query)  override;
};

} // namespace org::apache::lucene::search
