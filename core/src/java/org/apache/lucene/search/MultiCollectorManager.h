#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Collectors.h"

#include  "core/src/java/org/apache/lucene/search/Collector.h"
namespace org::apache::lucene::search
{
template <typename Ctypename T>
class CollectorManager;
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

/**
 * A {@link CollectorManager} implements which wrap a set of {@link
 * CollectorManager} as {@link MultiCollector} acts for {@link Collector}.
 */
class MultiCollectorManager
    : public std::enable_shared_from_this<MultiCollectorManager>,
      public CollectorManager<
          std::shared_ptr<MultiCollectorManager::Collectors>,
          std::deque<std::any>>
{
  GET_CLASS_NAME(MultiCollectorManager)

private:
  std::deque < CollectorManager < std::shared_ptr<Collector>,
      ? >> const collectorManagers;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SafeVarargs @SuppressWarnings({"varargs", "unchecked"})
  // public MultiCollectorManager(final CollectorManager<? extends Collector,
  // ?>... collectorManagers) C++ TODO: Java wildcard generics are not converted
  // to C++:
      MultiCollectorManager(std::deque<CollectorManager<? extends Collector, ?>> &collectorManagers);

      std::shared_ptr<Collectors> newCollector()  override;

      std::deque<std::any>
      reduce(std::shared_ptr<std::deque<std::shared_ptr<Collectors>>>
                 reducableCollectors)  override;

    public:
      class Collectors : public std::enable_shared_from_this<Collectors>,
                         public Collector
      {
        GET_CLASS_NAME(Collectors)
      private:
        std::shared_ptr<MultiCollectorManager> outerInstance;

        std::deque<std::shared_ptr<Collector>> const collectors;

        Collectors(std::shared_ptr<MultiCollectorManager> outerInstance) throw(
            IOException);

      public:
        std::shared_ptr<LeafCollector>
        getLeafCollector(std::shared_ptr<LeafReaderContext> context) throw(
            IOException) override;

        bool needsScores() override;

      public:
        class LeafCollectors
            : public std::enable_shared_from_this<LeafCollectors>,
              public LeafCollector
        {
          GET_CLASS_NAME(LeafCollectors)
        private:
          std::shared_ptr<MultiCollectorManager::Collectors> outerInstance;

          std::deque<std::shared_ptr<LeafCollector>> const leafCollectors;

          LeafCollectors(
              std::shared_ptr<MultiCollectorManager::Collectors> outerInstance,
              std::shared_ptr<LeafReaderContext> context) ;

        public:
          void
          setScorer(std::shared_ptr<Scorer> scorer)  override;

          void collect(int const doc)  override;
        };
      };
};

} // #include  "core/src/java/org/apache/lucene/search/
