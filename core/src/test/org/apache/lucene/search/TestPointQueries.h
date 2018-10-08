#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsWriter.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
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
namespace org::apache::lucene::search
{

using Codec = org::apache::lucene::codecs::Codec;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPointQueries : public LuceneTestCase
{
  GET_CLASS_NAME(TestPointQueries)

  // Controls what range of values we randomly generate, so we sometimes test
  // narrow ranges:
public:
  static int64_t valueMid;
  static int valueRange;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass()
  static void beforeClass();

  virtual void testBasicInts() ;

  virtual void testBasicFloats() ;

  virtual void testBasicLongs() ;

  virtual void testBasicDoubles() ;

  virtual void testCrazyDoubles() ;

  virtual void testCrazyFloats() ;

  virtual void testAllEqual() ;

  virtual void testRandomLongsTiny() ;

  virtual void testRandomLongsMedium() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testRandomLongsBig() throws Exception
  virtual void testRandomLongsBig() ;

private:
  void doTestRandomLongs(int count) ;

public:
  virtual void testLongEncode();

  // verify for long values
private:
  static void verifyLongs(std::deque<int64_t> &values,
                          std::deque<int> &ids) ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::deque<int64_t> values;
    std::shared_ptr<BitSet> missing;
    std::shared_ptr<BitSet> deleted;
    std::shared_ptr<Document> doc;
    std::shared_ptr<IndexReader> r;
    std::shared_ptr<org::apache::lucene::search::IndexSearcher> s;
    int iters = 0;
    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<AtomicBoolean> failed;

  public:
    ThreadAnonymousInnerClass(
        std::deque<int64_t> &values, std::shared_ptr<BitSet> missing,
        std::shared_ptr<BitSet> deleted, std::shared_ptr<Document> doc,
        std::shared_ptr<IndexReader> r,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> s,
        int iters, std::shared_ptr<CountDownLatch> startingGun,
        std::shared_ptr<AtomicBoolean> failed);

    void run() override;

  private:
    void _run() ;

  private:
    class SimpleCollectorAnonymousInnerClass : public SimpleCollector
    {
      GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<ThreadAnonymousInnerClass> outerInstance;

      std::shared_ptr<BitSet> hits;

    public:
      SimpleCollectorAnonymousInnerClass(
          std::shared_ptr<ThreadAnonymousInnerClass> outerInstance,
          std::shared_ptr<BitSet> hits);

    private:
      int docBase = 0;

    public:
      bool needsScores() override;

    protected:
      void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    public:
      void collect(int doc) override;

    protected:
      std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
            SimpleCollector::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testRandomBinaryTiny() ;

  virtual void testRandomBinaryMedium() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testRandomBinaryBig() throws Exception
  virtual void testRandomBinaryBig() ;

private:
  void doTestRandomBinary(int count) ;

  // verify for byte[][] values
  void verifyBinary(std::deque<std::deque<std::deque<char>>> &docValues,
                    std::deque<int> &ids,
                    int numBytesPerDim) ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestPointQueries> outerInstance;

    std::deque<std::deque<std::deque<char>>> docValues;
    std::deque<int> ids;
    int numDims = 0;
    int bytesPerDim = 0;
    int numValues = 0;
    std::shared_ptr<BitSet> missing;
    std::shared_ptr<BitSet> deleted;
    std::shared_ptr<Document> doc;
    std::shared_ptr<IndexReader> r;
    std::shared_ptr<org::apache::lucene::search::IndexSearcher> s;
    int iters = 0;
    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<AtomicBoolean> failed;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestPointQueries> outerInstance,
        std::deque<std::deque<std::deque<char>>> &docValues,
        std::deque<int> &ids, int numDims, int bytesPerDim, int numValues,
        std::shared_ptr<BitSet> missing, std::shared_ptr<BitSet> deleted,
        std::shared_ptr<Document> doc, std::shared_ptr<IndexReader> r,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> s,
        int iters, std::shared_ptr<CountDownLatch> startingGun,
        std::shared_ptr<AtomicBoolean> failed);

    void run() override;

  private:
    void _run() ;

  private:
    class SimpleCollectorAnonymousInnerClass2 : public SimpleCollector
    {
      GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass2)
    private:
      std::shared_ptr<ThreadAnonymousInnerClass2> outerInstance;

      std::shared_ptr<BitSet> hits;

    public:
      SimpleCollectorAnonymousInnerClass2(
          std::shared_ptr<ThreadAnonymousInnerClass2> outerInstance,
          std::shared_ptr<BitSet> hits);

    private:
      int docBase = 0;

    public:
      bool needsScores() override;

    protected:
      void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    public:
      void collect(int doc) override;

    protected:
      std::shared_ptr<SimpleCollectorAnonymousInnerClass2> shared_from_this()
      {
        return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass2>(
            SimpleCollector::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

public:
  static std::wstring bytesToString(std::deque<char> &bytes);

private:
  static bool matches(int bytesPerDim, std::deque<std::deque<char>> &lower,
                      std::deque<std::deque<char>> &upper,
                      std::deque<std::deque<char>> &value);

  static int64_t randomValue();

public:
  virtual void testMinMaxLong() ;

private:
  static std::deque<char> toUTF8(const std::wstring &s);

  // Right zero pads:
  static std::deque<char> toUTF8(const std::wstring &s, int length);

public:
  virtual void testBasicSortedSet() ;

  virtual void testLongMinMaxNumeric() ;

  virtual void testLongMinMaxSortedSet() ;

  virtual void testSortedSetNoOrdsMatch() ;

  virtual void testNumericNoValuesMatch() ;

  virtual void testNoDocs() ;

  virtual void testWrongNumDims() ;

  virtual void testWrongNumBytes() ;

  virtual void
  testAllPointDocsWereDeletedAndThenMergedAgain() ;

private:
  static std::shared_ptr<Codec> getCodec();

private:
  class FilterCodecAnonymousInnerClass : public FilterCodec
  {
    GET_CLASS_NAME(FilterCodecAnonymousInnerClass)
  private:
    int maxPointsInLeafNode = 0;
    double maxMBSortInHeap = 0;

  public:
    FilterCodecAnonymousInnerClass(std::shared_ptr<Codec> getDefault,
                                   int maxPointsInLeafNode,
                                   double maxMBSortInHeap);

    std::shared_ptr<PointsFormat> pointsFormat() override;

  private:
    class PointsFormatAnonymousInnerClass : public PointsFormat
    {
      GET_CLASS_NAME(PointsFormatAnonymousInnerClass)
    private:
      std::shared_ptr<FilterCodecAnonymousInnerClass> outerInstance;

    public:
      PointsFormatAnonymousInnerClass(
          std::shared_ptr<FilterCodecAnonymousInnerClass> outerInstance);

      std::shared_ptr<PointsWriter>
      fieldsWriter(std::shared_ptr<SegmentWriteState> writeState) throw(
          IOException) override;

      std::shared_ptr<PointsReader>
      fieldsReader(std::shared_ptr<SegmentReadState> readState) throw(
          IOException) override;

    protected:
      std::shared_ptr<PointsFormatAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PointsFormatAnonymousInnerClass>(
            org.apache.lucene.codecs.PointsFormat::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FilterCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.FilterCodec::shared_from_this());
    }
  };

public:
  virtual void testExactPoints() ;

  virtual void testToString() ;

private:
  std::deque<int> toArray(std::shared_ptr<Set<int>> valuesSet);

  static int randomIntValue(std::optional<int> &min, std::optional<int> &max);

public:
  virtual void testRandomPointInSetQuery() ;

private:
  class ThreadAnonymousInnerClass3 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass3)
  private:
    std::shared_ptr<TestPointQueries> outerInstance;

    std::deque<int> values;
    std::deque<int> docValues;
    std::shared_ptr<org::apache::lucene::search::IndexSearcher> s;
    int iters = 0;
    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<AtomicBoolean> failed;

  public:
    ThreadAnonymousInnerClass3(
        std::shared_ptr<TestPointQueries> outerInstance,
        std::deque<int> &values, std::deque<int> &docValues,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> s,
        int iters, std::shared_ptr<CountDownLatch> startingGun,
        std::shared_ptr<AtomicBoolean> failed);

    void run() override;

  private:
    void _run() ;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass3>(
          Thread::shared_from_this());
    }
  };

