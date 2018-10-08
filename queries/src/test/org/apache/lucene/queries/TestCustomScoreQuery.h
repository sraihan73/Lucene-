#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::queries::function
{
class FunctionQuery;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::queries
{
class CustomScoreProvider;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::queries::function
{
class ValueSource;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class TopDocs;
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
namespace org::apache::lucene::queries
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionQuery = org::apache::lucene::queries::function::FunctionQuery;
using FunctionTestSetup =
    org::apache::lucene::queries::function::FunctionTestSetup;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TopDocs = org::apache::lucene::search::TopDocs;

/**
 * Test CustomScoreQuery search.
 */
class TestCustomScoreQuery : public FunctionTestSetup
{
  GET_CLASS_NAME(TestCustomScoreQuery)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  /**
   * Test that CustomScoreQuery of Type.INT returns the expected scores.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCustomScoreInt() throws Exception
  virtual void testCustomScoreInt() ;

  /**
   * Test that CustomScoreQuery of Type.FLOAT returns the expected scores.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCustomScoreFloat() throws Exception
  virtual void testCustomScoreFloat() ;

  // must have static class otherwise serialization tests fail
private:
  class CustomAddQuery : public CustomScoreQuery
  {
    GET_CLASS_NAME(CustomAddQuery)
    // constructor
  public:
    CustomAddQuery(std::shared_ptr<Query> q,
                   std::shared_ptr<FunctionQuery> qValSrc);

    /*(non-Javadoc) @see
     * org.apache.lucene.search.function.CustomScoreQuery#name() */
    std::wstring name() override;

  protected:
    std::shared_ptr<CustomScoreProvider>
    getCustomScoreProvider(std::shared_ptr<LeafReaderContext> context) override;

  private:
    class CustomScoreProviderAnonymousInnerClass : public CustomScoreProvider
    {
      GET_CLASS_NAME(CustomScoreProviderAnonymousInnerClass)
    private:
      std::shared_ptr<CustomAddQuery> outerInstance;

    public:
      CustomScoreProviderAnonymousInnerClass(
          std::shared_ptr<CustomAddQuery> outerInstance,
          std::shared_ptr<LeafReaderContext> context);

      float customScore(int doc, float subQueryScore,
                        float valSrcScore) override;

      std::shared_ptr<Explanation>
      customExplain(int doc, std::shared_ptr<Explanation> subQueryExpl,
                    std::shared_ptr<Explanation> valSrcExpl) override;

    protected:
      std::shared_ptr<CustomScoreProviderAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<CustomScoreProviderAnonymousInnerClass>(
            CustomScoreProvider::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<CustomAddQuery> shared_from_this()
    {
      return std::static_pointer_cast<CustomAddQuery>(
          CustomScoreQuery::shared_from_this());
    }
  };

  // must have static class otherwise serialization tests fail
private:
  class CustomMulAddQuery : public CustomScoreQuery
  {
    GET_CLASS_NAME(CustomMulAddQuery)
    // constructor
  public:
    CustomMulAddQuery(std::shared_ptr<Query> q,
                      std::shared_ptr<FunctionQuery> qValSrc1,
                      std::shared_ptr<FunctionQuery> qValSrc2);

    /*(non-Javadoc) @see
     * org.apache.lucene.search.function.CustomScoreQuery#name() */
    std::wstring name() override;

  protected:
    std::shared_ptr<CustomScoreProvider>
    getCustomScoreProvider(std::shared_ptr<LeafReaderContext> context) override;

  private:
    class CustomScoreProviderAnonymousInnerClass : public CustomScoreProvider
    {
      GET_CLASS_NAME(CustomScoreProviderAnonymousInnerClass)
    private:
      std::shared_ptr<CustomMulAddQuery> outerInstance;

    public:
      CustomScoreProviderAnonymousInnerClass(
          std::shared_ptr<CustomMulAddQuery> outerInstance,
          std::shared_ptr<LeafReaderContext> context);

      float customScore(int doc, float subQueryScore,
                        std::deque<float> &valSrcScores) override;

      std::shared_ptr<Explanation> customExplain(
          int doc, std::shared_ptr<Explanation> subQueryExpl,
          std::deque<std::shared_ptr<Explanation>> &valSrcExpls) override;

    protected:
      std::shared_ptr<CustomScoreProviderAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<CustomScoreProviderAnonymousInnerClass>(
            CustomScoreProvider::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<CustomMulAddQuery> shared_from_this()
    {
      return std::static_pointer_cast<CustomMulAddQuery>(
          CustomScoreQuery::shared_from_this());
    }
  };

private:
  class CustomExternalQuery final : public CustomScoreQuery
  {
    GET_CLASS_NAME(CustomExternalQuery)

  protected:
    std::shared_ptr<CustomScoreProvider> getCustomScoreProvider(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class CustomScoreProviderAnonymousInnerClass : public CustomScoreProvider
    {
      GET_CLASS_NAME(CustomScoreProviderAnonymousInnerClass)
    private:
      std::shared_ptr<CustomExternalQuery> outerInstance;

      std::shared_ptr<LeafReaderContext> context;
      std::shared_ptr<NumericDocValues> values;

    public:
      CustomScoreProviderAnonymousInnerClass(
          std::shared_ptr<CustomExternalQuery> outerInstance,
          std::shared_ptr<LeafReaderContext> context,
          std::shared_ptr<NumericDocValues> values);

      float customScore(int doc, float subScore,
                        float valSrcScore)  override;

    protected:
      std::shared_ptr<CustomScoreProviderAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<CustomScoreProviderAnonymousInnerClass>(
            CustomScoreProvider::shared_from_this());
      }
    };

  public:
    CustomExternalQuery(std::shared_ptr<Query> q);

  protected:
    std::shared_ptr<CustomExternalQuery> shared_from_this()
    {
      return std::static_pointer_cast<CustomExternalQuery>(
          CustomScoreQuery::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCustomExternalQuery() throws Exception
  virtual void testCustomExternalQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRewrite() throws Exception
  virtual void testRewrite() ;

  // Test that FieldScoreQuery returns docs with expected score.
private:
  void doTestCustomScore(std::shared_ptr<ValueSource> valueSource,
                         double dboost) ;

  // verify results are as expected.
  void verifyResults(float boost, std::shared_ptr<IndexSearcher> s,
                     std::unordered_map<int, float> &h1,
                     std::unordered_map<int, float> &h2customNeutral,
                     std::unordered_map<int, float> &h3CustomMul,
                     std::unordered_map<int, float> &h4CustomAdd,
                     std::unordered_map<int, float> &h5CustomMulAdd,
                     std::shared_ptr<Query> q1, std::shared_ptr<Query> q2,
                     std::shared_ptr<Query> q3, std::shared_ptr<Query> q4,
                     std::shared_ptr<Query> q5) ;

  void logResult(const std::wstring &msg, std::shared_ptr<IndexSearcher> s,
                 std::shared_ptr<Query> q, int doc,
                 float score1) ;

  // since custom scoring modifies the order of docs, map_obj results
  // by doc ids so that we can later compare/verify them
  std::unordered_map<int, float> topDocsToMap(std::shared_ptr<TopDocs> td);

protected:
  std::shared_ptr<TestCustomScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestCustomScoreQuery>(
        org.apache.lucene.queries.function
            .FunctionTestSetup::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries
