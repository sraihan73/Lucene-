#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PointValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefComparator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/MSBRadixSorter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/OfflineSorter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/PriorityQueue.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}

namespace org::apache::lucene::store
{
class TrackingDirectoryWrapper;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::util::bkd
{
class OfflinePointWriter;
}
namespace org::apache::lucene::util::bkd
{
class HeapPointWriter;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::util
{
class OfflineSorter;
}
namespace org::apache::lucene::util
{
class BufferSize;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::codecs::simpletext
{
class SimpleTextBKDReader;
}
namespace org::apache::lucene::codecs::simpletext
{
class IntersectState;
}
namespace org::apache::lucene::index
{
class MergeState;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::util::bkd
{
class BKDWriter;
}
namespace org::apache::lucene::util::bkd
{
class MergeReader;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::codecs
{
class MutablePointValues;
}
namespace org::apache::lucene::util::bkd
{
class OneDimensionBKDWriter;
}
namespace org::apache::lucene::util::bkd
{
class PointWriter;
}
namespace org::apache::lucene::util
{
class ByteSequencesWriter;
}
namespace org::apache::lucene::store
{
class ChecksumIndexInput;
}
namespace org::apache::lucene::util
{
class ByteSequencesReader;
}
namespace org::apache::lucene::util
{
class LongBitSet;
}
namespace org::apache::lucene::util::bkd
{
class PathSlice;
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

using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using MergeState = org::apache::lucene::index::MergeState;
using Directory = org::apache::lucene::store::Directory;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using HeapPointWriter = org::apache::lucene::util::bkd::HeapPointWriter;
using OfflinePointWriter = org::apache::lucene::util::bkd::OfflinePointWriter;
using PointWriter = org::apache::lucene::util::bkd::PointWriter;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_DOC_ID;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_FP;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_VALUE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BYTES_PER_DIM;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.DOC_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.INDEX_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MAX_LEAF_POINTS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MAX_VALUE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MIN_VALUE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.NUM_DIMS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.POINT_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_COUNT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_DIM;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_VALUE;

// TODO
//   - allow variable length byte[] (across docs and dims), but this is quite a
//   bit more hairy
//   - we could also index "auto-prefix terms" here, and use better compression,
//   and maybe only use for the "fully contained" case so we'd
//     only index docIDs
//   - the index could be efficiently encoded as an FST, so we don't have
//   wasteful
//     (monotonic) long[] leafBlockFPs; or we could use MonotonicLongValues ...
//     but then the index is already plenty small: 60M OSM points --> 1.1 MB
//     with 128 points per leaf, and you can reduce that by putting more points
//     per leaf
//   - we could use threads while building; the higher nodes are very
//   parallelizable

/** Forked from {@link BKDWriter} and simplified/specialized for SimpleText's
 * usage */

class SimpleTextBKDWriter final
    : public std::enable_shared_from_this<SimpleTextBKDWriter>
{
  GET_CLASS_NAME(SimpleTextBKDWriter)

public:
  static const std::wstring CODEC_NAME;
  static constexpr int VERSION_START = 0;
  static constexpr int VERSION_COMPRESSED_DOC_IDS = 1;
  static constexpr int VERSION_COMPRESSED_VALUES = 2;
  static constexpr int VERSION_IMPLICIT_SPLIT_DIM_1D = 3;
  static constexpr int VERSION_CURRENT = VERSION_IMPLICIT_SPLIT_DIM_1D;

  /** How many bytes each docs takes in the fixed-width offline format */
private:
  const int bytesPerDoc;

  /** Default maximum number of point in each leaf block */
public:
  static constexpr int DEFAULT_MAX_POINTS_IN_LEAF_NODE = 1024;

  /** Default maximum heap to use, before spilling to (slower) disk */
  static constexpr float DEFAULT_MAX_MB_SORT_IN_HEAP = 16.0f;

  /** Maximum number of dimensions */
  static constexpr int MAX_DIMS = 8;

  /** How many dimensions we are indexing */
protected:
  const int numDims;

  /** How many bytes each value in each dimension takes. */
  const int bytesPerDim;

  /** numDims * bytesPerDim */
  const int packedBytesLength;

public:
  const std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();

  const std::shared_ptr<TrackingDirectoryWrapper> tempDir;
  const std::wstring tempFileNamePrefix;
  const double maxMBSortInHeap;

  std::deque<char> const scratchDiff;
  std::deque<char> const scratch1;
  std::deque<char> const scratch2;
  const std::shared_ptr<BytesRef> scratchBytesRef1 =
      std::make_shared<BytesRef>();
  const std::shared_ptr<BytesRef> scratchBytesRef2 =
      std::make_shared<BytesRef>();
  std::deque<int> const commonPrefixLengths;

protected:
  const std::shared_ptr<FixedBitSet> docsSeen;

private:
  std::shared_ptr<OfflinePointWriter> offlinePointWriter;
  std::shared_ptr<HeapPointWriter> heapPointWriter;

  std::shared_ptr<IndexOutput> tempInput;

protected:
  const int maxPointsInLeafNode;

private:
  const int maxPointsSortInHeap;

  /** Minimum per-dim values, packed */
protected:
  std::deque<char> const minPackedValue;

  /** Maximum per-dim values, packed */
  std::deque<char> const maxPackedValue;

  int64_t pointCount = 0;

  /** true if we have so many values that we must write ords using long (8
   * bytes) instead of int (4 bytes) */
  const bool longOrds;

  /** An upper bound on how many points the caller will add (includes deletions)
   */
private:
  const int64_t totalPointCount;

  /** True if every document has at most one value.  We specialize this case by
   * not bothering to store the ord since it's redundant with docID.  */
protected:
  const bool singleValuePerDoc;

  /** How much heap OfflineSorter is allowed to use */
  const std::shared_ptr<OfflineSorter::BufferSize> offlineSorterBufferMB;

  /** How much heap OfflineSorter is allowed to use */
  const int offlineSorterMaxTempFiles;

private:
  const int maxDoc;

public:
  SimpleTextBKDWriter(int maxDoc, std::shared_ptr<Directory> tempDir,
                      const std::wstring &tempFileNamePrefix, int numDims,
                      int bytesPerDim, int maxPointsInLeafNode,
                      double maxMBSortInHeap, int64_t totalPointCount,
                      bool singleValuePerDoc) ;

private:
  SimpleTextBKDWriter(int maxDoc, std::shared_ptr<Directory> tempDir,
                      const std::wstring &tempFileNamePrefix, int numDims,
                      int bytesPerDim, int maxPointsInLeafNode,
                      double maxMBSortInHeap, int64_t totalPointCount,
                      bool singleValuePerDoc, bool longOrds,
                      int64_t offlineSorterBufferMB,
                      int offlineSorterMaxTempFiles) ;

public:
  static void verifyParams(int numDims, int maxPointsInLeafNode,
                           double maxMBSortInHeap, int64_t totalPointCount);

  /** If the current segment has too many points then we spill over to temp
   * files / offline sort. */
private:
  void spillToOffline() ;

public:
  void add(std::deque<char> &packedValue, int docID) ;

  /** How many points have been added so far */
  int64_t getPointCount();

private:
  class MergeReader : public std::enable_shared_from_this<MergeReader>
  {
    GET_CLASS_NAME(MergeReader)
  public:
    const std::shared_ptr<SimpleTextBKDReader> bkd;
    const std::shared_ptr<SimpleTextBKDReader::IntersectState> state;
    const std::shared_ptr<MergeState::DocMap> docMap;

    /** Current doc ID */
    int docID = 0;

    /** Which doc in this block we are up to */
  private:
    int docBlockUpto = 0;

    /** How many docs in the current block */
    int docsInBlock = 0;

    /** Which leaf block we are up to */
    int blockID = 0;

    std::deque<char> const packedValues;

  public:
    MergeReader(std::shared_ptr<SimpleTextBKDReader> bkd,
                std::shared_ptr<MergeState::DocMap> docMap) ;

    virtual bool next() ;

  private:
    class IntersectVisitorAnonymousInnerClass
        : public std::enable_shared_from_this<
              IntersectVisitorAnonymousInnerClass>,
          public IntersectVisitor
    {
      GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
    private:
      std::shared_ptr<BKDWriter::MergeReader> outerInstance;

    public:
      IntersectVisitorAnonymousInnerClass(
          std::shared_ptr<BKDWriter::MergeReader> outerInstance);

      int i = 0;

      void visit(int docID)  override;

      void visit(int docID,
                 std::deque<char> &packedValue)  override;

      Relation compare(std::deque<char> &minPackedValue,
                       std::deque<char> &maxPackedValue);
    };
  };

private:
  class BKDMergeQueue : public PriorityQueue<std::shared_ptr<MergeReader>>
  {
    GET_CLASS_NAME(BKDMergeQueue)
  private:
    const int bytesPerDim;

  public:
    BKDMergeQueue(int bytesPerDim, int maxSize);

    bool lessThan(std::shared_ptr<BKDWriter::MergeReader> a,
                  std::shared_ptr<BKDWriter::MergeReader> b) override;

  protected:
    std::shared_ptr<BKDMergeQueue> shared_from_this()
    {
      return std::static_pointer_cast<BKDMergeQueue>(
          org.apache.lucene.util
              .PriorityQueue<MergeReader>::shared_from_this());
    }
  };

  /** Write a field from a {@link MutablePointValues}. This way of writing
   *  points is faster than regular writes with {@link BKDWriter#add} since
   *  there is opportunity for reordering points before writing them to
   *  disk. This method does not use transient disk in order to reorder points.
   */
public:
  int64_t
  writeField(std::shared_ptr<IndexOutput> out, const std::wstring &fieldName,
             std::shared_ptr<MutablePointValues> reader) ;

  /* In the 2+D case, we recursively pick the split dimension, compute the
   * median value and partition other values around it. */
private:
  int64_t writeFieldNDims(
      std::shared_ptr<IndexOutput> out, const std::wstring &fieldName,
      std::shared_ptr<MutablePointValues> values) ;

  /* In the 1D case, we can simply sort points in ascending order and use the
   * same writing logic as we use at merge time. */
  int64_t
  writeField1Dim(std::shared_ptr<IndexOutput> out,
                 const std::wstring &fieldName,
                 std::shared_ptr<MutablePointValues> reader) ;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextBKDWriter> outerInstance;

    std::shared_ptr<BKDWriter::OneDimensionBKDWriter> oneDimWriter;

  public:
    IntersectVisitorAnonymousInnerClass(
        std::shared_ptr<SimpleTextBKDWriter> outerInstance,
        std::shared_ptr<BKDWriter::OneDimensionBKDWriter> oneDimWriter);

    void visit(int docID,
               std::deque<char> &packedValue)  override;

    void visit(int docID)  override;

    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue);
  };

