#pragma once
#include "stringhelper.h"
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::document
{
class Field;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
using Field = org::apache::lucene::document::Field;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Abstract class to do basic tests for a docvalues format.
 * NOTE: This test focuses on the docvalues impl, nothing else.
 * The [stretch] goal is for this test to be
 * so thorough in testing a new DocValuesFormat that if this
 * test passes, then all Lucene/Solr tests should also pass.  Ie,
 * if there is some bug in a given DocValuesFormat that this
 * test fails to catch then this test needs to be improved! */
class BaseDocValuesFormatTestCase : public BaseIndexFileFormatTestCase
{
  GET_CLASS_NAME(BaseDocValuesFormatTestCase)

protected:
  void addRandomFields(std::shared_ptr<Document> doc) override;

public:
  virtual void testOneNumber() ;

  virtual void testOneFloat() ;

  virtual void testTwoNumbers() ;

  virtual void testTwoBinaryValues() ;

  virtual void testTwoFieldsMixed() ;

  virtual void testThreeFieldsMixed() ;

  virtual void testThreeFieldsMixed2() ;

  virtual void testTwoDocumentsNumeric() ;

  virtual void testTwoDocumentsMerged() ;

  virtual void testBigNumericRange() ;

  virtual void testBigNumericRange2() ;

  virtual void testBytes() ;

  virtual void testBytesTwoDocumentsMerged() ;

  virtual void testBytesMergeAwayAllValues() ;

  virtual void testSortedBytes() ;

  virtual void testSortedBytesTwoDocuments() ;

  virtual void testSortedBytesThreeDocuments() ;

  virtual void testSortedBytesTwoDocumentsMerged() ;

  virtual void testSortedMergeAwayAllValues() ;

  virtual void testBytesWithNewline() ;

  virtual void testMissingSortedBytes() ;

  virtual void testSortedTermsEnum() ;

  virtual void testEmptySortedBytes() ;

  virtual void testEmptyBytes() ;

  virtual void testVeryLargeButLegalBytes() ;

  virtual void testVeryLargeButLegalSortedBytes() ;

  virtual void testCodecUsesOwnBytes() ;

  virtual void testCodecUsesOwnSortedBytes() ;

  /*
   * Simple test case to show how to use the API
   */
  virtual void testDocValuesSimple() ;

  virtual void testRandomSortedBytes() ;

private:
  void doTestNumericsVsStoredFields(
      double density,
      std::function<int64_t()> &longs) ;

  void doTestSortedNumericsVsStoredFields(
      std::function<int64_t()> &counts,
      std::function<int64_t()> &values) ;

public:
  virtual void testBooleanNumericsVsStoredFields() ;

  virtual void
  testSparseBooleanNumericsVsStoredFields() ;

  virtual void testByteNumericsVsStoredFields() ;

  virtual void testSparseByteNumericsVsStoredFields() ;

  virtual void testShortNumericsVsStoredFields() ;

  virtual void
  testSparseShortNumericsVsStoredFields() ;

  virtual void testIntNumericsVsStoredFields() ;

  virtual void testSparseIntNumericsVsStoredFields() ;

  virtual void testLongNumericsVsStoredFields() ;

  virtual void testSparseLongNumericsVsStoredFields() ;

private:
  void doTestBinaryVsStoredFields(
      double density,
      std::function<char[] * ()> &bytes) ;

public:
  virtual void testBinaryFixedLengthVsStoredFields() ;

  virtual void
  testSparseBinaryFixedLengthVsStoredFields() ;

private:
  void doTestBinaryFixedLengthVsStoredFields(double density) throw(
      std::runtime_error);

public:
  virtual void
  testBinaryVariableLengthVsStoredFields() ;

  virtual void
  testSparseBinaryVariableLengthVsStoredFields() ;

  virtual void doTestBinaryVariableLengthVsStoredFields(double density) throw(
      std::runtime_error);

protected:
  virtual void doTestSortedVsStoredFields(
      int numDocs, double density,
      std::function<char[] * ()> &bytes) ;

public:
  virtual void testSortedFixedLengthVsStoredFields() ;

  virtual void
  testSparseSortedFixedLengthVsStoredFields() ;

  virtual void
  testSortedVariableLengthVsStoredFields() ;

  virtual void
  testSparseSortedVariableLengthVsStoredFields() ;

protected:
  virtual void
  doTestSortedVsStoredFields(int numDocs, double density, int minLength,
                             int maxLength) ;

public:
  virtual void testSortedSetOneValue() ;

  virtual void testSortedSetTwoFields() ;

  virtual void testSortedSetTwoDocumentsMerged() ;

  virtual void testSortedSetTwoValues() ;

  virtual void testSortedSetTwoValuesUnordered() ;

  virtual void testSortedSetThreeValuesTwoDocs() ;

  virtual void testSortedSetTwoDocumentsLastMissing() ;

  virtual void testSortedSetTwoDocumentsLastMissingMerge() ;

  virtual void testSortedSetTwoDocumentsFirstMissing() ;

  virtual void testSortedSetTwoDocumentsFirstMissingMerge() ;

  virtual void testSortedSetMergeAwayAllValues() ;

  virtual void testSortedSetTermsEnum() ;

protected:
  virtual void
  doTestSortedSetVsStoredFields(int numDocs, int minLength, int maxLength,
                                int maxValuesPerDoc,
                                int maxUniqueValues) ;

public:
  virtual void
  testSortedSetFixedLengthVsStoredFields() ;

  virtual void
  testSortedNumericsSingleValuedVsStoredFields() ;

  virtual void testSortedNumericsSingleValuedMissingVsStoredFields() throw(
      std::runtime_error);

  virtual void
  testSortedNumericsMultipleValuesVsStoredFields() ;

  virtual void
  testSortedNumericsFewUniqueSetsVsStoredFields() ;

  virtual void
  testSortedSetVariableLengthVsStoredFields() ;

  virtual void testSortedSetFixedLengthSingleValuedVsStoredFields() throw(
      std::runtime_error);

  virtual void testSortedSetVariableLengthSingleValuedVsStoredFields() throw(
      std::runtime_error);

  virtual void testSortedSetFixedLengthFewUniqueSetsVsStoredFields() throw(
      std::runtime_error);

  virtual void testSortedSetVariableLengthFewUniqueSetsVsStoredFields() throw(
      std::runtime_error);

  virtual void
  testSortedSetVariableLengthManyValuesPerDocVsStoredFields() throw(
      std::runtime_error);

  virtual void testSortedSetFixedLengthManyValuesPerDocVsStoredFields() throw(
      std::runtime_error);

  virtual void testGCDCompression() ;

  virtual void testSparseGCDCompression() ;

private:
  void doTestGCDCompression(double density) ;

public:
  virtual void testZeros() ;

  virtual void testSparseZeros() ;

  virtual void testZeroOrMin() ;

  virtual void testTwoNumbersOneMissing() ;

  virtual void testTwoNumbersOneMissingWithMerging() ;

  virtual void testThreeNumbersOneMissingWithMerging() ;

  virtual void testTwoBytesOneMissing() ;

  virtual void testTwoBytesOneMissingWithMerging() ;

  virtual void testThreeBytesOneMissingWithMerging() ;

  /** Tests dv against stored fields with threads (binary/numeric/sorted, no
   * missing) */
  virtual void testThreads() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance;

    std::shared_ptr<org::apache::lucene::index::DirectoryReader> ir;
    std::shared_ptr<CountDownLatch> startingGun;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance,
        std::shared_ptr<org::apache::lucene::index::DirectoryReader> ir,
        std::shared_ptr<CountDownLatch> startingGun);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  /** Tests dv against stored fields with threads (all types + missing) */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testThreads2() throws Exception
  virtual void testThreads2() ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance;

    std::shared_ptr<org::apache::lucene::index::DirectoryReader> ir;
    std::shared_ptr<CountDownLatch> startingGun;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance,
        std::shared_ptr<org::apache::lucene::index::DirectoryReader> ir,
        std::shared_ptr<CountDownLatch> startingGun);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testThreads3() throws Exception
  virtual void testThreads3() ;

