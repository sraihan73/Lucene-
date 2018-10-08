#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/LongBitSet.h"

#include  "core/src/java/org/apache/lucene/index/OrdinalMap.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/util/LongValues.h"
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
namespace org::apache::lucene::search::join
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Collector = org::apache::lucene::search::Collector;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Scorer = org::apache::lucene::search::Scorer;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using LongValues = org::apache::lucene::util::LongValues;

/**
 * A collector that collects all ordinals from a specified field matching the
 * query.
 *
 * @lucene.experimental
 */
class GlobalOrdinalsCollector final
    : public std::enable_shared_from_this<GlobalOrdinalsCollector>,
      public Collector
{
  GET_CLASS_NAME(GlobalOrdinalsCollector)

public:
  const std::wstring field;
  const std::shared_ptr<LongBitSet> collectedOrds;
  const std::shared_ptr<OrdinalMap> ordinalMap;

  GlobalOrdinalsCollector(const std::wstring &field,
                          std::shared_ptr<OrdinalMap> ordinalMap,
                          int64_t valueCount);

  std::shared_ptr<LongBitSet> getCollectorOrdinals();

  bool needsScores() override;

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

public:
  class OrdinalMapCollector final
      : public std::enable_shared_from_this<OrdinalMapCollector>,
        public LeafCollector
  {
    GET_CLASS_NAME(OrdinalMapCollector)
  private:
    std::shared_ptr<GlobalOrdinalsCollector> outerInstance;

    const std::shared_ptr<SortedDocValues> docTermOrds;
    const std::shared_ptr<LongValues> segmentOrdToGlobalOrdLookup;

  public:
    OrdinalMapCollector(
        std::shared_ptr<GlobalOrdinalsCollector> outerInstance,
        std::shared_ptr<SortedDocValues> docTermOrds,
        std::shared_ptr<LongValues> segmentOrdToGlobalOrdLookup);

    void collect(int doc)  override;

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
  };

public:
  class SegmentOrdinalCollector final
      : public std::enable_shared_from_this<SegmentOrdinalCollector>,
        public LeafCollector
  {
    GET_CLASS_NAME(SegmentOrdinalCollector)
  private:
    std::shared_ptr<GlobalOrdinalsCollector> outerInstance;

    const std::shared_ptr<SortedDocValues> docTermOrds;

  public:
    SegmentOrdinalCollector(
        std::shared_ptr<GlobalOrdinalsCollector> outerInstance,
        std::shared_ptr<SortedDocValues> docTermOrds);

    void collect(int doc)  override;

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
  };
};

} // #include  "core/src/java/org/apache/lucene/search/join/