  // TODO: remove this opto: SimpleText is supposed to be simple!

  /** More efficient bulk-add for incoming {@link SimpleTextBKDReader}s.  This
   * does a merge sort of the already sorted values and currently only works
   * when numDims==1.  This returns -1 if all documents containing dimensional
   * values were deleted. */
public:
  int64_t merge(std::shared_ptr<IndexOutput> out,
                  std::deque<std::shared_ptr<MergeState::DocMap>> &docMaps,
                  std::deque<std::shared_ptr<SimpleTextBKDReader>>
                      &readers) ;

private:
  class OneDimensionBKDWriter
      : public std::enable_shared_from_this<OneDimensionBKDWriter>
  {
    GET_CLASS_NAME(OneDimensionBKDWriter)
  private:
    std::shared_ptr<SimpleTextBKDWriter> outerInstance;

  public:
    const std::shared_ptr<IndexOutput> out;
    const std::deque<int64_t> leafBlockFPs = std::deque<int64_t>();
    const std::deque<std::deque<char>> leafBlockStartValues =
        std::deque<std::deque<char>>();
    std::deque<char> const leafValues = std::deque<char>(
        outerInstance->maxPointsInLeafNode * outerInstance->packedBytesLength);
    std::deque<int> const leafDocs =
        std::deque<int>(outerInstance->maxPointsInLeafNode);
    int64_t valueCount = 0;
    int leafCount = 0;

    OneDimensionBKDWriter(std::shared_ptr<SimpleTextBKDWriter> outerInstance,
                          std::shared_ptr<IndexOutput> out);

    // for asserts
    std::deque<char> const lastPackedValue;
    int lastDocID = 0;

    virtual void add(std::deque<char> &packedValue,
                     int docID) ;

    virtual int64_t finish() ;

  private:
    void writeLeafBlock() ;

  private:
    class functionAnonymousInnerClass : public std::function<BytesRef *(int)>
    {
      GET_CLASS_NAME(functionAnonymousInnerClass)
    private:
      std::shared_ptr<BKDWriter::OneDimensionBKDWriter> outerInstance;

    public:
      functionAnonymousInnerClass(
          std::shared_ptr<BKDWriter::OneDimensionBKDWriter> outerInstance,
          std::shared_ptr<UnknownType> int);

      const std::shared_ptr<BytesRef> outerInstance->outerInstance->scratch;

      std::shared_ptr<BytesRef> apply(int i) override;

    protected:
      std::shared_ptr<functionAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<functionAnonymousInnerClass>(
            std::function<org.apache.lucene.util.BytesRef *(
                int)>::shared_from_this());
      }
    };
  };

  // TODO: there must be a simpler way?
private:
  void rotateToTree(int nodeID, int offset, int count, std::deque<char> &index,
                    std::deque<std::deque<char>> &leafBlockStartValues);

  // TODO: if we fixed each partition step to just record the file offset at the
  // "split point", we could probably handle variable length encoding and not
  // have our own ByteSequencesReader/Writer

  /** Sort the heap writer by the specified dim */
  void sortHeapPointWriter(std::shared_ptr<HeapPointWriter> writer, int dim);

private:
  class MSBRadixSorterAnonymousInnerClass : public MSBRadixSorter
  {
    GET_CLASS_NAME(MSBRadixSorterAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextBKDWriter> outerInstance;

    std::shared_ptr<HeapPointWriter> writer;
    int dim = 0;

  public:
    MSBRadixSorterAnonymousInnerClass(
        std::shared_ptr<SimpleTextBKDWriter> outerInstance,
        std::shared_ptr<UnknownType> BYTES,
        std::shared_ptr<HeapPointWriter> writer, int dim);

  protected:
    int byteAt(int i, int k) override;

    void swap(int i, int j) override;

  protected:
    std::shared_ptr<MSBRadixSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<MSBRadixSorterAnonymousInnerClass>(
          org.apache.lucene.util.MSBRadixSorter::shared_from_this());
    }
  };

private:
  std::shared_ptr<PointWriter> sort(int dim) ;

private:
  class BytesRefComparatorAnonymousInnerClass : public BytesRefComparator
  {
    GET_CLASS_NAME(BytesRefComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextBKDWriter> outerInstance;

    int offset = 0;

  public:
    BytesRefComparatorAnonymousInnerClass(
        std::shared_ptr<SimpleTextBKDWriter> outerInstance,
        std::shared_ptr<UnknownType> BYTES, int offset);

  protected:
    int byteAt(std::shared_ptr<BytesRef> ref, int i) override;

  protected:
    std::shared_ptr<BytesRefComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BytesRefComparatorAnonymousInnerClass>(
          org.apache.lucene.util.BytesRefComparator::shared_from_this());
    }
  };

private:
  class BytesRefComparatorAnonymousInnerClass2 : public BytesRefComparator
  {
    GET_CLASS_NAME(BytesRefComparatorAnonymousInnerClass2)
  private:
    std::shared_ptr<SimpleTextBKDWriter> outerInstance;