  // TODO: in the future, if there is demand for real usage, we can "graduate"
  // this test-only query factory as IntPoint.newMultiSetQuery or something (and
  // same for other XXXPoint classes):
private:
  static std::shared_ptr<Query>
  newMultiDimIntSetQuery(const std::wstring &field, int const numDims,
                         std::deque<int> &valuesIn) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::deque<char>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::deque<char> &a, std::deque<char> &b);
  };

private:
  class StreamAnonymousInnerClass : public PointInSetQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass)
  private:
    std::deque<std::deque<char>> packedValues;
    std::shared_ptr<BytesRef> value;

  public:
    StreamAnonymousInnerClass(std::deque<std::deque<char>> &packedValues,
                              std::shared_ptr<BytesRef> value);

    int upto = 0;
    std::shared_ptr<BytesRef> next() override;

  protected:
    std::shared_ptr<StreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StreamAnonymousInnerClass>(
          PointInSetQuery.Stream::shared_from_this());
    }
  };

public:
  virtual void testBasicMultiDimPointInSetQuery() ;

  virtual void
  testBasicMultiValueMultiDimPointInSetQuery() ;

  virtual void
  testManyEqualValuesMultiDimPointInSetQuery() ;

  virtual void testInvalidMultiDimPointInSetQuery() ;

  virtual void testBasicPointInSetQuery() ;

  /** Boxed methods for primitive types should behave the same as unboxed: just
   * sugar */
  virtual void testPointIntSetBoxed() ;

  virtual void testBasicMultiValuedPointInSetQuery() ;

  virtual void testEmptyPointInSetQuery() ;

  virtual void testPointInSetQueryManyEqualValues() ;

  virtual void
  testPointInSetQueryManyEqualValuesWithBigGap() ;

  virtual void testInvalidPointInSetQuery() ;

