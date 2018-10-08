#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"
#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

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
namespace org::apache::lucene::queries::function
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleFieldSource =
    org::apache::lucene::queries::function::valuesource::DoubleFieldSource;
using FloatFieldSource =
    org::apache::lucene::queries::function::valuesource::FloatFieldSource;
using IntFieldSource =
    org::apache::lucene::queries::function::valuesource::IntFieldSource;
using LongFieldSource =
    org::apache::lucene::queries::function::valuesource::LongFieldSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// TODO: add separate docvalues test
/**
 * barebones tests for function queries.
 */
class TestValueSources : public LuceneTestCase
{
  GET_CLASS_NAME(TestValueSources)
public:
  static std::shared_ptr<Directory> dir;
  static std::shared_ptr<Analyzer> analyzer;
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<IndexSearcher> searcher;

  static const std::shared_ptr<ValueSource> BOGUS_FLOAT_VS;
  static const std::shared_ptr<ValueSource> BOGUS_DOUBLE_VS;
  static const std::shared_ptr<ValueSource> BOGUS_INT_VS;
  static const std::shared_ptr<ValueSource> BOGUS_LONG_VS;

  static const std::deque<std::deque<std::wstring>> documents;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void testConst() ;

  virtual void testDiv() ;

  virtual void testDocFreq() ;

  virtual void testDoubleConst() ;

  virtual void testDouble() ;

  virtual void testDoubleMultiValued() ;

  virtual void testFloat() ;

  virtual void testFloatMultiValued() ;

  virtual void testIDF() ;

  virtual void testIf() ;

  virtual void testInt() ;

  virtual void testIntMultiValued() ;

  virtual void testJoinDocFreq() ;

  virtual void testLinearFloat() ;

  virtual void testLong() ;

  virtual void testLongMultiValued() ;

  virtual void testMaxDoc() ;

  virtual void testMaxFloat() ;

  virtual void testMinFloat() ;

  virtual void testNorm() ;

  virtual void testNumDocs() ;

  virtual void testPow() ;

  virtual void testProduct() ;

  virtual void testQueryWrapedFuncWrapedQuery() ;

  virtual void testQuery() ;

private:
  class MultiFloatFunctionAnonymousInnerClass : public MultiFloatFunction
  {
    GET_CLASS_NAME(MultiFloatFunctionAnonymousInnerClass)
  private:
    std::shared_ptr<TestValueSources> outerInstance;

  public:
    MultiFloatFunctionAnonymousInnerClass(
        std::shared_ptr<TestValueSources> outerInstance,
        std::deque<std::shared_ptr<ValueSource>> &org);

  protected:
    std::wstring name() override;
    float func(int doc, std::deque<std::shared_ptr<FunctionValues>>
                            &valsArr)  override;
    bool exists(int doc, std::deque<std::shared_ptr<FunctionValues>>
                             &valsArr)  override;

  protected:
    std::shared_ptr<MultiFloatFunctionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<MultiFloatFunctionAnonymousInnerClass>(
          org.apache.lucene.queries.function.valuesource
              .MultiFloatFunction::shared_from_this());
    }
  };

public:
  virtual void testRangeMap() ;

  virtual void testReciprocal() ;

  virtual void testScale() ;

  virtual void testSumFloat() ;

  virtual void testSumTotalTermFreq() ;

  virtual void testTermFreq() ;

  virtual void testTF() ;

  virtual void testTotalTermFreq() ;

  virtual void testMultiFunctionHelperEquivilence() ;

  /**
   * Asserts that for every doc, the {@link FunctionValues#exists} value
   * from the {@link ValueSource} is <b>true</b>.
   */
  virtual void assertAllExist(std::shared_ptr<ValueSource> vs);
  /**
   * Asserts that for every doc, the {@link FunctionValues#exists} value
   * from the {@link ValueSource} is <b>false</b>.
   */
  virtual void assertNoneExist(std::shared_ptr<ValueSource> vs);
  /**
   * Asserts that for every doc, the {@link FunctionValues#exists} value from
   * the <code>actual</code> {@link ValueSource} matches the {@link
   * FunctionValues#exists} value from the <code>expected</code> {@link
   * ValueSource}
   */
  virtual void assertExists(std::shared_ptr<ValueSource> expected,
                            std::shared_ptr<ValueSource> actual);

  virtual void assertHits(std::shared_ptr<Query> q,
                          std::deque<float> &scores) ;

  /**
   * Simple test value source that implements {@link FunctionValues#exists} as a
   * constant
   * @see #ALL_EXIST_VS
   * @see #NONE_EXIST_VS
   */
private:
  class ExistsValueSource final : public ValueSource
  {
    GET_CLASS_NAME(ExistsValueSource)

  public:
    const bool expected;
    const int value;

    ExistsValueSource(bool expected);

    virtual bool equals(std::any o);

    virtual int hashCode();

    std::wstring description() override;

    std::shared_ptr<FunctionValues>
    getValues(std::unordered_map context,
              std::shared_ptr<LeafReaderContext> readerContext) override;

  private:
    class FloatDocValuesAnonymousInnerClass : public FloatDocValues
    {
      GET_CLASS_NAME(FloatDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<ExistsValueSource> outerInstance;

    public:
      FloatDocValuesAnonymousInnerClass(
          std::shared_ptr<ExistsValueSource> outerInstance);

      float floatVal(int doc) override;
      bool exists(int doc) override;

    protected:
      std::shared_ptr<FloatDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FloatDocValuesAnonymousInnerClass>(
            org.apache.lucene.queries.function.docvalues
                .FloatDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<ExistsValueSource> shared_from_this()
    {
      return std::static_pointer_cast<ExistsValueSource>(
          ValueSource::shared_from_this());
    }
  };

  /** @see ExistsValueSource */
private:
  static const std::shared_ptr<ValueSource> ALL_EXIST_VS;
  /** @see ExistsValueSource */
  static const std::shared_ptr<ValueSource> NONE_EXIST_VS;

protected:
  std::shared_ptr<TestValueSources> shared_from_this()
  {
    return std::static_pointer_cast<TestValueSources>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/
