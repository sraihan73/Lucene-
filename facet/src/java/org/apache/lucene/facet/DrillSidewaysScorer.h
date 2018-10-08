#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Collector.h"

#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/facet/DocsAndCost.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/search/ChildScorer.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"

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
namespace org::apache::lucene::facet
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using BulkScorer = org::apache::lucene::search::BulkScorer;
using Collector = org::apache::lucene::search::Collector;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using Bits = org::apache::lucene::util::Bits;

class DrillSidewaysScorer : public BulkScorer
{
  GET_CLASS_NAME(DrillSidewaysScorer)

  // private static bool DEBUG = false;

private:
  const std::shared_ptr<Collector> drillDownCollector;
  std::shared_ptr<LeafCollector> drillDownLeafCollector;

  std::deque<std::shared_ptr<DocsAndCost>> const dims;

  // DrillDown DocsEnums:
  const std::shared_ptr<Scorer> baseScorer;
  const std::shared_ptr<DocIdSetIterator> baseIterator;

  const std::shared_ptr<LeafReaderContext> context;

public:
  const bool scoreSubDocsAtOnce;

private:
  static constexpr int CHUNK = 2048;
  static const int MASK = CHUNK - 1;

  int collectDocID = -1;
  float collectScore = 0;

public:
  DrillSidewaysScorer(std::shared_ptr<LeafReaderContext> context,
                      std::shared_ptr<Scorer> baseScorer,
                      std::shared_ptr<Collector> drillDownCollector,
                      std::deque<std::shared_ptr<DocsAndCost>> &dims,
                      bool scoreSubDocsAtOnce);

  int64_t cost() override;

  int score(std::shared_ptr<LeafCollector> collector,
            std::shared_ptr<Bits> acceptDocs, int min,
            int maxDoc)  override;

  /** Used when base query is highly constraining vs the
   *  drilldowns, or when the docs must be scored at once
   *  (i.e., like BooleanScorer2, not BooleanScorer).  In
   *  this case we just .next() on base and .advance() on
   *  the dim filters. */
private:
  void doQueryFirstScoring(
      std::shared_ptr<Bits> acceptDocs,
      std::shared_ptr<LeafCollector> collector,
      std::deque<std::shared_ptr<DocsAndCost>> &dims) ;

  /** Used when drill downs are highly constraining vs
   *  baseQuery. */
  void doDrillDownAdvanceScoring(
      std::shared_ptr<Bits> acceptDocs,
      std::shared_ptr<LeafCollector> collector,
      std::deque<std::shared_ptr<DocsAndCost>> &dims) ;

  void doUnionScoring(
      std::shared_ptr<Bits> acceptDocs,
      std::shared_ptr<LeafCollector> collector,
      std::deque<std::shared_ptr<DocsAndCost>> &dims) ;

  void collectHit(
      std::shared_ptr<LeafCollector> collector,
      std::deque<std::shared_ptr<DocsAndCost>> &dims) ;

  void collectNearMiss(std::shared_ptr<LeafCollector> sidewaysCollector) throw(
      IOException);

private:
  class FakeScorer final : public Scorer
  {
    GET_CLASS_NAME(FakeScorer)
  private:
    std::shared_ptr<DrillSidewaysScorer> outerInstance;

  public:
    FakeScorer(std::shared_ptr<DrillSidewaysScorer> outerInstance);

    int docID() override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

    float score() override;

    std::shared_ptr<std::deque<std::shared_ptr<Scorer::ChildScorer>>>
    getChildren() override;

    std::shared_ptr<Weight> getWeight() override;

  protected:
    std::shared_ptr<FakeScorer> shared_from_this()
    {
      return std::static_pointer_cast<FakeScorer>(
          org.apache.lucene.search.Scorer::shared_from_this());
    }
  };

public:
  class DocsAndCost : public std::enable_shared_from_this<DocsAndCost>
  {
    GET_CLASS_NAME(DocsAndCost)
    // approximation of matching docs, or the scorer itself
  public:
    const std::shared_ptr<DocIdSetIterator> approximation;
    // two-phase confirmation, or null if the approximation is accurate
    const std::shared_ptr<TwoPhaseIterator> twoPhase;
    const std::shared_ptr<Collector> sidewaysCollector;
    std::shared_ptr<LeafCollector> sidewaysLeafCollector;

    DocsAndCost(std::shared_ptr<Scorer> scorer,
                std::shared_ptr<Collector> sidewaysCollector);
  };

protected:
  std::shared_ptr<DrillSidewaysScorer> shared_from_this()
  {
    return std::static_pointer_cast<DrillSidewaysScorer>(
        org.apache.lucene.search.BulkScorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
