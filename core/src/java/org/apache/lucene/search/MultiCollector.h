#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Collector;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class LeafCollector;
}
namespace org::apache::lucene::search
{
class Scorer;
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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

/**
 * A {@link Collector} which allows running a search with several
 * {@link Collector}s. It offers a static {@link #wrap} method which accepts a
 * deque of collectors and wraps them with {@link MultiCollector}, while
 * filtering out the <code>null</code> null ones.
 */
class MultiCollector : public std::enable_shared_from_this<MultiCollector>,
                       public Collector
{
  GET_CLASS_NAME(MultiCollector)

  /** See {@link #wrap(Iterable)}. */
public:
  static std::shared_ptr<Collector> wrap(std::deque<Collector> &collectors);

  /**
   * Wraps a deque of {@link Collector}s with a {@link MultiCollector}. This
   * method works as follows:
   * <ul>
   * <li>Filters out the <code>null</code> collectors, so they are not used
   * during search time.
   * <li>If the input contains 1 real collector (i.e. non-<code>null</code> ),
   * it is returned.
   * <li>Otherwise the method returns a {@link MultiCollector} which wraps the
   * non-<code>null</code> ones.
   * </ul>
   *
   * @throws IllegalArgumentException
   *           if either 0 collectors were input, or all collectors are
   *           <code>null</code>.
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static Collector wrap(Iterable<? extends
  // Collector> collectors)
  static std::shared_ptr<Collector> wrap(std::deque<T1> collectors);

private:
  const bool cacheScores;
  std::deque<std::shared_ptr<Collector>> const collectors;

  MultiCollector(std::deque<Collector> &collectors);

public:
  bool needsScores() override;

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class MultiLeafCollector
      : public std::enable_shared_from_this<MultiLeafCollector>,
        public LeafCollector
  {
    GET_CLASS_NAME(MultiLeafCollector)

  private:
    const bool cacheScores;
    std::deque<std::shared_ptr<LeafCollector>> const collectors;
    int numCollectors = 0;

    MultiLeafCollector(std::deque<std::shared_ptr<LeafCollector>> &collectors,
                       bool cacheScores);

  public:
    void setScorer(std::shared_ptr<Scorer> scorer)  override;

  private:
    void removeCollector(int i);

  public:
    void collect(int doc)  override;
  };
};

} // namespace org::apache::lucene::search
