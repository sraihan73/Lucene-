#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
namespace org::apache::lucene::search
{
template <typename T>
class TopDocsCollector;
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
using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTopDocsCollector : public LuceneTestCase
{
  GET_CLASS_NAME(TestTopDocsCollector)

private:
  class MyTopsDocCollector final
      : public TopDocsCollector<std::shared_ptr<ScoreDoc>>
  {
    GET_CLASS_NAME(MyTopsDocCollector)

  private:
    int idx = 0;

  public:
    MyTopsDocCollector(int size);

  protected:
    std::shared_ptr<TopDocs>
    newTopDocs(std::deque<std::shared_ptr<ScoreDoc>> &results,
               int start) override;

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
      std::shared_ptr<MyTopsDocCollector> outerInstance;

      int base = 0;

    public:
      LeafCollectorAnonymousInnerClass(
          std::shared_ptr<MyTopsDocCollector> outerInstance, int base);

      void collect(int doc) override;

      void setScorer(std::shared_ptr<Scorer> scorer) override;
    };

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<MyTopsDocCollector> shared_from_this()
    {
      return std::static_pointer_cast<MyTopsDocCollector>(
          TopDocsCollector<ScoreDoc>::shared_from_this());
    }
  };

  // Scores array to be used by MyTopDocsCollector. If it is changed, MAX_SCORE
  // must also change.
private:
  static std::deque<float> const scores;

  static constexpr float MAX_SCORE = 9.17561f;

  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;

  std::shared_ptr<TopDocsCollector<std::shared_ptr<ScoreDoc>>>
  doSearch(int numResults) ;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testInvalidArguments() ;

  virtual void testZeroResults() ;

  virtual void testFirstResultsPage() ;

  virtual void testSecondResultsPages() ;

  virtual void testGetAllResults() ;

  virtual void testGetResultsFromStart() ;

  virtual void testMaxScore() ;

  // This does not test the PQ's correctness, but whether topDocs()
  // implementations return the results in decreasing score order.
  virtual void testResultsOrder() ;

protected:
  std::shared_ptr<TestTopDocsCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestTopDocsCollector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
