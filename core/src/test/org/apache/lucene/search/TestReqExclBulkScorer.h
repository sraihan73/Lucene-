#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"

#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestReqExclBulkScorer : public LuceneTestCase
{
  GET_CLASS_NAME(TestReqExclBulkScorer)

public:
  virtual void testRandom() ;

  virtual void doTestRandom() ;

private:
  class BulkScorerAnonymousInnerClass : public BulkScorer
  {
    GET_CLASS_NAME(BulkScorerAnonymousInnerClass)
  private:
    std::shared_ptr<TestReqExclBulkScorer> outerInstance;

    std::shared_ptr<org::apache::lucene::search::DocIdSet> req;

  public:
    BulkScorerAnonymousInnerClass(
        std::shared_ptr<TestReqExclBulkScorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::DocIdSet> req);

    const std::shared_ptr<DocIdSetIterator> iterator;

    int score(std::shared_ptr<LeafCollector> collector,
              std::shared_ptr<Bits> acceptDocs, int min,
              int max)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<BulkScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BulkScorerAnonymousInnerClass>(
          BulkScorer::shared_from_this());
    }
  };

private:
  class LeafCollectorAnonymousInnerClass
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestReqExclBulkScorer> outerInstance;

    std::shared_ptr<FixedBitSet> actualMatches;

  public:
    LeafCollectorAnonymousInnerClass(
        std::shared_ptr<TestReqExclBulkScorer> outerInstance,
        std::shared_ptr<FixedBitSet> actualMatches);

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
    std::shared_ptr<TestReqExclBulkScorer> outerInstance;

    std::shared_ptr<FixedBitSet> actualMatches;

  public:
    LeafCollectorAnonymousInnerClass2(
        std::shared_ptr<TestReqExclBulkScorer> outerInstance,
        std::shared_ptr<FixedBitSet> actualMatches);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
    void collect(int doc)  override;
  };

protected:
  std::shared_ptr<TestReqExclBulkScorer> shared_from_this()
  {
    return std::static_pointer_cast<TestReqExclBulkScorer>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
