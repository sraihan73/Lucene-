#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
template <typename T>
class DocValuesStats;
}

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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

/** A {@link Collector} which computes statistics for a DocValues field. */
class DocValuesStatsCollector
    : public std::enable_shared_from_this<DocValuesStatsCollector>,
      public Collector
{
  GET_CLASS_NAME(DocValuesStatsCollector)

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private final DocValuesStats<?> stats;
  const std::shared_ptr < DocValuesStats < ? >> stats;

  /** Creates a collector to compute statistics for a DocValues field using the
   * given {@code stats}. */
public:
  template <typename T1>
  DocValuesStatsCollector(std::shared_ptr<DocValuesStats<T1>> stats);

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class LeafCollectorAnonymousInnerClass
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesStatsCollector> outerInstance;

  public:
    LeafCollectorAnonymousInnerClass(
        std::shared_ptr<DocValuesStatsCollector> outerInstance);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
    void collect(int doc)  override;
  };

private:
  class LeafCollectorAnonymousInnerClass2
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass2>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<DocValuesStatsCollector> outerInstance;

  public:
    LeafCollectorAnonymousInnerClass2(
        std::shared_ptr<DocValuesStatsCollector> outerInstance);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
    void collect(int doc)  override;
  };

public:
  bool needsScores() override;
};

} // #include  "core/src/java/org/apache/lucene/search/
