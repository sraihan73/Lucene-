#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"
#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::index
{

using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests the maxTermFrequency statistic in FieldInvertState
 */
class TestMaxTermFrequency : public LuceneTestCase
{
  GET_CLASS_NAME(TestMaxTermFrequency)
public:
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;
  /* expected maxTermFrequency values for our documents */
  std::deque<int> expected = std::deque<int>();

  void setUp()  override;

  void tearDown()  override;

  virtual void test() ;

  /**
   * Makes a bunch of single-char tokens (the max freq will at most be 255).
   * shuffles them around, and returns the whole deque with Arrays.toString().
   * This works fine because we use lettertokenizer.
   * puts the max-frequency term into expected, to be checked against the norm.
   */
private:
  std::wstring addValue();

  /**
   * Simple similarity that encodes maxTermFrequency directly as a byte
   */
public:
  class TestSimilarity : public Similarity
  {
    GET_CLASS_NAME(TestSimilarity)

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
      std::shared_ptr<TestSimilarity> outerInstance;

    public:
      SimWeightAnonymousInnerClass(
          std::shared_ptr<TestSimilarity> outerInstance);

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
      std::shared_ptr<TestSimilarity> outerInstance;

    public:
      SimScorerAnonymousInnerClass(
          std::shared_ptr<TestSimilarity> outerInstance);

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
    std::shared_ptr<TestSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<TestSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestMaxTermFrequency> shared_from_this()
  {
    return std::static_pointer_cast<TestMaxTermFrequency>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
