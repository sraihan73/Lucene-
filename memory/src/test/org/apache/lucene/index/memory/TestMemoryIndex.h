#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/MockAnalyzer.h"

#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"
#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"

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
namespace org::apache::lucene::index::memory
{

using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.hamcrest.CoreMatchers.is;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.hamcrest.CoreMatchers.not;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.@internal.matchers.StringContains.containsString;

class TestMemoryIndex : public LuceneTestCase
{
  GET_CLASS_NAME(TestMemoryIndex)

private:
  std::shared_ptr<MockAnalyzer> analyzer;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void setup()
  virtual void setup();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFreezeAPI()
  virtual void testFreezeAPI();

  virtual void testSeekByTermOrd() ;

  virtual void testFieldsOnlyReturnsIndexedFields() ;

  virtual void testReaderConsistency() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSimilarities() throws
  // java.io.IOException
  virtual void testSimilarities() ;

private:
  class SimilarityAnonymousInnerClass : public Similarity
  {
    GET_CLASS_NAME(SimilarityAnonymousInnerClass)
  private:
    std::shared_ptr<TestMemoryIndex> outerInstance;

  public:
    SimilarityAnonymousInnerClass(
        std::shared_ptr<TestMemoryIndex> outerInstance);

    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> weight,
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<SimilarityAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimilarityAnonymousInnerClass>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOmitNorms() throws java.io.IOException
  virtual void testOmitNorms() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBuildFromDocument()
  virtual void testBuildFromDocument();

  virtual void testDocValues() ;

  virtual void testDocValues_resetIterator() ;

  virtual void testInvalidDocValuesUsage() ;

  virtual void
  testDocValuesDoNotAffectBoostPositionsOrOffset() ;

  virtual void testPointValues() ;

  virtual void testMissingPoints() ;

  virtual void
  testPointValuesDoNotAffectPositionsOrOffset() ;

  virtual void test2DPoints() ;

  virtual void testMultiValuedPointsSortedCorrectly() ;

  virtual void testIndexingPointsAndDocValues() ;

  virtual void testToStringDebug();

protected:
  std::shared_ptr<TestMemoryIndex> shared_from_this()
  {
    return std::static_pointer_cast<TestMemoryIndex>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/memory/
