#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/search/TopFieldCollector.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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

using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTopFieldCollector : public LuceneTestCase
{
  GET_CLASS_NAME(TestTopFieldCollector)
private:
  std::shared_ptr<IndexSearcher> is;
  std::shared_ptr<IndexReader> ir;
  std::shared_ptr<Directory> dir;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testSortWithoutFillFields() ;

  virtual void testSortWithoutScoreTracking() ;

  virtual void testSortWithoutTotalHitTracking() ;

  virtual void testSortWithScoreNoMaxScoreTracking() ;

  // MultiComparatorScoringNoMaxScoreCollector
  virtual void
  testSortWithScoreNoMaxScoreTrackingMulti() ;

  virtual void testSortWithScoreAndMaxScoreTracking() ;

  virtual void
  testSortWithScoreAndMaxScoreTrackingNoResults() ;

  virtual void testComputeScoresOnlyOnce() ;

private:
  class CollectorAnonymousInnerClass
      : public std::enable_shared_from_this<CollectorAnonymousInnerClass>,
        public Collector
  {
    GET_CLASS_NAME(CollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestTopFieldCollector> outerInstance;

    std::shared_ptr<org::apache::lucene::search::TopFieldCollector>
        topCollector;

  public:
    CollectorAnonymousInnerClass(
        std::shared_ptr<TestTopFieldCollector> outerInstance,
        std::shared_ptr<org::apache::lucene::search::TopFieldCollector>
            topCollector);

    std::shared_ptr<LeafCollector> getLeafCollector(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class FilterLeafCollectorAnonymousInnerClass : public FilterLeafCollector
    {
      GET_CLASS_NAME(FilterLeafCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<CollectorAnonymousInnerClass> outerInstance;

    public:
      FilterLeafCollectorAnonymousInnerClass(
          std::shared_ptr<CollectorAnonymousInnerClass> outerInstance);

      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

    private:
      class ScorerAnonymousInnerClass : public Scorer
      {
        GET_CLASS_NAME(ScorerAnonymousInnerClass)
      private:
        std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> outerInstance;

        std::shared_ptr<org::apache::lucene::search::Scorer> scorer;

      public:
        ScorerAnonymousInnerClass(
            std::shared_ptr<FilterLeafCollectorAnonymousInnerClass>
                outerInstance,
            std::shared_ptr<org::apache::lucene::search::Scorer> scorer);

        int lastComputedDoc = 0;

        float score()  override;

        int docID() override;

        std::shared_ptr<DocIdSetIterator> iterator() override;

      protected:
        std::shared_ptr<ScorerAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<ScorerAnonymousInnerClass>(
              Scorer::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterLeafCollectorAnonymousInnerClass>(
            FilterLeafCollector::shared_from_this());
      }
    };

  public:
    bool needsScores() override;
  };

protected:
  std::shared_ptr<TestTopFieldCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestTopFieldCollector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