    int offset = 0;

  public:
    BytesRefComparatorAnonymousInnerClass2(
        std::shared_ptr<SimpleTextBKDWriter> outerInstance,
        std::shared_ptr<UnknownType> BYTES, int offset);

  protected:
    int byteAt(std::shared_ptr<BytesRef> ref, int i) override;

  protected:
    std::shared_ptr<BytesRefComparatorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<BytesRefComparatorAnonymousInnerClass2>(
          org.apache.lucene.util.BytesRefComparator::shared_from_this());
    }
  };

private:
  class OfflineSorterAnonymousInnerClass : public OfflineSorter
  {
    GET_CLASS_NAME(OfflineSorterAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextBKDWriter> outerInstance;

  public:
    OfflineSorterAnonymousInnerClass(
        std::shared_ptr<SimpleTextBKDWriter> outerInstance,
        std::shared_ptr<TrackingDirectoryWrapper> tempDir,
        const std::wstring &tempFileNamePrefix,
        std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> cmp,
        std::shared_ptr<OfflineSorter::BufferSize> offlineSorterBufferMB,
        int offlineSorterMaxTempFiles, int bytesPerDoc);

    /** We write/read fixed-byte-width file that {@link OfflinePointReader} can
     * read. */
  protected:
    std::shared_ptr<OfflineSorter::ByteSequencesWriter>
    getWriter(std::shared_ptr<IndexOutput> out, int64_t count) override;

  private:
    class ByteSequencesWriterAnonymousInnerClass : public ByteSequencesWriter
    {
      GET_CLASS_NAME(ByteSequencesWriterAnonymousInnerClass)
    private:
      std::shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance;

      std::shared_ptr<IndexOutput> out;

    public:
      ByteSequencesWriterAnonymousInnerClass(
          std::shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance,
          std::shared_ptr<IndexOutput> out);

      void write(std::deque<char> &bytes, int off,
                 int len)  override;

    protected:
      std::shared_ptr<ByteSequencesWriterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ByteSequencesWriterAnonymousInnerClass>(
            ByteSequencesWriter::shared_from_this());
      }
    };

    /** We write/read fixed-byte-width file that {@link OfflinePointReader} can
     * read. */
  protected:
    std::shared_ptr<OfflineSorter::ByteSequencesReader>
    getReader(std::shared_ptr<ChecksumIndexInput> in_,
              const std::wstring &name)  override;

  private:
    class ByteSequencesReaderAnonymousInnerClass : public ByteSequencesReader
    {
      GET_CLASS_NAME(ByteSequencesReaderAnonymousInnerClass)
    private:
      std::shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance;

      std::shared_ptr<ChecksumIndexInput> in_;

    public:
      ByteSequencesReaderAnonymousInnerClass(
          std::shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance,
          std::shared_ptr<ChecksumIndexInput> in_, const std::wstring &name);

      const std::shared_ptr<BytesRef> outerInstance->outerInstance.scratch;
      std::shared_ptr<BytesRef> next()  override;

    protected:
      std::shared_ptr<ByteSequencesReaderAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ByteSequencesReaderAnonymousInnerClass>(
            ByteSequencesReader::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<OfflineSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<OfflineSorterAnonymousInnerClass>(
          org.apache.lucene.util.OfflineSorter::shared_from_this());
    }
  };

private:
  void checkMaxLeafNodeCount(int numLeaves);

  /** Writes the BKD tree to the provided {@link IndexOutput} and returns the
   * file offset where index was written. */
public:
  int64_t finish(std::shared_ptr<IndexOutput> out) ;

  /** Subclass can change how it writes the index. */
private:
  void writeIndex(std::shared_ptr<IndexOutput> out,
                  std::deque<int64_t> &leafBlockFPs,
                  std::deque<char> &splitPackedValues) ;

protected:
  void writeLeafBlockDocs(std::shared_ptr<IndexOutput> out,
                          std::deque<int> &docIDs, int start,
                          int count) ;

  void writeLeafBlockPackedValues(
      std::shared_ptr<IndexOutput> out, std::deque<int> &commonPrefixLengths,
      int count, int sortedDim,
      std::function<BytesRef *(int)> &packedValues) ;

private:
  void writeLeafBlockPackedValuesRange(
      std::shared_ptr<IndexOutput> out, std::deque<int> &commonPrefixLengths,
      int start, int end,
      std::function<BytesRef *(int)> &packedValues) ;

  static int runLen(std::function<BytesRef *(int)> &packedValues, int start,
                    int end, int byteOffset);

public:
  virtual ~SimpleTextBKDWriter();

  /** Sliced reference to points in an OfflineSorter.ByteSequencesWriter file.
   */
private:
  class PathSlice final : public std::enable_shared_from_this<PathSlice>
  {
    GET_CLASS_NAME(PathSlice)
  public:
    const std::shared_ptr<PointWriter> writer;
    const int64_t start;
    const int64_t count;

    PathSlice(std::shared_ptr<PointWriter> writer, int64_t start,
              int64_t count);

    virtual std::wstring toString();
  };

