#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::document
{
class Document;
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
namespace org::apache::lucene::index
{

using Document = org::apache::lucene::document::Document;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Similarity = org::apache::lucene::search::similarities::Similarity;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Abstract class to do basic tests for a norms format.
 * NOTE: This test focuses on the norms impl, nothing else.
 * The [stretch] goal is for this test to be
 * so thorough in testing a new NormsFormat that if this
 * test passes, then all Lucene/Solr tests should also pass.  Ie,
 * if there is some bug in a given NormsFormat that this
 * test fails to catch then this test needs to be improved! */
class BaseNormsFormatTestCase : public BaseIndexFileFormatTestCase
{
  GET_CLASS_NAME(BaseNormsFormatTestCase)

  /** Whether the codec supports sparse values. */
protected:
  virtual bool codecSupportsSparsity();

public:
  virtual void testByteRange() ;

  virtual void testSparseByteRange() ;

  virtual void testShortRange() ;

  virtual void testSparseShortRange() ;

  virtual void testLongRange() ;

  virtual void testSparseLongRange() ;

  virtual void testFullLongRange() ;

  virtual void testSparseFullLongRange() ;

  virtual void testFewValues() ;

  virtual void testFewSparseValues() ;

  virtual void testFewLargeValues() ;

  virtual void testFewSparseLargeValues() ;

  virtual void testAllZeros() ;

  virtual void testSparseAllZeros() ;

  virtual void testMostZeros() ;

  virtual void testOutliers() ;

  virtual void testSparseOutliers() ;

  virtual void testOutliers2() ;

  virtual void testSparseOutliers2() ;

  virtual void testNCommon() ;

  virtual void testSparseNCommon() ;

  /**
   * a more thorough n-common that tests all low bpv
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testNCommonBig() throws Exception
  virtual void testNCommonBig() ;

  /**
   * a more thorough n-common that tests all low bpv and sparse docs
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testSparseNCommonBig() throws Exception
  virtual void testSparseNCommonBig() ;

private:
  void doTestNormsVersusDocValues(
      double density,
      std::function<int64_t()> &longs) ;

  void
  checkNormsVsDocValues(std::shared_ptr<IndexReader> ir) ;

public:
  class CannedNormSimilarity : public Similarity
  {
    GET_CLASS_NAME(CannedNormSimilarity)
  public:
    std::deque<int64_t> const norms;
    int index = 0;

    CannedNormSimilarity(std::deque<int64_t> &norms);

    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> weight,
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<CannedNormSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<CannedNormSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

protected:
  void addRandomFields(std::shared_ptr<Document> doc) override;

public:
  void testMergeStability()  override;

  // TODO: test thread safety (e.g. across different fields) explicitly here

  /*
   * LUCENE-6006: Tests undead norms.
   *                                 .....
   *                             C C  /
   *                            /<   /
   *             ___ __________/_#__=o
   *            /(- /(\_\________   \
   *            \ ) \ )_      \o     \
   *            /|\ /|\       |'     |
   *                          |     _|
   *                          /o   __\
   *                         / '     |
   *                        / /      |
   *                       /_/\______|
   *                      (   _(    <
   *                       \    \    \
   *                        \    \    |
   *                         \____\____\
   *                         ____\_\__\_\
   *                       /`   /`     o\
   *                       |___ |_______|
   *
   */
  virtual void testUndeadNorms() ;

  virtual void testThreads() ;

protected:
  std::shared_ptr<BaseNormsFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseNormsFormatTestCase>(
        BaseIndexFileFormatTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
