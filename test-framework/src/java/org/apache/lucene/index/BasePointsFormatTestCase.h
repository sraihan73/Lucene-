#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class RandomIndexWriter;
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
using Directory = org::apache::lucene::store::Directory;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Abstract class to do basic tests for a points format.
 * NOTE: This test focuses on the points impl, nothing else.
 * The [stretch] goal is for this test to be
 * so thorough in testing a new PointsFormat that if this
 * test passes, then all Lucene/Solr tests should also pass.  Ie,
 * if there is some bug in a given PointsFormat that this
 * test fails to catch then this test needs to be improved! */
class BasePointsFormatTestCase : public BaseIndexFileFormatTestCase
{
  GET_CLASS_NAME(BasePointsFormatTestCase)

protected:
  void addRandomFields(std::shared_ptr<Document> doc) override;

public:
  virtual void testBasic() ;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<BasePointsFormatTestCase> outerInstance;

    std::shared_ptr<BitSet> seen;

  public:
    IntersectVisitorAnonymousInnerClass(
        std::shared_ptr<BasePointsFormatTestCase> outerInstance,
        std::shared_ptr<BitSet> seen);

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
    void visit(int docID) override;
    void visit(int docID, std::deque<char> &packedValue) override;
  };

public:
  virtual void testMerge() ;

private:
  class IntersectVisitorAnonymousInnerClass2
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass2>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass2)
  private:
    std::shared_ptr<BasePointsFormatTestCase> outerInstance;

    std::shared_ptr<BitSet> seen;

  public:
    IntersectVisitorAnonymousInnerClass2(
        std::shared_ptr<BasePointsFormatTestCase> outerInstance,
        std::shared_ptr<BitSet> seen);

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
    void visit(int docID) override;
    void visit(int docID, std::deque<char> &packedValue) override;
  };

public:
  virtual void testAllPointDocsDeletedInSegment() ;

private:
  class IntersectVisitorAnonymousInnerClass3
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass3>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass3)
  private:
    std::shared_ptr<BasePointsFormatTestCase> outerInstance;

    std::shared_ptr<Bits> liveDocs;
    std::deque<int> docIDToID;
    int docID = 0;
    std::shared_ptr<BitSet> seen;

  public:
    IntersectVisitorAnonymousInnerClass3(
        std::shared_ptr<BasePointsFormatTestCase> outerInstance,
        std::shared_ptr<Bits> liveDocs, std::deque<int> &docIDToID, int docID,
        std::shared_ptr<BitSet> seen);

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
    void visit(int docID) override;
    void visit(int docID, std::deque<char> &packedValue) override;
  };

  /** Make sure we close open files, delete temp files, etc., on exception */
public:
  virtual void testWithExceptions() ;

  // TODO: merge w/ BaseIndexFileFormatTestCase.handleFakeIOException
private:
  bool handlePossiblyFakeException(std::runtime_error e);

public:
  virtual void testMultiValued() ;

  virtual void testAllEqual() ;

  virtual void testOneDimEqual() ;

  // this should trigger run-length compression with lengths that are greater
  // than 255
  virtual void testOneDimTwoValues() ;

  // Tests on N-dimensional points where each dimension is a int64_t
  virtual void testBigIntNDims() ;

private:
  class IntersectVisitorAnonymousInnerClass4
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass4>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass4)
  private:
    std::shared_ptr<BasePointsFormatTestCase> outerInstance;

    int numBytesPerDim = 0;
    int numDims = 0;
    std::deque<std::shared_ptr<int64_t>> queryMin;
    std::deque<std::shared_ptr<int64_t>> queryMax;
    std::shared_ptr<BitSet> hits;
    int docBase = 0;

  public:
    IntersectVisitorAnonymousInnerClass4(
        std::shared_ptr<BasePointsFormatTestCase> outerInstance,
        int numBytesPerDim, int numDims,
        std::deque<std::shared_ptr<int64_t>> &queryMin,
        std::deque<std::shared_ptr<int64_t>> &queryMax,
        std::shared_ptr<BitSet> hits, int docBase);

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
  };

public:
  virtual void testRandomBinaryTiny() ;

  virtual void testRandomBinaryMedium() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testRandomBinaryBig() throws Exception
  virtual void testRandomBinaryBig() ;

private:
  void doTestRandomBinary(int count) ;

  /** docIDs can be null, for the single valued case, else it maps value to
   * docID, but all values for one doc must be adjacent */
  void verify(std::deque<std::deque<std::deque<char>>> &docValues,
              std::deque<int> &docIDs, int numDims,
              int numBytesPerDim) ;

  void verify(std::shared_ptr<Directory> dir,
              std::deque<std::deque<std::deque<char>>> &docValues,
              std::deque<int> &ids, int numDims, int numBytesPerDim,
              bool expectExceptions) ;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<BasePointsFormatTestCase> outerInstance;

    int numDims = 0;
    int numBytesPerDim = 0;
    std::deque<int> docIDToID;
    std::shared_ptr<Bits> liveDocs;
    std::deque<std::deque<char>> queryMin;
    std::deque<std::deque<char>> queryMax;
    std::shared_ptr<BitSet> hits;
    int docBase = 0;

  public:
    IntersectVisitorAnonymousInnerClass(
        std::shared_ptr<BasePointsFormatTestCase> outerInstance, int numDims,
        int numBytesPerDim, std::deque<int> &docIDToID,
        std::shared_ptr<Bits> liveDocs,
        std::deque<std::deque<char>> &queryMin,
        std::deque<std::deque<char>> &queryMax, std::shared_ptr<BitSet> hits,
        int docBase);

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPacked,
                     std::deque<char> &maxPacked);
  };

public:
  virtual void testAddIndexes() ;

private:
  void switchIndex(std::shared_ptr<RandomIndexWriter> w,
                   std::shared_ptr<Directory> dir,
                   std::shared_ptr<RandomIndexWriter> saveW) ;

  std::shared_ptr<int64_t> randomBigInt(int numBytes);

  std::shared_ptr<Directory> getDirectory(int numPoints) ;

protected:
  bool mergeIsStable() override;

  // LUCENE-7491
public:
  virtual void testMixedSchema() ;

protected:
  std::shared_ptr<BasePointsFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BasePointsFormatTestCase>(
        BaseIndexFileFormatTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
