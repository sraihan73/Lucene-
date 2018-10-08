#pragma once
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"

#include  "core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"

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
namespace org::apache::lucene::codecs::lucene70
{

using Codec = org::apache::lucene::codecs::Codec;
using Lucene70DocValuesFormat =
    org::apache::lucene::codecs::lucene70::Lucene70DocValuesFormat;
using BaseCompressingDocValuesFormatTestCase =
    org::apache::lucene::index::BaseCompressingDocValuesFormatTestCase;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using TestUtil = org::apache::lucene::util::TestUtil;

/**
 * Tests Lucene70DocValuesFormat
 */
class TestLucene70DocValuesFormat
    : public BaseCompressingDocValuesFormatTestCase
{
  GET_CLASS_NAME(TestLucene70DocValuesFormat)
private:
  const std::shared_ptr<Codec> codec = TestUtil::alwaysDocValuesFormat(
      std::make_shared<Lucene70DocValuesFormat>());

protected:
  std::shared_ptr<Codec> getCodec() override;

  // TODO: these big methods can easily blow up some of the other ram-hungry
  // codecs... for now just keep them here, as we want to test this for this
  // format.

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void
  // testSortedSetVariableLengthBigVsStoredFields() throws Exception
  virtual void
  testSortedSetVariableLengthBigVsStoredFields() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void
  // testSortedSetVariableLengthManyVsStoredFields() throws Exception
  virtual void
  testSortedSetVariableLengthManyVsStoredFields() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void
  // testSortedVariableLengthBigVsStoredFields() throws Exception
  virtual void
  testSortedVariableLengthBigVsStoredFields() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void
  // testSortedVariableLengthManyVsStoredFields() throws Exception
  virtual void
  testSortedVariableLengthManyVsStoredFields() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testTermsEnumFixedWidth() throws Exception
  virtual void testTermsEnumFixedWidth() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testTermsEnumVariableWidth() throws
  // Exception
  virtual void testTermsEnumVariableWidth() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testTermsEnumRandomMany() throws
  // Exception
  virtual void testTermsEnumRandomMany() ;

  virtual void testTermsEnumLongSharedPrefixes() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testSparseDocValuesVsStoredFields() throws
  // Exception
  virtual void testSparseDocValuesVsStoredFields() ;

private:
  void doTestSparseDocValuesVsStoredFields() ;

  // TODO: try to refactor this and some termsenum tests into the base class.
  // to do this we need to fix the test class to get a DVF not a Codec so we can
  // setup the postings format correctly.
  void doTestTermsEnumRandom(int numDocs,
                             std::function<std::wstring *()>
                                 &valuesProducer) ;

private:
  class AssertingCodecAnonymousInnerClass : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass)
  private:
    std::shared_ptr<TestLucene70DocValuesFormat> outerInstance;

    std::shared_ptr<PostingsFormat> pf;
    std::shared_ptr<DocValuesFormat> dv;

  public:
    AssertingCodecAnonymousInnerClass(
        std::shared_ptr<TestLucene70DocValuesFormat> outerInstance,
        std::shared_ptr<PostingsFormat> pf,
        std::shared_ptr<DocValuesFormat> dv);

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

    std::shared_ptr<DocValuesFormat>
    getDocValuesFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

private:
  void
  assertEquals(int64_t numOrds, std::shared_ptr<TermsEnum> expected,
               std::shared_ptr<TermsEnum> actual) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testSortedSetAroundBlockSize() throws
  // java.io.IOException
  virtual void testSortedSetAroundBlockSize() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testSortedNumericAroundBlockSize() throws
  // java.io.IOException
  virtual void testSortedNumericAroundBlockSize() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void
  // testSortedNumericBlocksOfVariousBitsPerValue() throws Exception
  virtual void
  testSortedNumericBlocksOfVariousBitsPerValue() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void
  // testSparseSortedNumericBlocksOfVariousBitsPerValue() throws Exception
  virtual void testSparseSortedNumericBlocksOfVariousBitsPerValue() throw(
      std::runtime_error);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testNumericBlocksOfVariousBitsPerValue()
  // throws Exception
  virtual void
  testNumericBlocksOfVariousBitsPerValue() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void
  // testSparseNumericBlocksOfVariousBitsPerValue() throws Exception
  virtual void
  testSparseNumericBlocksOfVariousBitsPerValue() ;

private:
  std::shared_ptr<static> std::function < int64_t();

private:
  class functionAnonymousInnerClass : public std::function<int64_t()>
  {
    GET_CLASS_NAME(functionAnonymousInnerClass)
  private:
    int64_t mul = 0;
    int64_t min = 0;

  public:
    functionAnonymousInnerClass(int64_t mul, int64_t min);

    int i = Lucene70DocValuesFormat::NUMERIC_BLOCK_SIZE;
    int maxDelta = 0;
    int64_t getAsLong() override;

  protected:
    std::shared_ptr<functionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<functionAnonymousInnerClass>(
          std::function<long()>::shared_from_this());
    }
  };

private:
  void doTestSortedNumericBlocksOfVariousBitsPerValue(
      std::function<int64_t()> &counts) ;

  void doTestSparseNumericBlocksOfVariousBitsPerValue(double density) throw(
      std::runtime_error);

protected:
  std::shared_ptr<TestLucene70DocValuesFormat> shared_from_this()
  {
    return std::static_pointer_cast<TestLucene70DocValuesFormat>(
        org.apache.lucene.index
            .BaseCompressingDocValuesFormatTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene70/
