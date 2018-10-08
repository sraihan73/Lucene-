#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PointValues.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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
namespace org::apache::lucene::util::bkd
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestBKD : public LuceneTestCase
{
  GET_CLASS_NAME(TestBKD)

public:
  virtual void testBasicInts1D() ;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    int queryMin = 0;
    int queryMax = 0;
    std::shared_ptr<BitSet> hits;

  public:
    IntersectVisitorAnonymousInnerClass(std::shared_ptr<TestBKD> outerInstance,
                                        int queryMin, int queryMax,
                                        std::shared_ptr<BitSet> hits);

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
  };

public:
  virtual void testRandomIntsNDims() ;

private:
  class IntersectVisitorAnonymousInnerClass2
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass2>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    int numDims = 0;
    std::deque<int> queryMin;
    std::deque<int> queryMax;
    std::shared_ptr<BitSet> hits;

  public:
    IntersectVisitorAnonymousInnerClass2(std::shared_ptr<TestBKD> outerInstance,
                                         int numDims,
                                         std::deque<int> &queryMin,
                                         std::deque<int> &queryMax,
                                         std::shared_ptr<BitSet> hits);

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
  };

  // Tests on N-dimensional points where each dimension is a int64_t
public:
  virtual void testBigIntNDims() ;

private:
  class IntersectVisitorAnonymousInnerClass3
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass3>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass3)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    int numBytesPerDim = 0;
    int numDims = 0;
    std::deque<std::shared_ptr<int64_t>> queryMin;
    std::deque<std::shared_ptr<int64_t>> queryMax;
    std::shared_ptr<BitSet> hits;

  public:
    IntersectVisitorAnonymousInnerClass3(
        std::shared_ptr<TestBKD> outerInstance, int numBytesPerDim, int numDims,
        std::deque<std::shared_ptr<int64_t>> &queryMin,
        std::deque<std::shared_ptr<int64_t>> &queryMax,
        std::shared_ptr<BitSet> hits);

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
  };

  /** Make sure we close open files, delete temp files, etc., on exception */
public:
  virtual void testWithExceptions() ;

  virtual void testRandomBinaryTiny() ;

  virtual void testRandomBinaryMedium() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testRandomBinaryBig() throws Exception
  virtual void testRandomBinaryBig() ;

  virtual void testTooLittleHeap() ;

private:
  void doTestRandomBinary(int count) ;

public:
  virtual void testAllEqual() ;

  virtual void testOneDimEqual() ;

  // This triggers the logic that makes sure all dimensions get indexed
  // by looking at how many times each dim has been split
  virtual void testOneDimLowCard() ;

  // this should trigger run-length compression with lengths that are greater
  // than 255
  virtual void testOneDimTwoValues() ;

  virtual void testMultiValued() ;

  /** docIDs can be null, for the single valued case, else it maps value to
   * docID */
private:
  void verify(std::deque<std::deque<std::deque<char>>> &docValues,
              std::deque<int> &docIDs, int numDims,
              int numBytesPerDim) ;

  void verify(std::deque<std::deque<std::deque<char>>> &docValues,
              std::deque<int> &docIDs, int numDims, int numBytesPerDim,
              int maxPointsInLeafNode) ;

  void verify(std::shared_ptr<Directory> dir,
              std::deque<std::deque<std::deque<char>>> &docValues,
              std::deque<int> &docIDs, int numDims, int numBytesPerDim,
              int maxPointsInLeafNode, double maxMB) ;

private:
  class DocMapAnonymousInnerClass : public MergeState::DocMap
  {
    GET_CLASS_NAME(DocMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    int docID = 0;
    int curDocIDBase = 0;

  public:
    DocMapAnonymousInnerClass(std::shared_ptr<TestBKD> outerInstance, int docID,
                              int curDocIDBase);

    int get(int docID) override;

  protected:
    std::shared_ptr<DocMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocMapAnonymousInnerClass>(
          org.apache.lucene.index.MergeState.DocMap::shared_from_this());
    }
  };

private:
  class DocMapAnonymousInnerClass2 : public MergeState::DocMap
  {
    GET_CLASS_NAME(DocMapAnonymousInnerClass2)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    int curDocIDBase = 0;

  public:
    DocMapAnonymousInnerClass2(std::shared_ptr<TestBKD> outerInstance,
                               int curDocIDBase);

    int get(int docID) override;

  protected:
    std::shared_ptr<DocMapAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<DocMapAnonymousInnerClass2>(
          org.apache.lucene.index.MergeState.DocMap::shared_from_this());
    }
  };

