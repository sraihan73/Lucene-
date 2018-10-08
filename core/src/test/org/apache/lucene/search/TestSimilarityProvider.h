#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
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

using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PerFieldSimilarityWrapper =
    org::apache::lucene::search::similarities::PerFieldSimilarityWrapper;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSimilarityProvider : public LuceneTestCase
{
  GET_CLASS_NAME(TestSimilarityProvider)
private:
  std::shared_ptr<Directory> directory;
  std::shared_ptr<DirectoryReader> reader;
  std::shared_ptr<IndexSearcher> searcher;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testBasics() ;

private:
  class ExampleSimilarityProvider : public PerFieldSimilarityWrapper
  {
    GET_CLASS_NAME(ExampleSimilarityProvider)
  private:
    std::shared_ptr<Similarity> sim1 = std::make_shared<Sim1>();
    std::shared_ptr<Similarity> sim2 = std::make_shared<Sim2>();

  public:
    std::shared_ptr<Similarity> get(const std::wstring &field) override;

  protected:
    std::shared_ptr<ExampleSimilarityProvider> shared_from_this()
    {
      return std::static_pointer_cast<ExampleSimilarityProvider>(
          org.apache.lucene.search.similarities
              .PerFieldSimilarityWrapper::shared_from_this());
    }
  };

private:
  class Sim1 : public Similarity
  {
    GET_CLASS_NAME(Sim1)

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
      std::shared_ptr<Sim1> outerInstance;

    public:
      SimWeightAnonymousInnerClass(std::shared_ptr<Sim1> outerInstance);

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
      std::shared_ptr<Sim1> outerInstance;

    public:
      SimScorerAnonymousInnerClass(std::shared_ptr<Sim1> outerInstance);

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
    std::shared_ptr<Sim1> shared_from_this()
    {
      return std::static_pointer_cast<Sim1>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

private:
  class Sim2 : public Similarity
  {
    GET_CLASS_NAME(Sim2)

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
      std::shared_ptr<Sim2> outerInstance;

    public:
      SimWeightAnonymousInnerClass(std::shared_ptr<Sim2> outerInstance);

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
      std::shared_ptr<Sim2> outerInstance;

    public:
      SimScorerAnonymousInnerClass(std::shared_ptr<Sim2> outerInstance);

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
    std::shared_ptr<Sim2> shared_from_this()
    {
      return std::static_pointer_cast<Sim2>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSimilarityProvider> shared_from_this()
  {
    return std::static_pointer_cast<TestSimilarityProvider>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
