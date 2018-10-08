#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class Scorer;
}
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
namespace org::apache::lucene::index
{
class FieldInvertState;
}
namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class SimWeight;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::util
{
class BytesRef;
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

using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSubScorerFreqs : public LuceneTestCase
{
  GET_CLASS_NAME(TestSubScorerFreqs)

private:
  static std::shared_ptr<Directory> dir;
  static std::shared_ptr<IndexSearcher> s;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void makeIndex() throws Exception
  static void makeIndex() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void finish() throws Exception
  static void finish() ;

private:
  class CountingCollector : public FilterCollector
  {
    GET_CLASS_NAME(CountingCollector)
  public:
    const std::unordered_map<int,
                             std::unordered_map<std::shared_ptr<Query>, float>>
        docCounts = std::unordered_map<
            int, std::unordered_map<std::shared_ptr<Query>, float>>();

  private:
    const std::unordered_map<std::shared_ptr<Query>, std::shared_ptr<Scorer>>
        subScorers = std::unordered_map<std::shared_ptr<Query>,
                                        std::shared_ptr<Scorer>>();
    const std::shared_ptr<Set<std::wstring>> relationships;

  public:
    CountingCollector(std::shared_ptr<Collector> other);

    CountingCollector(std::shared_ptr<Collector> other,
                      std::shared_ptr<Set<std::wstring>> relationships);

    virtual void
    setSubScorers(std::shared_ptr<Scorer> scorer,
                  const std::wstring &relationship) ;

    std::shared_ptr<LeafCollector> getLeafCollector(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class FilterLeafCollectorAnonymousInnerClass : public FilterLeafCollector
    {
      GET_CLASS_NAME(FilterLeafCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<CountingCollector> outerInstance;

      int docBase = 0;

    public:
      FilterLeafCollectorAnonymousInnerClass(
          std::shared_ptr<CountingCollector> outerInstance,
          std::shared_ptr<org::apache::lucene::search::LeafCollector>
              getLeafCollector,
          int docBase);

      void collect(int doc)  override;

      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

    protected:
      std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterLeafCollectorAnonymousInnerClass>(
            FilterLeafCollector::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<CountingCollector> shared_from_this()
    {
      return std::static_pointer_cast<CountingCollector>(
          FilterCollector::shared_from_this());
    }
  };

private:
  static constexpr float FLOAT_TOLERANCE = 0.00001F;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTermQuery() throws Exception
  virtual void testTermQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBooleanQuery() throws Exception
  virtual void testBooleanQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPhraseQuery() throws Exception
  virtual void testPhraseQuery() ;

  // Similarity that just returns the frequency as the score
private:
  class CountingSimilarity : public Similarity
  {
    GET_CLASS_NAME(CountingSimilarity)

  public:
    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> weight,
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class SimScorerAnonymousInnerClass : public SimScorer
    {
      GET_CLASS_NAME(SimScorerAnonymousInnerClass)
    private:
      std::shared_ptr<CountingSimilarity> outerInstance;

    public:
      SimScorerAnonymousInnerClass(
          std::shared_ptr<CountingSimilarity> outerInstance);

      float score(int doc, float freq)  override;

      float computeSlopFactor(int distance) override;

      float computePayloadFactor(int doc, int start, int end,
                                 std::shared_ptr<BytesRef> payload) override;

    protected:
      std::shared_ptr<SimScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimScorerAnonymousInnerClass>(
            SimScorer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<CountingSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<CountingSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSubScorerFreqs> shared_from_this()
  {
    return std::static_pointer_cast<TestSubScorerFreqs>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
