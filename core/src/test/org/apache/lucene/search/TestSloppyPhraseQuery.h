#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::search
{
class PhraseQuery;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
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
class IndexSearcher;
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

using Document = org::apache::lucene::document::Document;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSloppyPhraseQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestSloppyPhraseQuery)

private:
  static const std::wstring S_1;
  static const std::wstring S_2;

  static const std::shared_ptr<Document> DOC_1;
  static const std::shared_ptr<Document> DOC_2;
  static const std::shared_ptr<Document> DOC_3;
  static const std::shared_ptr<Document> DOC_1_B;
  static const std::shared_ptr<Document> DOC_2_B;
  static const std::shared_ptr<Document> DOC_3_B;
  static const std::shared_ptr<Document> DOC_4;
  static const std::shared_ptr<Document> DOC_5_3;
  static const std::shared_ptr<Document> DOC_5_4;

  static const std::shared_ptr<PhraseQuery> QUERY_1;
  static const std::shared_ptr<PhraseQuery> QUERY_2;
  static const std::shared_ptr<PhraseQuery> QUERY_4;
  static const std::shared_ptr<PhraseQuery> QUERY_5_4;

  /**
   * Test DOC_4 and QUERY_4.
   * QUERY_4 has a fuzzy (len=1) match to DOC_4, so all slop values &gt; 0
   * should succeed. But only the 3rd sequence of A's in DOC_4 will do.
   */
public:
  virtual void
  testDoc4_Query4_All_Slops_Should_match() ;

  /**
   * Test DOC_1 and QUERY_1.
   * QUERY_1 has an exact match to DOC_1, so all slop values should succeed.
   * Before LUCENE-1310, a slop value of 1 did not succeed.
   */
  virtual void
  testDoc1_Query1_All_Slops_Should_match() ;

  /**
   * Test DOC_2 and QUERY_1.
   * 6 should be the minimum slop to make QUERY_1 match DOC_2.
   * Before LUCENE-1310, 7 was the minimum.
   */
  virtual void
  testDoc2_Query1_Slop_6_or_more_Should_match() ;

  /**
   * Test DOC_2 and QUERY_2.
   * QUERY_2 has an exact match to DOC_2, so all slop values should succeed.
   * Before LUCENE-1310, 0 succeeds, 1 through 7 fail, and 8 or greater
   * succeeds.
   */
  virtual void
  testDoc2_Query2_All_Slops_Should_match() ;

  /**
   * Test DOC_3 and QUERY_1.
   * QUERY_1 has an exact match to DOC_3, so all slop values should succeed.
   */
  virtual void
  testDoc3_Query1_All_Slops_Should_match() ;

  /** LUCENE-3412 */
  virtual void
  testDoc5_Query5_Any_Slop_Should_be_consistent() ;

private:
  float checkPhraseQuery(std::shared_ptr<Document> doc,
                         std::shared_ptr<PhraseQuery> query, int slop,
                         int expectedNumResults) ;

  static std::shared_ptr<Document> makeDocument(const std::wstring &docText);

  static std::shared_ptr<PhraseQuery>
  makePhraseQuery(const std::wstring &terms);

public:
  class MaxFreqCollector : public SimpleCollector
  {
    GET_CLASS_NAME(MaxFreqCollector)
  public:
    float max = 0;
    int totalHits = 0;
    std::shared_ptr<Scorer> scorer;

    std::shared_ptr<Similarity::SimScorer> simScorer =
        std::make_shared<SimScorerAnonymousInnerClass>();

  private:
    class SimScorerAnonymousInnerClass : public Similarity::SimScorer
    {
      GET_CLASS_NAME(SimScorerAnonymousInnerClass)
    public:
      SimScorerAnonymousInnerClass();

      float score(int doc, float freq)  override;

      float computeSlopFactor(int distance) override;

      float computePayloadFactor(int doc, int start, int end,
                                 std::shared_ptr<BytesRef> payload) override;

    protected:
      std::shared_ptr<SimScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimScorerAnonymousInnerClass>(
            org.apache.lucene.search.similarities.Similarity
                .SimScorer::shared_from_this());
      }
    };

  public:
    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    void collect(int doc)  override;

    bool needsScores() override;

  protected:
    std::shared_ptr<MaxFreqCollector> shared_from_this()
    {
      return std::static_pointer_cast<MaxFreqCollector>(
          SimpleCollector::shared_from_this());
    }
  };

  /** checks that no scores are infinite */
private:
  void assertSaneScoring(
      std::shared_ptr<PhraseQuery> pq,
      std::shared_ptr<IndexSearcher> searcher) ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestSloppyPhraseQuery> outerInstance;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestSloppyPhraseQuery> outerInstance);

    std::shared_ptr<Scorer> scorer;

    void setScorer(std::shared_ptr<Scorer> scorer) override;

    void collect(int doc)  override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

  // LUCENE-3215
public:
  virtual void testSlopWithHoles() ;

  // LUCENE-3215
  virtual void testInfiniteFreq1() ;

  // LUCENE-3215
  virtual void testInfiniteFreq2() ;

protected:
  std::shared_ptr<TestSloppyPhraseQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestSloppyPhraseQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
