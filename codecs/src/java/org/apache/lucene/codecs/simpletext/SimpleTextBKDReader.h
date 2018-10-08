#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PointValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::index
{
class IntersectVisitor;
}
namespace org::apache::lucene::util::bkd
{
class BKDReader;
}
namespace org::apache::lucene::util::bkd
{
class IntersectState;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
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
namespace org::apache::lucene::codecs::simpletext
{

using PointValues = org::apache::lucene::index::PointValues;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_DOC_ID;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_VALUE;

/** Forked from {@link BKDReader} and simplified/specialized for SimpleText's
 * usage */

class SimpleTextBKDReader final : public PointValues, public Accountable
{
  GET_CLASS_NAME(SimpleTextBKDReader)
  // Packed array of byte[] holding all split values in the full binary tree:
private:
  std::deque<char> const splitPackedValues;

public:
  std::deque<int64_t> const leafBlockFPs;

private:
  const int leafNodeOffset;

public:
  const int numDims;
  const int bytesPerDim;
  const int bytesPerIndexEntry;
  const std::shared_ptr<IndexInput> in_;
  const int maxPointsInLeafNode;
  std::deque<char> const minPackedValue;
  std::deque<char> const maxPackedValue;
  const int64_t pointCount;
  const int docCount;
  const int version;

protected:
  const int packedBytesLength;

public:
  SimpleTextBKDReader(std::shared_ptr<IndexInput> in_, int numDims,
                      int maxPointsInLeafNode, int bytesPerDim,
                      std::deque<int64_t> &leafBlockFPs,
                      std::deque<char> &splitPackedValues,
                      std::deque<char> &minPackedValue,
                      std::deque<char> &maxPackedValue, int64_t pointCount,
                      int docCount) ;

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

    IntersectState(std::shared_ptr<IndexInput> in_, int numDims,
                   int packedBytesLength, int maxPointsInLeafNode,
                   std::shared_ptr<PointValues::IntersectVisitor> visitor);
  };

public:
  void intersect(std::shared_ptr<PointValues::IntersectVisitor> visitor) throw(
      IOException) override;

  /** Fast path: this is called when the query box fully encompasses all cells
   * under this node. */
private:
  void addAll(std::shared_ptr<BKDReader::IntersectState> state,
              int nodeID) ;

  /** Create a new {@link IntersectState} */
public:
  std::shared_ptr<BKDReader::IntersectState>
  getIntersectState(std::shared_ptr<PointValues::IntersectVisitor> visitor);

  /** Visits all docIDs and packed values in a single leaf block */
  void visitLeafBlockValues(
      int nodeID,
      std::shared_ptr<BKDReader::IntersectState> state) ;

  void visitDocIDs(std::shared_ptr<IndexInput> in_, int64_t blockFP,
                   std::shared_ptr<PointValues::IntersectVisitor>
                       visitor) ;

  int readDocIDs(std::shared_ptr<IndexInput> in_, int64_t blockFP,
                 std::deque<int> &docIDs) ;

  void visitDocValues(std::deque<int> &commonPrefixLengths,
                      std::deque<char> &scratchPackedValue,
                      std::shared_ptr<IndexInput> in_, std::deque<int> &docIDs,
                      int count,
                      std::shared_ptr<PointValues::IntersectVisitor>
                          visitor) ;

private:
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

  void intersect(std::shared_ptr<BKDReader::IntersectState> state, int nodeID,
                 std::deque<char> &cellMinPacked,
                 std::deque<char> &cellMaxPacked) ;

public:
  int64_t estimatePointCount(
      std::shared_ptr<PointValues::IntersectVisitor> visitor) override;

private:
  int64_t estimatePointCount(std::shared_ptr<BKDReader::IntersectState> state,
                               int nodeID, std::deque<char> &cellMinPacked,
                               std::deque<char> &cellMaxPacked);

  /** Copies the split value for this node into the provided byte array */
public:
  void copySplitValue(int nodeID, std::deque<char> &splitPackedValue);

  int64_t ramBytesUsed() override;

  std::deque<char> getMinPackedValue() override;

  std::deque<char> getMaxPackedValue() override;

  int getNumDimensions() override;

  int getBytesPerDimension() override;

  int64_t size() override;

  int getDocCount() override;

  bool isLeafNode(int nodeID);

private:
  int parseInt(std::shared_ptr<BytesRefBuilder> scratch,
               std::shared_ptr<BytesRef> prefix);

  std::wstring stripPrefix(std::shared_ptr<BytesRefBuilder> scratch,
                           std::shared_ptr<BytesRef> prefix);

  bool startsWith(std::shared_ptr<BytesRefBuilder> scratch,
                  std::shared_ptr<BytesRef> prefix);

  void readLine(std::shared_ptr<IndexInput> in_,
                std::shared_ptr<BytesRefBuilder> scratch) ;

protected:
  std::shared_ptr<SimpleTextBKDReader> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextBKDReader>(
        org.apache.lucene.index.PointValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
