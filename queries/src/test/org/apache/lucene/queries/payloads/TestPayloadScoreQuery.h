#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spans
{
class SpanQuery;
}

namespace org::apache::lucene::queries::payloads
{
class PayloadFunction;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::queries::payloads
{
class JustScorePayloadSimilarity;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class TermStatistics;
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
namespace org::apache::lucene::queries::payloads
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPayloadScoreQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestPayloadScoreQuery)

private:
  static void checkQuery(std::shared_ptr<SpanQuery> query,
                         std::shared_ptr<PayloadFunction> function,
                         std::deque<int> &expectedDocs,
                         std::deque<float> &expectedScores) ;

  static void checkQuery(std::shared_ptr<SpanQuery> query,
                         std::shared_ptr<PayloadFunction> function,
                         bool includeSpanScore, std::deque<int> &expectedDocs,
                         std::deque<float> &expectedScores) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTermQuery() throws java.io.IOException
  virtual void testTermQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOrQuery() throws java.io.IOException
  virtual void testOrQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNearQuery() throws java.io.IOException
  virtual void testNearQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNestedNearQuery() throws Exception
  virtual void testNestedNearQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSpanContainingQuery() throws Exception
  virtual void testSpanContainingQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEquality()
  virtual void testEquality();

  virtual void testRewrite() ;

private:
  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<Directory> directory;
  static std::shared_ptr<JustScorePayloadSimilarity> similarity;
  static std::deque<char> payload2;
  static std::deque<char> payload4;

private:
  class PayloadAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(PayloadAnalyzer)
  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<PayloadAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<PayloadAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class PayloadFilter : public TokenFilter
  {
    GET_CLASS_NAME(PayloadFilter)

  private:
    int numSeen = 0;
    const std::shared_ptr<PayloadAttribute> payAtt;

  public:
    PayloadFilter(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override;

    void reset()  override;

  protected:
    std::shared_ptr<PayloadFilter> shared_from_this()
    {
      return std::static_pointer_cast<PayloadFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

public:
  class JustScorePayloadSimilarity : public ClassicSimilarity
  {
    GET_CLASS_NAME(JustScorePayloadSimilarity)

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Make everything else 1 so we see the effect of the payload
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  public:
    float lengthNorm(int length) override;

    float sloppyFreq(int distance) override;

    float tf(float freq) override;

    // idf used for phrase queries
    std::shared_ptr<Explanation> idfExplain(
        std::shared_ptr<CollectionStatistics> collectionStats,
        std::deque<std::shared_ptr<TermStatistics>> &termStats) override;

    std::shared_ptr<Explanation>
    idfExplain(std::shared_ptr<CollectionStatistics> collectionStats,
               std::shared_ptr<TermStatistics> termStats) override;

  protected:
    std::shared_ptr<JustScorePayloadSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<JustScorePayloadSimilarity>(
          org.apache.lucene.search.similarities
              .ClassicSimilarity::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPayloadScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestPayloadScoreQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::payloads
