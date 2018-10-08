#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class PointValues;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Test Indexing/IndexWriter with points */
class TestPointValues : public LuceneTestCase
{
  GET_CLASS_NAME(TestPointValues)

  // Suddenly add points to an existing field:
public:
  virtual void testUpgradeFieldToPoints() ;

  // Illegal schema change tests:

  virtual void testIllegalDimChangeOneDoc() ;

  virtual void testIllegalDimChangeTwoDocs() ;

  virtual void testIllegalDimChangeTwoSegments() ;

  virtual void testIllegalDimChangeTwoWriters() ;

  virtual void
  testIllegalDimChangeViaAddIndexesDirectory() ;

  virtual void
  testIllegalDimChangeViaAddIndexesCodecReader() ;

  virtual void
  testIllegalDimChangeViaAddIndexesSlowCodecReader() ;

  virtual void testIllegalNumBytesChangeOneDoc() ;

  virtual void testIllegalNumBytesChangeTwoDocs() ;

  virtual void testIllegalNumBytesChangeTwoSegments() ;

  virtual void testIllegalNumBytesChangeTwoWriters() ;

  virtual void
  testIllegalNumBytesChangeViaAddIndexesDirectory() ;

  virtual void
  testIllegalNumBytesChangeViaAddIndexesCodecReader() ;

  virtual void testIllegalNumBytesChangeViaAddIndexesSlowCodecReader() throw(
      std::runtime_error);

  virtual void testIllegalTooManyBytes() ;

  virtual void testIllegalTooManyDimensions() ;

  // Write point values, one segment with Lucene70, another with SimpleText,
  // then forceMerge with SimpleText
  virtual void testDifferentCodecs1() ;

  // Write point values, one segment with Lucene70, another with SimpleText,
  // then forceMerge with Lucene70
  virtual void testDifferentCodecs2() ;

  virtual void testInvalidIntPointUsage() ;

  virtual void testInvalidLongPointUsage() ;

  virtual void testInvalidFloatPointUsage() ;

  virtual void testInvalidDoublePointUsage() ;

  virtual void testTieBreakByDocID() ;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<TestPointValues> outerInstance;

  public:
    IntersectVisitorAnonymousInnerClass(
        std::shared_ptr<TestPointValues> outerInstance);

    int lastDocID = 0;

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue);
  };

public:
  virtual void testDeleteAllPointDocs() ;

  virtual void testPointsFieldMissingFromOneSegment() ;

  virtual void testSparsePoints() ;

  virtual void testCheckIndexIncludesPoints() ;

  virtual void testMergedStatsEmptyReader() ;

  virtual void testMergedStatsOneSegmentWithoutPoints() ;

  virtual void testMergedStatsAllPointsDeleted() ;

  virtual void testMergedStats() ;

private:
  static std::deque<std::deque<char>> randomBinaryValue(int numDims,
                                                          int numBytesPerDim);

  void doTestMergedStats() ;

protected:
  std::shared_ptr<TestPointValues> shared_from_this()
  {
    return std::static_pointer_cast<TestPointValues>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