public:
                                                        );
                                                        TestUtil::assertEquals(
                                                            L"packed point "
                                                            L"length should be "
                                                            L"12 but got 3; "
                                                            L"field=\"foo\" "
                                                            L"numDims=3 "
                                                            L"bytesPerDim=4",
                                                            expected::
                                                                getMessage());

                                                      protected:
                                                        std::shared_ptr<
                                                            TestPointQueries>
                                                        shared_from_this()
                                                        {
                                                          return std::static_pointer_cast<
                                                              TestPointQueries>(
                                                              org.apache.lucene
                                                                  .util
                                                                  .LuceneTestCase::
                                                                      shared_from_this());
                                                        }
};

public:
virtual void testInvalidPointInSetBinaryQuery() ;

virtual void testPointInSetQueryToString() ;

virtual void testPointInSetQueryGetPackedPoints() ;

virtual void testRangeOptimizesIfAllPointsMatch() ;

virtual void testPointRangeEquals();

virtual void testPointExactEquals();

virtual void testPointInSetEquals();

virtual void testInvalidPointLength();
                                                 );
                                                 TestUtil::assertEquals(
                                                     L"lowerPoint has length=4 "
                                                     L"but upperPoint has "
                                                     L"different length=8",
                                                     e::getMessage());
                                                 } // namespace
                                                   // org::apache::lucene::search

                                                 virtual void testNextUp();

                                                 virtual void testNextDown();

                                                 virtual void
                                                 testInversePointRange() throw(
                                                     IOException);
                                                 }
                                                 }
