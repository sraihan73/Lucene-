#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::queries::payloads
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 *
 *
 **/
class TestPayloadTermQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestPayloadTermQuery)
private:
  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<Similarity> similarity;
  static std::deque<char> const payloadField;
  static std::deque<char> const payloadMultiField1;
  static std::deque<char> const payloadMultiField2;

protected:
  static std::shared_ptr<Directory> directory;

private:
  class PayloadAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(PayloadAnalyzer)

  private:
    PayloadAnalyzer();

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
    const std::wstring fieldName;
    int numSeen = 0;

    const std::shared_ptr<PayloadAttribute> payloadAtt;

  public:
    PayloadFilter(std::shared_ptr<TokenStream> input,
                  const std::wstring &fieldName);

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

  virtual void test() ;

  virtual void testQuery();

  virtual void testMultipleMatchesPerDoc() ;

  virtual void testNoMatch() ;

  virtual void testNoPayload() ;

public:
  class BoostingSimilarity : public ClassicSimilarity
  {
    GET_CLASS_NAME(BoostingSimilarity)

    // TODO: Remove warning after API has been finalized
  public:
    float scorePayload(int docId, int start, int end,
                       std::shared_ptr<BytesRef> payload) override;

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Make everything else 1 so we see the effect of the payload
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    float lengthNorm(int length) override;

    float sloppyFreq(int distance) override;

    float idf(int64_t docFreq, int64_t docCount) override;

    float tf(float freq) override;

  protected:
    std::shared_ptr<BoostingSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<BoostingSimilarity>(
          org.apache.lucene.search.similarities
              .ClassicSimilarity::shared_from_this());
    }
  };

public:
  class FullSimilarity : public ClassicSimilarity
  {
    GET_CLASS_NAME(FullSimilarity)
  public:
    virtual float scorePayload(int docId, const std::wstring &fieldName,
                               std::deque<char> &payload, int offset,
                               int length);

  protected:
    std::shared_ptr<FullSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<FullSimilarity>(
          org.apache.lucene.search.similarities
              .ClassicSimilarity::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPayloadTermQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestPayloadTermQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::payloads
