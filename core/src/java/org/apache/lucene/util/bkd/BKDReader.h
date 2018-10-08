#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/IntersectVisitor.h"

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

using PointValues = org::apache::lucene::index::PointValues;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Handles intersection of an multi-dimensional shape in byte[] space with a
 * block KD-tree previously written with {@link BKDWriter}.
 *
 * @lucene.experimental */

class BKDReader final : public PointValues, public Accountable
{
  GET_CLASS_NAME(BKDReader)
  // Packed array of byte[] holding all split values in the full binary tree:
public:
  const int leafNodeOffset;
  const int numDims;
  const int bytesPerDim;
  const int numLeaves;
  const std::shared_ptr<IndexInput> in_;
  const int maxPointsInLeafNode;
  std::deque<char> const minPackedValue;
  std::deque<char> const maxPackedValue;
  const int64_t pointCount;
  const int docCount;
  const int version;

protected:
  const int packedBytesLength;

  // Used for 6.4.0+ index format:
public:
  std::deque<char> const packedIndex;

  // Used for Legacy (pre-6.4.0) index format, to hold a compact form of the
  // index:
private:
  std::deque<char> const splitPackedValues;

public:
  const int bytesPerIndexEntry;
  std::deque<int64_t> const leafBlockFPs;

  /** Caller must pre-seek the provided {@link IndexInput} to the index location
   * that {@link BKDWriter#finish} returned */
  BKDReader(std::shared_ptr<IndexInput> in_) ;

  int64_t getMinLeafBlockFP();

  /** Used to walk the in-heap index
   *
   * @lucene.internal */
public:
  class IndexTree : public std::enable_shared_from_this<IndexTree>,
                    public Cloneable
  {
    GET_CLASS_NAME(IndexTree)
  private:
    std::shared_ptr<BKDReader> outerInstance;

  protected:
    int nodeID = 0;
    // level is 1-based so that we can do level-1 w/o checking each time:
    int level = 0;
    int splitDim = 0;
    std::deque<std::deque<char>> const splitPackedValueStack;

    IndexTree(std::shared_ptr<BKDReader> outerInstance);

  public:
    virtual void pushLeft();

    /** Clone, but you are not allowed to pop up past the point where the clone
     * happened. */
    virtual std::shared_ptr<IndexTree> clone() = 0;

    virtual void pushRight();

    virtual void pop();

    virtual bool isLeafNode();

    virtual bool nodeExists();

    virtual int getNodeID();

    virtual std::deque<char> getSplitPackedValue();

    /** Only valid after pushLeft or pushRight, not pop! */
    virtual int getSplitDim();

    /** Only valid after pushLeft or pushRight, not pop! */
    virtual std::shared_ptr<BytesRef> getSplitDimValue() = 0;

    /** Only valid after pushLeft or pushRight, not pop! */
    virtual int64_t getLeafBlockFP() = 0;

    /** Return the number of leaves below the current node. */
    virtual int getNumLeaves();

    // for assertions
  private:
    int getNumLeavesSlow(int node);
  };

  /** Reads the original simple yet heap-heavy index format */
private:
  class LegacyIndexTree final : public IndexTree
  {
    GET_CLASS_NAME(LegacyIndexTree)
  private:
    std::shared_ptr<BKDReader> outerInstance;

    int64_t leafBlockFP = 0;
    std::deque<char> const splitDimValue =
        std::deque<char>(outerInstance->bytesPerDim);
    const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();

  public:
    LegacyIndexTree(std::shared_ptr<BKDReader> outerInstance);

    std::shared_ptr<LegacyIndexTree> clone() override;

    void pushLeft() override;

    void pushRight() override;

  private:
    void setNodeData();

  public:
    int64_t getLeafBlockFP() override;

    std::shared_ptr<BytesRef> getSplitDimValue() override;

    void pop() override;

  protected:
    std::shared_ptr<LegacyIndexTree> shared_from_this()
    {
      return std::static_pointer_cast<LegacyIndexTree>(
          IndexTree::shared_from_this());
    }
  };

  /** Reads the new packed byte[] index format which can be up to ~63% smaller
   * than the legacy index format on 20M NYC taxis tests.  This format takes
   * advantage of the limited access pattern to the BKD tree at search time,
   * i.e. starting at the root node and recursing
   *  downwards one child at a time. */
private:
  class PackedIndexTree final : public IndexTree
  {
    GET_CLASS_NAME(PackedIndexTree)
  private:
    std::shared_ptr<BKDReader> outerInstance;

    // used to read the packed byte[]
    const std::shared_ptr<ByteArrayDataInput> in_;
    // holds the minimum (left most) leaf block file pointer for each level
    // we've recursed to:
    std::deque<int64_t> const leafBlockFPStack;
    // holds the address, in the packed byte[] index, of the left-node of each
    // level:
    std::deque<int> const leftNodePositions;
    // holds the address, in the packed byte[] index, of the right-node of each
    // level:
    std::deque<int> const rightNodePositions;
    // holds the splitDim for each level:
    std::deque<int> const splitDims;
    // true if the per-dim delta we read for the node at this level is a
    // negative offset vs. the last split on this dim; this is a packed 2D
    // array, i.e. to access array[level][dim] you read from
    // negativeDeltas[level*numDims+dim].  this will be true if the last time we
    // split on this dimension, we next pushed to the left sub-tree:
    std::deque<bool> const negativeDeltas;
    // holds the packed per-level split values; the intersect method uses this
    // to save the cell min/max as it recurses:
    std::deque<std::deque<char>> const splitValuesStack;
    // scratch value to return from getPackedValue:
    const std::shared_ptr<BytesRef> scratch;