private:
  class ThreadAnonymousInnerClass3 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass3)
  private:
    std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance;

    std::shared_ptr<org::apache::lucene::index::DirectoryReader> r;
    std::shared_ptr<CountDownLatch> startingGun;

  public:
    ThreadAnonymousInnerClass3(
        std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance,
        std::shared_ptr<org::apache::lucene::index::DirectoryReader> r,
        std::shared_ptr<CountDownLatch> startingGun);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass3>(
          Thread::shared_from_this());
    }
  };

  // LUCENE-5218
public:
  virtual void testEmptyBinaryValueOnPageSizes() ;

  virtual void testOneSortedNumber() ;

  virtual void testOneSortedNumberOneMissing() ;

  virtual void testNumberMergeAwayAllValues() ;

  virtual void testTwoSortedNumber() ;

  virtual void testTwoSortedNumberSameValue() ;

  virtual void testTwoSortedNumberOneMissing() ;

  virtual void testSortedNumberMerge() ;

  virtual void testSortedNumberMergeAwayAllValues() ;

  virtual void testSortedEnumAdvanceIndependently() ;

  virtual void testSortedSetEnumAdvanceIndependently() ;

protected:
  virtual void doTestSortedSetEnumAdvanceIndependently(
      std::shared_ptr<SortedSetDocValues> dv) ;

  // same as testSortedMergeAwayAllValues but on more than 1024 docs to have
  // sparse encoding on
