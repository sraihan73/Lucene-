#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::document
{
class Document;
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
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search
{
class Scorer;
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

class TestConjunctions : public LuceneTestCase
{
  GET_CLASS_NAME(TestConjunctions)
public:
  std::shared_ptr<Analyzer> analyzer;
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;

  static const std::wstring F1;
  static const std::wstring F2;

  void setUp()  override;

  static std::shared_ptr<Document> doc(const std::wstring &v1,
                                       const std::wstring &v2);

  virtual void testTermConjunctionsWithOmitTF() ;

  void tearDown()  override;

  // Similarity that returns the TF as score
private:
  class TFSimilarity : public Similarity
  {
    GET_CLASS_NAME(TFSimilarity)

  public:
    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

  private:
    class SimWeightAnonymousInnerClass : public SimWeight
    {
      GET_CLASS_NAME(SimWeightAnonymousInnerClass)
    private:
      std::shared_ptr<TFSimilarity> outerInstance;

    public:
      SimWeightAnonymousInnerClass(std::shared_ptr<TFSimilarity> outerInstance);

    protected:
      std::shared_ptr<SimWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimWeightAnonymousInnerClass>(
            SimWeight::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> weight,
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class SimScorerAnonymousInnerClass : public SimScorer
    {
      GET_CLASS_NAME(SimScorerAnonymousInnerClass)
    private:
      std::shared_ptr<TFSimilarity> outerInstance;

    public:
      SimScorerAnonymousInnerClass(std::shared_ptr<TFSimilarity> outerInstance);

      float score(int doc, float freq) override;

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
    std::shared_ptr<TFSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<TFSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

public:
  virtual void testScorerGetChildren() ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestConjunctions> outerInstance;

    std::shared_ptr<Document> doc;
    std::shared_ptr<org::apache::lucene::search::IndexSearcher> s;
    std::deque<bool> setScorerCalled;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestConjunctions> outerInstance,
        std::shared_ptr<Document> doc,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> s,
        std::deque<bool> &setScorerCalled);

    void setScorer(std::shared_ptr<Scorer> s)  override;

    void collect(int doc) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestConjunctions> shared_from_this()
  {
    return std::static_pointer_cast<TestConjunctions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