  /** Called on exception, to check whether the checksum is also corrupt in this
   * source, and add that information (checksum matched or didn't) as a
   * suppressed exception. */
private:
  std::shared_ptr<Error>
  verifyChecksum(std::runtime_error priorException,
                 std::shared_ptr<PointWriter> writer) ;

  /** Marks bits for the ords (points) that belong in the right sub tree (those
   * docs that have values >= the splitValue). */
  std::deque<char>
  markRightTree(int64_t rightCount, int splitDim,
                std::shared_ptr<BKDWriter::PathSlice> source,
                std::shared_ptr<LongBitSet> ordBitSet) ;

  /** Called only in assert */
  bool valueInBounds(std::shared_ptr<BytesRef> packedValue,
                     std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue);

protected:
  int split(std::deque<char> &minPackedValue,
            std::deque<char> &maxPackedValue);

  /** Pull a partition back into heap once the point count is low enough while
   * recursing. */
private:
  std::shared_ptr<BKDWriter::PathSlice>
  switchToHeap(std::shared_ptr<BKDWriter::PathSlice> source,
               std::deque<std::shared_ptr<Closeable>>
                   &toCloseHeroically) ;

  /* Recursively reorders the provided reader and writes the bkd-tree on the
   * fly. */
  void build(int nodeID, int leafNodeOffset,
             std::shared_ptr<MutablePointValues> reader, int from, int to,
             std::shared_ptr<IndexOutput> out,
             std::deque<char> &minPackedValue,
             std::deque<char> &maxPackedValue,
             std::deque<char> &splitPackedValues,
             std::deque<int64_t> &leafBlockFPs,
             std::deque<int> &spareDocIds) ;

  /** The array (sized numDims) of PathSlice describe the cell we have currently
   * recursed to. */
  void build(int nodeID, int leafNodeOffset,
             std::deque<std::shared_ptr<BKDWriter::PathSlice>> &slices,
             std::shared_ptr<LongBitSet> ordBitSet,
             std::shared_ptr<IndexOutput> out,
             std::deque<char> &minPackedValue,
             std::deque<char> &maxPackedValue,
             std::deque<char> &splitPackedValues,
             std::deque<int64_t> &leafBlockFPs,
             std::deque<std::shared_ptr<Closeable>>
                 &toCloseHeroically) ;

private:
  class functionAnonymousInnerClass : public std::function<BytesRef *(int)>
  {
    GET_CLASS_NAME(functionAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextBKDWriter> outerInstance;

    std::shared_ptr<BKDWriter::PathSlice> source;
    std::shared_ptr<HeapPointWriter> heapSource;

  public:
    functionAnonymousInnerClass(
        std::shared_ptr<SimpleTextBKDWriter> outerInstance,
        std::shared_ptr<UnknownType> int,
        std::shared_ptr<BKDWriter::PathSlice> source,
        std::shared_ptr<HeapPointWriter> heapSource);

    const std::shared_ptr<BytesRef> outerInstance->scratch;

    std::shared_ptr<BytesRef> apply(int i) override;

  protected:
    std::shared_ptr<functionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<functionAnonymousInnerClass>(
          std::function<org.apache.lucene.util.BytesRef *(
              int)>::shared_from_this());
    }
  };

  // only called from assert
private:
  bool valuesInOrderAndBounds(int count, int sortedDim,
                              std::deque<char> &minPackedValue,
                              std::deque<char> &maxPackedValue,
                              std::function<BytesRef *(int)> &values,
                              std::deque<int> &docs,
                              int docsOffset) ;

  // only called from assert
  bool valueInOrder(int64_t ord, int sortedDim,
                    std::deque<char> &lastPackedValue,
                    std::deque<char> &packedValue, int packedValueOffset,
                    int doc, int lastDoc);

public:
  std::shared_ptr<PointWriter>
  getPointWriter(int64_t count, const std::wstring &desc) ;

private:
  void write(std::shared_ptr<IndexOutput> out,
             const std::wstring &s) ;

  void writeInt(std::shared_ptr<IndexOutput> out, int x) ;

  void writeLong(std::shared_ptr<IndexOutput> out,
                 int64_t x) ;

  void write(std::shared_ptr<IndexOutput> out,
             std::shared_ptr<BytesRef> b) ;

  void newline(std::shared_ptr<IndexOutput> out) ;
};

} // namespace org::apache::lucene::codecs::simpletext
