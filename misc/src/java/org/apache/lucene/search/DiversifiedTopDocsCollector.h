#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <deque>
#include "core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include "core/src/java/org/apache/lucene/search/TopDocsCollector.h"

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
  /**
   * An extension to ScoreDoc that includes a key used for grouping purposes
   */
  class ScoreDocKey : public ScoreDoc
  {
    GET_CLASS_NAME(ScoreDocKey)
  public:
    std::optional<int64_t> key;

    ScoreDocKey(int doc, float score);

    virtual std::optional<int64_t> getKey();

    virtual std::wstring toString();

  protected:
    std::shared_ptr<ScoreDocKey> shared_from_this()
    {
      return std::static_pointer_cast<ScoreDocKey>(
          ScoreDoc::shared_from_this());
    }
  };
  bool operator < (std::shared_ptr<ScoreDocKey> const hitA, std::shared_ptr<ScoreDocKey> const hitB);
}

namespace org::apache::lucene::search
{
  class ScoreDocKeyQueue : public PriorityQueue<std::shared_ptr<ScoreDocKey>>
  {
    GET_CLASS_NAME(ScoreDocKeyQueue)

  public:
    ScoreDocKeyQueue(int size = 0);

    std::shared_ptr<ScoreDocKeyQueue> shared_from_this()
    {
      return std::make_shared<ScoreDocKeyQueue>();
    }
  };
}

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
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

using org::apache::lucene::index::LeafReaderContext;
using org::apache::lucene::index::NumericDocValues;
//using org::apache::lucene::util::PriorityQueue;

/**
 * A {@link TopDocsCollector} that controls diversity in results by ensuring no
 * more than maxHitsPerKey results from a common source are collected in the
 * final results.
 *
 * An example application might be a product search in a marketplace where no
 * more than 3 results per retailer are permitted in search results.
 *
 * <p>
 * To compare behaviour with other forms of collector, a useful analogy might be
 * the problem of making a compilation album of 1967's top hit records:
 * <ol>
 * <li>A vanilla query's results might look like a "Best of the Beatles" album -
 * high quality but not much diversity</li>
 * <li>A GroupingSearch would produce the equivalent of "The 10 top-selling
 * artists of 1967 - some killer and quite a lot of filler"</li>
 * <li>A "diversified" query would be the top 20 hit records of that year - with
 * a max of 3 Beatles hits in order to maintain diversity</li>
 * </ol>
 * This collector improves on the "GroupingSearch" type queries by
 * <ul>
 * <li>Working in one pass over the data</li>
 * <li>Not requiring the client to guess how many groups are required</li>
 * <li>Removing low-scoring "filler" which sits at the end of each group's
 * hits</li>
 * </ul>
 *
 * This is an abstract class and subclasses have to provide a source of keys for
 * documents which is then used to help identify duplicate sources.
 *
 * @lucene.experimental
 *
 */
class DiversifiedTopDocsCollector
    : public TopDocsCollector<std::shared_ptr<ScoreDocKey>>
{
  GET_CLASS_NAME(DiversifiedTopDocsCollector)
public:
  std::shared_ptr<ScoreDocKey> spare;

private:
  std::shared_ptr<ScoreDocKeyQueue> globalQueue;
  int numHits = 0;
  std::unordered_map<int64_t, std::shared_ptr<ScoreDocKeyQueue>> perKeyQueues;

protected:
  int maxNumPerKey = 0;

private:
  std::stack<std::shared_ptr<ScoreDocKeyQueue>> sparePerKeyQueues =
      std::stack<std::shared_ptr<ScoreDocKeyQueue>>();

public:
  DiversifiedTopDocsCollector(int numHits, int maxHitsPerKey);

  /**
   * Get a source of values used for grouping keys
   */
protected:
  virtual std::shared_ptr<NumericDocValues>
  getKeys(std::shared_ptr<LeafReaderContext> context) = 0;

public:
  bool needsScores() override;

protected:
  std::shared_ptr<TopDocs>
  newTopDocs(std::deque<std::shared_ptr<ScoreDoc>> &results,
             int start) override;

  virtual std::shared_ptr<ScoreDocKey>
  insert(std::shared_ptr<ScoreDocKey> addition, int docBase,
         std::shared_ptr<NumericDocValues> keys) ;

private:
  void perKeyGroupRemove(std::shared_ptr<ScoreDocKey> globalOverflow);

public:
  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class LeafCollectorAnonymousInnerClass
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<DiversifiedTopDocsCollector> outerInstance;

    int base = 0;
    std::shared_ptr<NumericDocValues> keySource;

  public:
    LeafCollectorAnonymousInnerClass(
        std::shared_ptr<DiversifiedTopDocsCollector> outerInstance, int base,
        std::shared_ptr<NumericDocValues> keySource);

    std::shared_ptr<Scorer> scorer;

    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    void collect(int doc)  override;
  };

public:

  //
protected:
  std::shared_ptr<DiversifiedTopDocsCollector> shared_from_this()
  {
    return std::static_pointer_cast<DiversifiedTopDocsCollector>(
        TopDocsCollector<org.apache.lucene.search.DiversifiedTopDocsCollector
                             .ScoreDocKey>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
