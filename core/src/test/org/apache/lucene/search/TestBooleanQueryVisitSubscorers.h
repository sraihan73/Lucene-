#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class Scorer;
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
class TopDocs;
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Document = org::apache::lucene::document::Document;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// TODO: refactor to a base class, that collects freqs from the scorer tree
// and test all queries with it
class TestBooleanQueryVisitSubscorers : public LuceneTestCase
{
  GET_CLASS_NAME(TestBooleanQueryVisitSubscorers)

public:
  std::shared_ptr<Analyzer> analyzer;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexSearcher> scorerSearcher;
  std::shared_ptr<Directory> dir;

  static const std::wstring F1;
  static const std::wstring F2;

  void setUp()  override;

  void tearDown()  override;

  virtual void testDisjunctions() ;

  virtual void testNestedDisjunctions() ;

  virtual void testConjunctions() ;

  static std::shared_ptr<Document> doc(const std::wstring &v1,
                                       const std::wstring &v2);

  static std::unordered_map<int, int>
  getDocCounts(std::shared_ptr<IndexSearcher> searcher,
               std::shared_ptr<Query> query) ;

public:
  class MyCollector : public FilterCollector
  {
    GET_CLASS_NAME(MyCollector)

  public:
    const std::unordered_map<int, int> docCounts =
        std::unordered_map<int, int>();

  private:
    const std::shared_ptr<Set<std::shared_ptr<Scorer>>> tqsSet =
        std::unordered_set<std::shared_ptr<Scorer>>();

  public:
    MyCollector();

    std::shared_ptr<LeafCollector> getLeafCollector(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class FilterLeafCollectorAnonymousInnerClass : public FilterLeafCollector
    {
      GET_CLASS_NAME(FilterLeafCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<MyCollector> outerInstance;

      int docBase = 0;

    public:
      FilterLeafCollectorAnonymousInnerClass(
          std::shared_ptr<MyCollector> outerInstance,
          std::shared_ptr<org::apache::lucene::search::LeafCollector>
              getLeafCollector,
          int docBase);

      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

      void collect(int doc)  override;

    protected:
      std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterLeafCollectorAnonymousInnerClass>(
            FilterLeafCollector::shared_from_this());
      }
    };

  private:
    void fillLeaves(
        std::shared_ptr<Scorer> scorer,
        std::shared_ptr<Set<std::shared_ptr<Scorer>>> set) ;

  public:
    virtual std::shared_ptr<TopDocs> topDocs();

    virtual int freq(int doc) ;

  protected:
    std::shared_ptr<MyCollector> shared_from_this()
    {
      return std::static_pointer_cast<MyCollector>(
          FilterCollector::shared_from_this());
    }
  };

public:
  virtual void testDisjunctionMatches() ;

  virtual void testMinShouldMatchMatches() ;

  virtual void testGetChildrenMinShouldMatchSumScorer() ;

  virtual void testGetChildrenBoosterScorer() ;

private:
  class ScorerSummarizingCollector
      : public std::enable_shared_from_this<ScorerSummarizingCollector>,
        public Collector
  {
    GET_CLASS_NAME(ScorerSummarizingCollector)
  private:
    const std::deque<std::wstring> summaries = std::deque<std::wstring>();
    std::deque<int> numHits = std::deque<int>(1);

  public:
    virtual int getNumHits();

    virtual std::deque<std::wstring> getSummaries();

    bool needsScores() override;

    std::shared_ptr<LeafCollector> getLeafCollector(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class LeafCollectorAnonymousInnerClass
        : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
          public LeafCollector
    {
      GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<ScorerSummarizingCollector> outerInstance;

    public:
      LeafCollectorAnonymousInnerClass(
          std::shared_ptr<ScorerSummarizingCollector> outerInstance);

      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

      void collect(int doc)  override;
    };

  private:
    static void summarizeScorer(std::shared_ptr<StringBuilder> builder,
                                std::shared_ptr<Scorer> scorer,
                                int const indent) ;

    static std::shared_ptr<StringBuilder>
    indent(std::shared_ptr<StringBuilder> builder, int const indent);
  };

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
  std::shared_ptr<TestBooleanQueryVisitSubscorers> shared_from_this()
  {
    return std::static_pointer_cast<TestBooleanQueryVisitSubscorers>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