public:
  virtual void testSortedMergeAwayAllValuesLargeSegment() ;

  // same as testSortedSetMergeAwayAllValues but on more than 1024 docs to have
  // sparse encoding on
  virtual void testSortedSetMergeAwayAllValuesLargeSegment() ;

  // same as testNumericMergeAwayAllValues but on more than 1024 docs to have
  // sparse encoding on
  virtual void testNumericMergeAwayAllValuesLargeSegment() ;

  // same as testSortedNumericMergeAwayAllValues but on more than 1024 docs to
  // have sparse encoding on
  virtual void
  testSortedNumericMergeAwayAllValuesLargeSegment() ;

  // same as testBinaryMergeAwayAllValues but on more than 1024 docs to have
  // sparse encoding on
  virtual void testBinaryMergeAwayAllValuesLargeSegment() ;

  virtual void testRandomAdvanceNumeric() ;

private:
  class FieldCreatorAnonymousInnerClass
      : public std::enable_shared_from_this<FieldCreatorAnonymousInnerClass>,
        public FieldCreator
  {
    GET_CLASS_NAME(FieldCreatorAnonymousInnerClass)
  private:
    std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance;

    int64_t longRange = 0;

  public:
    FieldCreatorAnonymousInnerClass(
        std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance,
        int64_t longRange);

    std::shared_ptr<Field> next() override;

    std::shared_ptr<DocIdSetIterator>
    iterator(std::shared_ptr<IndexReader> r)  override;
  };

public:
  virtual void testRandomAdvanceBinary() ;

private:
  class FieldCreatorAnonymousInnerClass2
      : public std::enable_shared_from_this<FieldCreatorAnonymousInnerClass2>,
        public FieldCreator
  {
    GET_CLASS_NAME(FieldCreatorAnonymousInnerClass2)
  private:
    std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance;

  public:
    FieldCreatorAnonymousInnerClass2(
        std::shared_ptr<BaseDocValuesFormatTestCase> outerInstance);

    std::shared_ptr<Field> next() override;

    std::shared_ptr<DocIdSetIterator>
    iterator(std::shared_ptr<IndexReader> r)  override;
  };

private:
  class FieldCreator
  {
    GET_CLASS_NAME(FieldCreator)
  public:
    virtual std::shared_ptr<Field> next() = 0;
    virtual std::shared_ptr<DocIdSetIterator>
    iterator(std::shared_ptr<IndexReader> r) = 0;
  };

private:
  void doTestRandomAdvance(std::shared_ptr<FieldCreator> fieldCreator) throw(
      IOException);

protected:
  virtual bool codecAcceptsHugeBinaryValues(const std::wstring &field);

protected:
  std::shared_ptr<BaseDocValuesFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseDocValuesFormatTestCase>(
        BaseIndexFileFormatTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