  public:
    PackedIndexTree(std::shared_ptr<BKDReader> outerInstance);

    std::shared_ptr<PackedIndexTree> clone() override;

    void pushLeft() override;

    void pushRight() override;

    void pop() override;

    int64_t getLeafBlockFP() override;

    std::shared_ptr<BytesRef> getSplitDimValue() override;

  private:
    void readNodeData(bool isLeft);

  protected:
    std::shared_ptr<PackedIndexTree> shared_from_this()
    {
      return std::static_pointer_cast<PackedIndexTree>(
          IndexTree::shared_from_this());
    }
  };

private:
  int getTreeDepth();

  /** Used to track all state for a single call to {@link #intersect}. */
public:
  class IntersectState final
      : public std::enable_shared_from_this<IntersectState>
  {
    GET_CLASS_NAME(IntersectState)
  public:
    const std::shared_ptr<IndexInput> in_;
    std::deque<int> const scratchDocIDs;
    std::deque<char> const scratchPackedValue;
    std::deque<int> const commonPrefixLengths;

    const std::shared_ptr<PointValues::IntersectVisitor> visitor;
    const std::shared_ptr<IndexTree> index;

    IntersectState(std::shared_ptr<IndexInput> in_, int numDims,
                   int packedBytesLength, int maxPointsInLeafNode,
                   std::shared_ptr<PointValues::IntersectVisitor> visitor,
                   std::shared_ptr<IndexTree> indexVisitor);
  };

public:
  void intersect(std::shared_ptr<PointValues::IntersectVisitor> visitor) throw(
      IOException) override;

  int64_t estimatePointCount(
      std::shared_ptr<PointValues::IntersectVisitor> visitor) override;

  /** Fast path: this is called when the query box fully encompasses all cells
   * under this node. */
private:
  void addAll(std::shared_ptr<IntersectState> state,
              bool grown) ;

  /** Create a new {@link IntersectState} */
public:
  std::shared_ptr<IntersectState>
  getIntersectState(std::shared_ptr<PointValues::IntersectVisitor> visitor);

  /** Visits all docIDs and packed values in a single leaf block */
  void visitLeafBlockValues(
      std::shared_ptr<IndexTree> index,
      std::shared_ptr<IntersectState> state) ;

private:
  void visitDocIDs(std::shared_ptr<IndexInput> in_, int64_t blockFP,
                   std::shared_ptr<PointValues::IntersectVisitor>
                       visitor) ;

public:
  int readDocIDs(std::shared_ptr<IndexInput> in_, int64_t blockFP,
                 std::deque<int> &docIDs) ;

  void visitDocValues(std::deque<int> &commonPrefixLengths,
                      std::deque<char> &scratchPackedValue,
                      std::shared_ptr<IndexInput> in_, std::deque<int> &docIDs,
                      int count,
                      std::shared_ptr<PointValues::IntersectVisitor>
                          visitor) ;

  // Just read suffixes for every dimension
private:
  void visitRawDocValues(std::deque<int> &commonPrefixLengths,
                         std::deque<char> &scratchPackedValue,
                         std::shared_ptr<IndexInput> in_,
                         std::deque<int> &docIDs, int count,
                         std::shared_ptr<PointValues::IntersectVisitor>
                             visitor) ;

  void visitCompressedDocValues(
      std::deque<int> &commonPrefixLengths,
      std::deque<char> &scratchPackedValue, std::shared_ptr<IndexInput> in_,
      std::deque<int> &docIDs, int count,
      std::shared_ptr<PointValues::IntersectVisitor> visitor,
      int compressedDim) ;

  int readCompressedDim(std::shared_ptr<IndexInput> in_) ;

  void readCommonPrefixes(std::deque<int> &commonPrefixLengths,
                          std::deque<char> &scratchPackedValue,
                          std::shared_ptr<IndexInput> in_) ;

  void intersect(std::shared_ptr<IntersectState> state,
                 std::deque<char> &cellMinPacked,
                 std::deque<char> &cellMaxPacked) ;

  int64_t estimatePointCount(std::shared_ptr<IntersectState> state,
                               std::deque<char> &cellMinPacked,
                               std::deque<char> &cellMaxPacked);

public:
  int64_t ramBytesUsed() override;

  std::deque<char> getMinPackedValue() override;

  std::deque<char> getMaxPackedValue() override;

  int getNumDimensions() override;

  int getBytesPerDimension() override;

  int64_t size() override;

  int getDocCount() override;

  bool isLeafNode(int nodeID);

protected:
  std::shared_ptr<BKDReader> shared_from_this()
  {
    return std::static_pointer_cast<BKDReader>(
        org.apache.lucene.index.PointValues::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/bkd/