private:
  class IntersectVisitorAnonymousInnerClass4
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass4>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass4)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    int numDims = 0;
    int numBytesPerDim = 0;
    std::deque<std::deque<char>> queryMin;
    std::deque<std::deque<char>> queryMax;
    std::shared_ptr<BitSet> hits;

  public:
    IntersectVisitorAnonymousInnerClass4(
        std::shared_ptr<TestBKD> outerInstance, int numDims, int numBytesPerDim,
        std::deque<std::deque<char>> &queryMin,
        std::deque<std::deque<char>> &queryMax, std::shared_ptr<BitSet> hits);

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
  };

private:
  std::shared_ptr<int64_t> randomBigInt(int numBytes);

  std::shared_ptr<Directory> getDirectory(int numPoints);

  /** Make sure corruption on an input sort file is caught, even if BKDWriter
   * doesn't get angry */
public:
  virtual void testBitFlippedOnPartition1() ;

private:
  class FilterDirectoryAnonymousInnerClass : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    std::shared_ptr<Directory> dir0;

  public:
    FilterDirectoryAnonymousInnerClass(std::shared_ptr<TestBKD> outerInstance,
                                       std::shared_ptr<Directory> dir0);

    bool corrupted = false;
    std::shared_ptr<IndexOutput> createTempOutput(
        const std::wstring &prefix, const std::wstring &suffix,
        std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

  /** Make sure corruption on a recursed partition is caught, when BKDWriter
   * does get angry */
public:
  virtual void testBitFlippedOnPartition2() ;

private:
  class FilterDirectoryAnonymousInnerClass2 : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass2)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    std::shared_ptr<Directory> dir0;

  public:
    FilterDirectoryAnonymousInnerClass2(std::shared_ptr<TestBKD> outerInstance,
                                        std::shared_ptr<Directory> dir0);

    bool corrupted = false;
    std::shared_ptr<IndexOutput> createTempOutput(
        const std::wstring &prefix, const std::wstring &suffix,
        std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass2>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

private:
  void assertCorruptionDetected(std::runtime_error t);

public:
  virtual void testTieBreakOrder() ;

private:
  class IntersectVisitorAnonymousInnerClass5
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass5>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass5)
  private:
    std::shared_ptr<TestBKD> outerInstance;

  public:
    IntersectVisitorAnonymousInnerClass5(
        std::shared_ptr<TestBKD> outerInstance);

    int lastDocID = 0;

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
  };

public:
  virtual void test2DLongOrdsOffline() ;

private:
  class IntersectVisitorAnonymousInnerClass6
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass6>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass6)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    std::deque<int> count;

  public:
    IntersectVisitorAnonymousInnerClass6(std::shared_ptr<TestBKD> outerInstance,
                                         std::deque<int> &count);

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
  };

  // Claims 16 bytes per dim, but only use the bottom N 1-3 bytes; this would
  // happen e.g. if a user indexes what are actually just short values as a
  // LongPoint:
public:
  virtual void testWastedLeadingBytes() ;

private:
  class IntersectVisitorAnonymousInnerClass7
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass7>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass7)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    std::deque<int> count;

  public:
    IntersectVisitorAnonymousInnerClass7(std::shared_ptr<TestBKD> outerInstance,
                                         std::deque<int> &count);

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
  };

public:
  virtual void testEstimatePointCount() ;

private:
  class IntersectVisitorAnonymousInnerClass8
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass8>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass8)
  private:
    std::shared_ptr<TestBKD> outerInstance;

  public:
    IntersectVisitorAnonymousInnerClass8(
        std::shared_ptr<TestBKD> outerInstance);

    void visit(int docID,
               std::deque<char> &packedValue)  override;
    void visit(int docID)  override;
    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue);
  };

private:
  class IntersectVisitorAnonymousInnerClass9
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass9>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass9)
  private:
    std::shared_ptr<TestBKD> outerInstance;

  public:
    IntersectVisitorAnonymousInnerClass9(
        std::shared_ptr<TestBKD> outerInstance);

    void visit(int docID,
               std::deque<char> &packedValue)  override;
    void visit(int docID)  override;
    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue);
  };

private:
  class IntersectVisitorAnonymousInnerClass10
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass10>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass10)
  private:
    std::shared_ptr<TestBKD> outerInstance;

    int numBytesPerDim = 0;
    std::deque<char> uniquePointValue;

  public:
    IntersectVisitorAnonymousInnerClass10(
        std::shared_ptr<TestBKD> outerInstance, int numBytesPerDim,
        std::deque<char> &uniquePointValue);

    void visit(int docID,
               std::deque<char> &packedValue)  override;
    void visit(int docID)  override;
    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue);
  };

protected:
  std::shared_ptr<TestBKD> shared_from_this()
  {
    return std::static_pointer_cast<TestBKD>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/bkd/
