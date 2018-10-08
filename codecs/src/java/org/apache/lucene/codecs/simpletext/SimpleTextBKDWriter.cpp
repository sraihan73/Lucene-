using namespace std;

#include "SimpleTextBKDWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/MutablePointValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/MergeState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/TrackingDirectoryWrapper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/LongBitSet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/NumericUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/bkd/BKDWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/bkd/HeapPointWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/bkd/MutablePointsReaderUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/bkd/OfflinePointWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/bkd/PointReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/bkd/PointWriter.h"
#include "SimpleTextBKDReader.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using MergeState = org::apache::lucene::index::MergeState;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefComparator = org::apache::lucene::util::BytesRefComparator;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using MSBRadixSorter = org::apache::lucene::util::MSBRadixSorter;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using StringHelper = org::apache::lucene::util::StringHelper;
using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;
using HeapPointWriter = org::apache::lucene::util::bkd::HeapPointWriter;
using MutablePointsReaderUtils =
    org::apache::lucene::util::bkd::MutablePointsReaderUtils;
using OfflinePointReader = org::apache::lucene::util::bkd::OfflinePointReader;
using OfflinePointWriter = org::apache::lucene::util::bkd::OfflinePointWriter;
using PointReader = org::apache::lucene::util::bkd::PointReader;
using PointWriter = org::apache::lucene::util::bkd::PointWriter;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_DOC_ID;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_FP;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_VALUE;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BYTES_PER_DIM;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.DOC_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.INDEX_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MAX_LEAF_POINTS;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MAX_VALUE;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.MIN_VALUE;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.NUM_DIMS;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.POINT_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_DIM;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.SPLIT_VALUE;
const wstring SimpleTextBKDWriter::CODEC_NAME = L"BKD";

SimpleTextBKDWriter::SimpleTextBKDWriter(
    int maxDoc, shared_ptr<Directory> tempDir,
    const wstring &tempFileNamePrefix, int numDims, int bytesPerDim,
    int maxPointsInLeafNode, double maxMBSortInHeap, int64_t totalPointCount,
    bool singleValuePerDoc) 
    : SimpleTextBKDWriter(
          maxDoc, tempDir, tempFileNamePrefix, numDims, bytesPerDim,
          maxPointsInLeafNode, maxMBSortInHeap, totalPointCount,
          singleValuePerDoc, totalPointCount > numeric_limits<int>::max(),
          Math::max(1, (long)maxMBSortInHeap), OfflineSorter::MAX_TEMPFILES)
{
}

SimpleTextBKDWriter::SimpleTextBKDWriter(
    int maxDoc, shared_ptr<Directory> tempDir,
    const wstring &tempFileNamePrefix, int numDims, int bytesPerDim,
    int maxPointsInLeafNode, double maxMBSortInHeap, int64_t totalPointCount,
    bool singleValuePerDoc, bool longOrds, int64_t offlineSorterBufferMB,
    int offlineSorterMaxTempFiles) 
    : numDims(numDims), bytesPerDim(bytesPerDim),
      packedBytesLength(numDims * bytesPerDim),
      tempDir(make_shared<TrackingDirectoryWrapper>(tempDir)),
      tempFileNamePrefix(tempFileNamePrefix), maxMBSortInHeap(maxMBSortInHeap),
      scratchDiff(std::deque<char>(bytesPerDim)),
      scratch1(std::deque<char>(packedBytesLength)),
      scratch2(std::deque<char>(packedBytesLength)),
      commonPrefixLengths(std::deque<int>(numDims)),
      docsSeen(make_shared<FixedBitSet>(maxDoc)),
      maxPointsInLeafNode(maxPointsInLeafNode),
      maxPointsSortInHeap(static_cast<int>(
          0.5 * (maxMBSortInHeap * 1024 * 1024) / (bytesPerDoc * numDims))),
      minPackedValue(std::deque<char>(packedBytesLength)),
      maxPackedValue(std::deque<char>(packedBytesLength)), longOrds(longOrds),
      totalPointCount(totalPointCount), singleValuePerDoc(singleValuePerDoc),
      offlineSorterBufferMB(
          OfflineSorter::BufferSize::megabytes(offlineSorterBufferMB)),
      offlineSorterMaxTempFiles(offlineSorterMaxTempFiles), maxDoc(maxDoc)
{
  verifyParams(numDims, maxPointsInLeafNode, maxMBSortInHeap, totalPointCount);
  // We use tracking dir to deal with removing files on exception, so each place
  // that creates temp files doesn't need crazy try/finally/sucess logic:

  // If we may have more than 1+Integer.MAX_VALUE values, then we must encode
  // ords with long (8 bytes), else we can use int (4 bytes).

  // dimensional values (numDims * bytesPerDim) + ord (int or long) + docID
  // (int)
  if (singleValuePerDoc) {
    // Lucene only supports up to 2.1 docs, so we better not need longOrds in
    // this case:
    assert(longOrds == false);
    bytesPerDoc = packedBytesLength + Integer::BYTES;
  } else if (longOrds) {
    bytesPerDoc = packedBytesLength + Long::BYTES + Integer::BYTES;
  } else {
    bytesPerDoc = packedBytesLength + Integer::BYTES + Integer::BYTES;
  }

  // As we recurse, we compute temporary partitions of the data, halving the
  // number of points at each recursion.  Once there are few enough points,
  // we can switch to sorting in heap instead of offline (on disk).  At any
  // time in the recursion, we hold the number of points at that level, plus
  // all recursive halves (i.e. 16 + 8 + 4 + 2) so the memory usage is 2X
  // what that level would consume, so we multiply by 0.5 to convert from
  // bytes to points here.  Each dimension has its own sorted partition, so
  // we must divide by numDims as wel.

  // Finally, we must be able to hold at least the leaf node in heap during
  // build:
  if (maxPointsSortInHeap < maxPointsInLeafNode) {
    throw invalid_argument(
        L"maxMBSortInHeap=" + to_wstring(maxMBSortInHeap) +
        L" only allows for maxPointsSortInHeap=" +
        to_wstring(maxPointsSortInHeap) +
        L", but this is less than maxPointsInLeafNode=" +
        to_wstring(maxPointsInLeafNode) +
        L"; either increase maxMBSortInHeap or decrease maxPointsInLeafNode");
  }

  // We write first maxPointsSortInHeap in heap, then cutover to offline for
  // additional points:
  heapPointWriter = make_shared<HeapPointWriter>(
      16, maxPointsSortInHeap, packedBytesLength, longOrds, singleValuePerDoc);
}

void SimpleTextBKDWriter::verifyParams(int numDims, int maxPointsInLeafNode,
                                       double maxMBSortInHeap,
                                       int64_t totalPointCount)
{
  // We encode dim in a single byte in the splitPackedValues, but we only expose
  // 4 bits for it now, in case we want to use remaining 4 bits for another
  // purpose later
  if (numDims < 1 || numDims > MAX_DIMS) {
    throw invalid_argument(L"numDims must be 1 .. " + to_wstring(MAX_DIMS) +
                           L" (got: " + to_wstring(numDims) + L")");
  }
  if (maxPointsInLeafNode <= 0) {
    throw invalid_argument(L"maxPointsInLeafNode must be > 0; got " +
                           to_wstring(maxPointsInLeafNode));
  }
  if (maxPointsInLeafNode > ArrayUtil::MAX_ARRAY_LENGTH) {
    throw invalid_argument(
        L"maxPointsInLeafNode must be <= ArrayUtil.MAX_ARRAY_LENGTH (= " +
        to_wstring(ArrayUtil::MAX_ARRAY_LENGTH) + L"); got " +
        to_wstring(maxPointsInLeafNode));
  }
  if (maxMBSortInHeap < 0.0) {
    throw invalid_argument(L"maxMBSortInHeap must be >= 0.0 (got: " +
                           to_wstring(maxMBSortInHeap) + L")");
  }
  if (totalPointCount < 0) {
    throw invalid_argument(L"totalPointCount must be >=0 (got: " +
                           to_wstring(totalPointCount) + L")");
  }
}

void SimpleTextBKDWriter::spillToOffline() 
{

  // For each .add we just append to this input file, then in .finish we sort
  // this input and resursively build the tree:
  offlinePointWriter = make_shared<OfflinePointWriter>(
      tempDir, tempFileNamePrefix, packedBytesLength, longOrds, L"spill", 0,
      singleValuePerDoc);
  tempInput = offlinePointWriter->out;
  shared_ptr<PointReader> reader = heapPointWriter->getReader(0, pointCount);
  for (int i = 0; i < pointCount; i++) {
    bool hasNext = reader->next();
    assert(hasNext);
    offlinePointWriter->append(reader->packedValue(), i,
                               heapPointWriter->docIDs[i]);
  }

  heapPointWriter.reset();
}

void SimpleTextBKDWriter::add(std::deque<char> &packedValue,
                              int docID) 
{
  if (packedValue.size() != packedBytesLength) {
    throw invalid_argument(L"packedValue should be length=" +
                           to_wstring(packedBytesLength) + L" (got: " +
                           packedValue.size() + L")");
  }

  if (pointCount >= maxPointsSortInHeap) {
    if (offlinePointWriter == nullptr) {
      spillToOffline();
    }
    offlinePointWriter->append(packedValue, pointCount, docID);
  } else {
    // Not too many points added yet, continue using heap:
    heapPointWriter->append(packedValue, pointCount, docID);
  }

  // TODO: we could specialize for the 1D case:
  if (pointCount == 0) {
    System::arraycopy(packedValue, 0, minPackedValue, 0, packedBytesLength);
    System::arraycopy(packedValue, 0, maxPackedValue, 0, packedBytesLength);
  } else {
    for (int dim = 0; dim < numDims; dim++) {
      int offset = dim * bytesPerDim;
      if (StringHelper::compare(bytesPerDim, packedValue, offset,
                                minPackedValue, offset) < 0) {
        System::arraycopy(packedValue, offset, minPackedValue, offset,
                          bytesPerDim);
      }
      if (StringHelper::compare(bytesPerDim, packedValue, offset,
                                maxPackedValue, offset) > 0) {
        System::arraycopy(packedValue, offset, maxPackedValue, offset,
                          bytesPerDim);
      }
    }
  }

  pointCount++;
  if (pointCount > totalPointCount) {
    throw make_shared<IllegalStateException>(
        L"totalPointCount=" + to_wstring(totalPointCount) +
        L" was passed when we were created, but we just hit " +
        to_wstring(pointCount) + L" values");
  }
  docsSeen->set(docID);
}

int64_t SimpleTextBKDWriter::getPointCount() { return pointCount; }

SimpleTextBKDWriter::MergeReader::MergeReader(
    shared_ptr<SimpleTextBKDReader> bkd,
    shared_ptr<MergeState::DocMap> docMap) 
    : bkd(bkd), state(make_shared<SimpleTextBKDReader::IntersectState>(
                    bkd->in_->clone(), bkd->numDims, bkd->packedBytesLength,
                    bkd->maxPointsInLeafNode, nullptr)),
      docMap(docMap), packedValues(std::deque<char>(bkd->maxPointsInLeafNode *
                                                     bkd->packedBytesLength))
{
  int64_t minFP = numeric_limits<int64_t>::max();
  // System.out.println("MR.init " + this + " bkdreader=" + bkd + "
  // leafBlockFPs.length=" + bkd.leafBlockFPs.length);
  for (auto fp : bkd->leafBlockFPs) {
    minFP = min(minFP, fp);
    // System.out.println("  leaf fp=" + fp);
  }
  state->in_->seek(minFP);
}

bool SimpleTextBKDWriter::MergeReader::next() 
{
  // System.out.println("MR.next this=" + this);
  while (true) {
    if (docBlockUpto == docsInBlock) {
      if (blockID == bkd->leafBlockFPs.size()) {
        // System.out.println("  done!");
        return false;
      }
      // System.out.println("  new block @ fp=" + state.in.getFilePointer());
      docsInBlock = bkd->readDocIDs(state->in_, state->in_->getFilePointer(),
                                    state->scratchDocIDs);
      assert(docsInBlock > 0);
      docBlockUpto = 0;
      bkd->visitDocValues(
          state->commonPrefixLengths, state->scratchPackedValue, state->in_,
          state->scratchDocIDs, docsInBlock,
          make_shared<IntersectVisitorAnonymousInnerClass>(shared_from_this()));

      blockID++;
    }

    constexpr int index = docBlockUpto++;
    int oldDocID = state->scratchDocIDs[index];

    int mappedDocID;
    if (docMap == nullptr) {
      mappedDocID = oldDocID;
    } else {
      mappedDocID = docMap->get(oldDocID);
    }

    if (mappedDocID != -1) {
      // Not deleted!
      docID = mappedDocID;
      System::arraycopy(packedValues, index * bkd->packedBytesLength,
                        state->scratchPackedValue, 0, bkd->packedBytesLength);
      return true;
    }
  }
}

SimpleTextBKDWriter::MergeReader::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<BKDWriter::MergeReader> outerInstance)
{
  this->outerInstance = outerInstance;
  i = 0;
}

void SimpleTextBKDWriter::MergeReader::IntersectVisitorAnonymousInnerClass::
    visit(int docID) 
{
  throw make_shared<UnsupportedOperationException>();
}

void SimpleTextBKDWriter::MergeReader::IntersectVisitorAnonymousInnerClass::
    visit(int docID, std::deque<char> &packedValue) 
{
  assert(docID == outerInstance->state->scratchDocIDs[i]);
  System::arraycopy(packedValue, 0, outerInstance->packedValues,
                    i * outerInstance->bkd->packedBytesLength,
                    outerInstance->bkd->packedBytesLength);
  i++;
}

Relation
SimpleTextBKDWriter::MergeReader::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  throw make_shared<UnsupportedOperationException>();
}

SimpleTextBKDWriter::BKDMergeQueue::BKDMergeQueue(int bytesPerDim, int maxSize)
    : org::apache::lucene::util::PriorityQueue<MergeReader>(maxSize),
      bytesPerDim(bytesPerDim)
{
}

bool SimpleTextBKDWriter::BKDMergeQueue::lessThan(
    shared_ptr<BKDWriter::MergeReader> a, shared_ptr<BKDWriter::MergeReader> b)
{
  assert(a != b);

  int cmp = StringHelper::compare(bytesPerDim, a->state->scratchPackedValue, 0,
                                  b->state->scratchPackedValue, 0);
  if (cmp < 0) {
    return true;
  } else if (cmp > 0) {
    return false;
  }

  // Tie break by sorting smaller docIDs earlier:
  return a->docID < b->docID;
}

int64_t SimpleTextBKDWriter::writeField(
    shared_ptr<IndexOutput> out, const wstring &fieldName,
    shared_ptr<MutablePointValues> reader) 
{
  if (numDims == 1) {
    return writeField1Dim(out, fieldName, reader);
  } else {
    return writeFieldNDims(out, fieldName, reader);
  }
}

int64_t SimpleTextBKDWriter::writeFieldNDims(
    shared_ptr<IndexOutput> out, const wstring &fieldName,
    shared_ptr<MutablePointValues> values) 
{
  if (pointCount != 0) {
    throw make_shared<IllegalStateException>(L"cannot mix add and writeField");
  }

  // Catch user silliness:
  if (heapPointWriter == nullptr && tempInput == nullptr) {
    throw make_shared<IllegalStateException>(L"already finished");
  }

  // Mark that we already finished:
  heapPointWriter.reset();

  int64_t countPerLeaf = pointCount = values->size();
  int64_t innerNodeCount = 1;

  while (countPerLeaf > maxPointsInLeafNode) {
    countPerLeaf = (countPerLeaf + 1) / 2;
    innerNodeCount *= 2;
  }

  int numLeaves = Math::toIntExact(innerNodeCount);

  checkMaxLeafNodeCount(numLeaves);

  const std::deque<char> splitPackedValues =
      std::deque<char>(numLeaves * (bytesPerDim + 1));
  const std::deque<int64_t> leafBlockFPs = std::deque<int64_t>(numLeaves);

  // compute the min/max for this slice
  Arrays::fill(minPackedValue, static_cast<char>(0xff));
  Arrays::fill(maxPackedValue, static_cast<char>(0));
  for (int i = 0; i < Math::toIntExact(pointCount); ++i) {
    values->getValue(i, scratchBytesRef1);
    for (int dim = 0; dim < numDims; dim++) {
      int offset = dim * bytesPerDim;
      if (StringHelper::compare(bytesPerDim, scratchBytesRef1->bytes,
                                scratchBytesRef1->offset + offset,
                                minPackedValue, offset) < 0) {
        System::arraycopy(scratchBytesRef1->bytes,
                          scratchBytesRef1->offset + offset, minPackedValue,
                          offset, bytesPerDim);
      }
      if (StringHelper::compare(bytesPerDim, scratchBytesRef1->bytes,
                                scratchBytesRef1->offset + offset,
                                maxPackedValue, offset) > 0) {
        System::arraycopy(scratchBytesRef1->bytes,
                          scratchBytesRef1->offset + offset, maxPackedValue,
                          offset, bytesPerDim);
      }
    }

    docsSeen->set(values->getDocID(i));
  }

  build(1, numLeaves, values, 0, Math::toIntExact(pointCount), out,
        minPackedValue, maxPackedValue, splitPackedValues, leafBlockFPs,
        std::deque<int>(maxPointsInLeafNode));

  int64_t indexFP = out->getFilePointer();
  writeIndex(out, leafBlockFPs, splitPackedValues);
  return indexFP;
}

int64_t SimpleTextBKDWriter::writeField1Dim(
    shared_ptr<IndexOutput> out, const wstring &fieldName,
    shared_ptr<MutablePointValues> reader) 
{
  MutablePointsReaderUtils::sort(maxDoc, packedBytesLength, reader, 0,
                                 Math::toIntExact(reader->size()));

  shared_ptr<BKDWriter::OneDimensionBKDWriter> *const oneDimWriter =
      make_shared<BKDWriter::OneDimensionBKDWriter>(out);

  reader->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
      shared_from_this(), oneDimWriter));

  return oneDimWriter->finish();
}

SimpleTextBKDWriter::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<SimpleTextBKDWriter> outerInstance,
        shared_ptr<BKDWriter::OneDimensionBKDWriter> oneDimWriter)
{
  this->outerInstance = outerInstance;
  this->oneDimWriter = oneDimWriter;
}

void SimpleTextBKDWriter::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue) 
{
  oneDimWriter->add(packedValue, docID);
}

void SimpleTextBKDWriter::IntersectVisitorAnonymousInnerClass::visit(
    int docID) 
{
  throw make_shared<IllegalStateException>();
}

Relation SimpleTextBKDWriter::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return Relation::CELL_CROSSES_QUERY;
}

int64_t SimpleTextBKDWriter::merge(
    shared_ptr<IndexOutput> out,
    deque<std::shared_ptr<MergeState::DocMap>> &docMaps,
    deque<std::shared_ptr<SimpleTextBKDReader>> &readers) 
{
  assert(docMaps.empty() || readers.size() == docMaps.size());

  shared_ptr<BKDWriter::BKDMergeQueue> queue =
      make_shared<BKDWriter::BKDMergeQueue>(bytesPerDim, readers.size());

  for (int i = 0; i < readers.size(); i++) {
    shared_ptr<SimpleTextBKDReader> bkd = readers[i];
    shared_ptr<MergeState::DocMap> docMap;
    if (docMaps.empty()) {
      docMap.reset();
    } else {
      docMap = docMaps[i];
    }
    shared_ptr<BKDWriter::MergeReader> reader =
        make_shared<BKDWriter::MergeReader>(bkd, docMap);
    if (reader->next()) {
      queue->push_back(reader);
    }
  }

  shared_ptr<BKDWriter::OneDimensionBKDWriter> oneDimWriter =
      make_shared<BKDWriter::OneDimensionBKDWriter>(out);

  while (queue->size() != 0) {
    shared_ptr<BKDWriter::MergeReader> reader = queue->top();
    // System.out.println("iter reader=" + reader);

    // NOTE: doesn't work with subclasses (e.g. SimpleText!)
    oneDimWriter->add(reader->state->scratchPackedValue, reader->docID);

    if (reader->next()) {
      queue->updateTop();
    } else {
      // This segment was exhausted
      queue->pop();
    }
  }

  return oneDimWriter->finish();
}

SimpleTextBKDWriter::OneDimensionBKDWriter::OneDimensionBKDWriter(
    shared_ptr<SimpleTextBKDWriter> outerInstance, shared_ptr<IndexOutput> out)
    : out(out),
      lastPackedValue(std::deque<char>(outerInstance->packedBytesLength)),
      outerInstance(outerInstance)
{
  if (outerInstance->numDims != 1) {
    throw make_shared<UnsupportedOperationException>(
        L"numDims must be 1 but got " + to_wstring(outerInstance->numDims));
  }
  if (outerInstance->pointCount != 0) {
    throw make_shared<IllegalStateException>(L"cannot mix add and merge");
  }

  // Catch user silliness:
  if (outerInstance->heapPointWriter == nullptr &&
      outerInstance->tempInput == nullptr) {
    throw make_shared<IllegalStateException>(L"already finished");
  }

  // Mark that we already finished:
  outerInstance->heapPointWriter.reset();
}

void SimpleTextBKDWriter::OneDimensionBKDWriter::add(
    std::deque<char> &packedValue, int docID) 
{
  assert(
      (outerInstance->valueInOrder(valueCount + leafCount, 0, lastPackedValue,
                                   packedValue, 0, docID, lastDocID)));

  System::arraycopy(packedValue, 0, leafValues,
                    leafCount * outerInstance->packedBytesLength,
                    outerInstance->packedBytesLength);
  leafDocs[leafCount] = docID;
  outerInstance->docsSeen->set(docID);
  leafCount++;

  if (valueCount > outerInstance->totalPointCount) {
    throw make_shared<IllegalStateException>(
        L"totalPointCount=" + to_wstring(outerInstance->totalPointCount) +
        L" was passed when we were created, but we just hit " +
        to_wstring(outerInstance->pointCount) + L" values");
  }

  if (leafCount == outerInstance->maxPointsInLeafNode) {
    // We write a block once we hit exactly the max count ... this is different
    // from when we flush a new segment, where we write between max/2 and max
    // per leaf block, so merged segments will behave differently from newly
    // flushed segments:
    writeLeafBlock();
    leafCount = 0;
  }

  assert(lastDocID = docID) >= 0; // only assign when asserts are enabled
}

int64_t
SimpleTextBKDWriter::OneDimensionBKDWriter::finish() 
{
  if (leafCount > 0) {
    writeLeafBlock();
    leafCount = 0;
  }

  if (valueCount == 0) {
    return -1;
  }

  outerInstance->pointCount = valueCount;

  int64_t indexFP = out->getFilePointer();

  int numInnerNodes = leafBlockStartValues.size();

  // System.out.println("BKDW: now rotate numInnerNodes=" + numInnerNodes + "
  // leafBlockStarts=" + leafBlockStartValues.size());

  std::deque<char> index((1 + numInnerNodes) *
                          (1 + outerInstance->bytesPerDim));
  outerInstance->rotateToTree(1, 0, numInnerNodes, index, leafBlockStartValues);
  std::deque<int64_t> arr(leafBlockFPs.size());
  for (int i = 0; i < leafBlockFPs.size(); i++) {
    arr[i] = leafBlockFPs[i];
  }
  outerInstance->writeIndex(out, arr, index);
  return indexFP;
}

void SimpleTextBKDWriter::OneDimensionBKDWriter::writeLeafBlock() throw(
    IOException)
{
  assert(leafCount != 0);
  if (valueCount == 0) {
    System::arraycopy(leafValues, 0, outerInstance->minPackedValue, 0,
                      outerInstance->packedBytesLength);
  }
  System::arraycopy(
      leafValues, (leafCount - 1) * outerInstance->packedBytesLength,
      outerInstance->maxPackedValue, 0, outerInstance->packedBytesLength);

  valueCount += leafCount;

  if (leafBlockFPs.size() > 0) {
    // Save the first (minimum) value in each leaf block except the first, to
    // build the split value index in the end:
    leafBlockStartValues.push_back(
        Arrays::copyOf(leafValues, outerInstance->packedBytesLength));
  }
  leafBlockFPs.push_back(out->getFilePointer());
  outerInstance->checkMaxLeafNodeCount(leafBlockFPs.size());

  Arrays::fill(outerInstance->commonPrefixLengths, outerInstance->bytesPerDim);
  // Find per-dim common prefix:
  for (int dim = 0; dim < outerInstance->numDims; dim++) {
    int offset1 = dim * outerInstance->bytesPerDim;
    int offset2 = (leafCount - 1) * outerInstance->packedBytesLength + offset1;
    for (int j = 0; j < outerInstance->commonPrefixLengths[dim]; j++) {
      if (leafValues[offset1 + j] != leafValues[offset2 + j]) {
        outerInstance->commonPrefixLengths[dim] = j;
        break;
      }
    }
  }

  outerInstance->writeLeafBlockDocs(out, leafDocs, 0, leafCount);

  const function<BytesRef *(int)> packedValues =
      make_shared<functionAnonymousInnerClass>(shared_from_this(), int);
  assert((outerInstance->valuesInOrderAndBounds(
      leafCount, 0,
      Arrays::copyOf(leafValues, outerInstance->packedBytesLength),
      Arrays::copyOfRange(leafValues,
                          (leafCount - 1) * outerInstance->packedBytesLength,
                          leafCount * outerInstance->packedBytesLength),
      packedValues, leafDocs, 0)));
  outerInstance->writeLeafBlockPackedValues(
      out, outerInstance->commonPrefixLengths, leafCount, 0, packedValues);
}

SimpleTextBKDWriter::OneDimensionBKDWriter::functionAnonymousInnerClass::
    functionAnonymousInnerClass(
        shared_ptr<BKDWriter::OneDimensionBKDWriter> outerInstance,
        shared_ptr<UnknownType> int)
    : function<org::apache::lucene::util::BytesRef *(int)>(int)
{
  this->outerInstance = outerInstance;
  outerInstance->outerInstance->scratch = make_shared<BytesRef>();

  outerInstance->outerInstance.scratch->length =
      outerInstance->outerInstance.packedBytesLength;
  outerInstance->outerInstance.scratch->bytes = outerInstance->leafValues;
}

shared_ptr<BytesRef>
SimpleTextBKDWriter::OneDimensionBKDWriter::functionAnonymousInnerClass::apply(
    int i)
{
  outerInstance->outerInstance->scratch->offset =
      outerInstance->outerInstance->packedBytesLength * i;
  return outerInstance->outerInstance->scratch;
}

void SimpleTextBKDWriter::rotateToTree(
    int nodeID, int offset, int count, std::deque<char> &index,
    deque<std::deque<char>> &leafBlockStartValues)
{
  // System.out.println("ROTATE: nodeID=" + nodeID + " offset=" + offset + "
  // count=" + count + " bpd=" + bytesPerDim + " index.length=" + index.length);
  if (count == 1) {
    // Leaf index node
    // System.out.println("  leaf index node");
    // System.out.println("  index[" + nodeID + "] = blockStartValues[" + offset
    // + "]");
    System::arraycopy(leafBlockStartValues[offset], 0, index,
                      nodeID * (1 + bytesPerDim) + 1, bytesPerDim);
  } else if (count > 1) {
    // Internal index node: binary partition of count
    int countAtLevel = 1;
    int totalCount = 0;
    while (true) {
      int countLeft = count - totalCount;
      // System.out.println("    cycle countLeft=" + countLeft + " coutAtLevel="
      // + countAtLevel);
      if (countLeft <= countAtLevel) {
        // This is the last level, possibly partially filled:
        int lastLeftCount = min(countAtLevel / 2, countLeft);
        assert(lastLeftCount >= 0);
        int leftHalf = (totalCount - 1) / 2 + lastLeftCount;

        int rootOffset = offset + leftHalf;
        /*
        System.out.println("  last left count " + lastLeftCount);
        System.out.println("  leftHalf " + leftHalf + " rightHalf=" +
        (count-leftHalf-1)); System.out.println("  rootOffset=" + rootOffset);
        */

        System::arraycopy(leafBlockStartValues[rootOffset], 0, index,
                          nodeID * (1 + bytesPerDim) + 1, bytesPerDim);
        // System.out.println("  index[" + nodeID + "] = blockStartValues[" +
        // rootOffset + "]");

        // TODO: we could optimize/specialize, when we know it's simply fully
        // balanced binary tree under here, to save this while loop on each
        // recursion

        // Recurse left
        rotateToTree(2 * nodeID, offset, leftHalf, index, leafBlockStartValues);

        // Recurse right
        rotateToTree(2 * nodeID + 1, rootOffset + 1, count - leftHalf - 1,
                     index, leafBlockStartValues);
        return;
      }
      totalCount += countAtLevel;
      countAtLevel *= 2;
    }
  } else {
    assert(count == 0);
  }
}

void SimpleTextBKDWriter::sortHeapPointWriter(
    shared_ptr<HeapPointWriter> writer, int dim)
{
  constexpr int pointCount = Math::toIntExact(this->pointCount);
  // Tie-break by docID:

  // No need to tie break on ord, for the case where the same doc has the same
  // value in a given dimension indexed more than once: it can't matter at
  // search time since we don't write ords into the index:
  make_shared<MSBRadixSorterAnonymousInnerClass>(
      shared_from_this(), bytesPerDim + Integer::BYTES, writer, dim)
      .sort(0, pointCount);
}

SimpleTextBKDWriter::MSBRadixSorterAnonymousInnerClass::
    MSBRadixSorterAnonymousInnerClass(
        shared_ptr<SimpleTextBKDWriter> outerInstance,
        shared_ptr<UnknownType> BYTES, shared_ptr<HeapPointWriter> writer,
        int dim)
    : org::apache::lucene::util::MSBRadixSorter(BYTES)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
  this->dim = dim;
}

int SimpleTextBKDWriter::MSBRadixSorterAnonymousInnerClass::byteAt(int i, int k)
{
  assert(k >= 0);
  if (k < outerInstance->bytesPerDim) {
    // dim bytes
    int block = i / writer->valuesPerBlock;
    int index = i % writer->valuesPerBlock;
    return writer->blocks[block][index * outerInstance->packedBytesLength +
                                 dim * outerInstance->bytesPerDim + k] &
           0xff;
  } else {
    // doc id
    int s = 3 - (k - outerInstance->bytesPerDim);
    return (static_cast<int>(static_cast<unsigned int>(writer->docIDs[i]) >>
                             (s * 8))) &
           0xff;
  }
}

void SimpleTextBKDWriter::MSBRadixSorterAnonymousInnerClass::swap(int i, int j)
{
  int docID = writer->docIDs[i];
  writer->docIDs[i] = writer->docIDs[j];
  writer->docIDs[j] = docID;

  if (outerInstance->singleValuePerDoc == false) {
    if (outerInstance->longOrds) {
      int64_t ord = writer->ordsLong[i];
      writer->ordsLong[i] = writer->ordsLong[j];
      writer->ordsLong[j] = ord;
    } else {
      int ord = writer->ords[i];
      writer->ords[i] = writer->ords[j];
      writer->ords[j] = ord;
    }
  }

  std::deque<char> blockI = writer->blocks[i / writer->valuesPerBlock];
  int indexI = (i % writer->valuesPerBlock) * outerInstance->packedBytesLength;
  std::deque<char> blockJ = writer->blocks[j / writer->valuesPerBlock];
  int indexJ = (j % writer->valuesPerBlock) * outerInstance->packedBytesLength;

  // scratch1 = values[i]
  System::arraycopy(blockI, indexI, outerInstance->scratch1, 0,
                    outerInstance->packedBytesLength);
  // values[i] = values[j]
  System::arraycopy(blockJ, indexJ, blockI, indexI,
                    outerInstance->packedBytesLength);
  // values[j] = scratch1
  System::arraycopy(outerInstance->scratch1, 0, blockJ, indexJ,
                    outerInstance->packedBytesLength);
}

shared_ptr<PointWriter> SimpleTextBKDWriter::sort(int dim) 
{
  assert(dim >= 0 && dim < numDims);

  if (heapPointWriter != nullptr) {

    assert(tempInput == nullptr);

    // We never spilled the incoming points to disk, so now we sort in heap:
    shared_ptr<HeapPointWriter> sorted;

    if (dim == 0) {
      // First dim can re-use the current heap writer
      sorted = heapPointWriter;
    } else {
      // Subsequent dims need a private copy
      sorted = make_shared<HeapPointWriter>(
          static_cast<int>(pointCount), static_cast<int>(pointCount),
          packedBytesLength, longOrds, singleValuePerDoc);
      sorted->copyFrom(heapPointWriter);
    }

    // long t0 = System.nanoTime();
    sortHeapPointWriter(sorted, dim);
    // long t1 = System.nanoTime();
    // System.out.println("BKD: sort took " + ((t1-t0)/1000000.0) + " msec");

    delete sorted;
    return sorted;
  } else {

    // Offline sort:
    assert(tempInput != nullptr);

    constexpr int offset = bytesPerDim * dim;

    shared_ptr<Comparator<std::shared_ptr<BytesRef>>> cmp;
    if (dim == numDims - 1) {
      // in that case the bytes for the dimension and for the doc id are
      // contiguous, so we don't need a branch
      cmp = make_shared<BytesRefComparatorAnonymousInnerClass>(
          shared_from_this(), bytesPerDim + Integer::BYTES, offset);
    } else {
      cmp = make_shared<BytesRefComparatorAnonymousInnerClass2>(
          shared_from_this(), bytesPerDim + Integer::BYTES, offset);
    }

    shared_ptr<OfflineSorter> sorter =
        make_shared<OfflineSorterAnonymousInnerClass>(
            shared_from_this(), tempDir,
            tempFileNamePrefix + L"_bkd" + to_wstring(dim), cmp,
            offlineSorterBufferMB, offlineSorterMaxTempFiles, bytesPerDoc);

    wstring name = sorter->sort(tempInput->getName());

    return make_shared<OfflinePointWriter>(tempDir, name, packedBytesLength,
                                           pointCount, longOrds,
                                           singleValuePerDoc);
  }
}

SimpleTextBKDWriter::BytesRefComparatorAnonymousInnerClass::
    BytesRefComparatorAnonymousInnerClass(
        shared_ptr<SimpleTextBKDWriter> outerInstance,
        shared_ptr<UnknownType> BYTES, int offset)
    : org::apache::lucene::util::BytesRefComparator(BYTES)
{
  this->outerInstance = outerInstance;
  this->offset = offset;
}

int SimpleTextBKDWriter::BytesRefComparatorAnonymousInnerClass::byteAt(
    shared_ptr<BytesRef> ref, int i)
{
  return ref->bytes[ref->offset + offset + i] & 0xff;
}

SimpleTextBKDWriter::BytesRefComparatorAnonymousInnerClass2::
    BytesRefComparatorAnonymousInnerClass2(
        shared_ptr<SimpleTextBKDWriter> outerInstance,
        shared_ptr<UnknownType> BYTES, int offset)
    : org::apache::lucene::util::BytesRefComparator(BYTES)
{
  this->outerInstance = outerInstance;
  this->offset = offset;
}

int SimpleTextBKDWriter::BytesRefComparatorAnonymousInnerClass2::byteAt(
    shared_ptr<BytesRef> ref, int i)
{
  if (i < outerInstance->bytesPerDim) {
    return ref->bytes[ref->offset + offset + i] & 0xff;
  } else {
    return ref->bytes[ref->offset + outerInstance->packedBytesLength + i -
                      outerInstance->bytesPerDim] &
           0xff;
  }
}

SimpleTextBKDWriter::OfflineSorterAnonymousInnerClass::
    OfflineSorterAnonymousInnerClass(
        shared_ptr<SimpleTextBKDWriter> outerInstance,
        shared_ptr<TrackingDirectoryWrapper> tempDir,
        const wstring &tempFileNamePrefix,
        shared_ptr<Comparator<std::shared_ptr<BytesRef>>> cmp,
        shared_ptr<OfflineSorter::BufferSize> offlineSorterBufferMB,
        int offlineSorterMaxTempFiles, int bytesPerDoc)
    : org::apache::lucene::util::OfflineSorter(
          tempDir, tempFileNamePrefix + L"_bkd" + dim, cmp,
          offlineSorterBufferMB, offlineSorterMaxTempFiles, bytesPerDoc,
          nullptr, 0)
{
  this->outerInstance = outerInstance;
}

shared_ptr<OfflineSorter::ByteSequencesWriter>
SimpleTextBKDWriter::OfflineSorterAnonymousInnerClass::getWriter(
    shared_ptr<IndexOutput> out, int64_t count)
{
  return make_shared<ByteSequencesWriterAnonymousInnerClass>(shared_from_this(),
                                                             out);
}

SimpleTextBKDWriter::OfflineSorterAnonymousInnerClass::
    ByteSequencesWriterAnonymousInnerClass::
        ByteSequencesWriterAnonymousInnerClass(
            shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance,
            shared_ptr<IndexOutput> out)
    : ByteSequencesWriter(out)
{
  this->outerInstance = outerInstance;
  this->out = out;
}

void SimpleTextBKDWriter::OfflineSorterAnonymousInnerClass::
    ByteSequencesWriterAnonymousInnerClass::write(std::deque<char> &bytes,
                                                  int off,
                                                  int len) 
{
  assert((len == outerInstance->outerInstance.bytesPerDoc,
          L"len=" + to_wstring(len) + L" bytesPerDoc=" +
              outerInstance->outerInstance.bytesPerDoc));
  out->writeBytes(bytes, off, len);
}

shared_ptr<OfflineSorter::ByteSequencesReader>
SimpleTextBKDWriter::OfflineSorterAnonymousInnerClass::getReader(
    shared_ptr<ChecksumIndexInput> in_, const wstring &name) 
{
  return make_shared<ByteSequencesReaderAnonymousInnerClass>(shared_from_this(),
                                                             in_, name);
}

SimpleTextBKDWriter::OfflineSorterAnonymousInnerClass::
    ByteSequencesReaderAnonymousInnerClass::
        ByteSequencesReaderAnonymousInnerClass(
            shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance,
            shared_ptr<ChecksumIndexInput> in_, const wstring &name)
    : ByteSequencesReader(in_, name)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
  outerInstance->outerInstance->scratch = make_shared<BytesRef>(
      std::deque<char>(outerInstance->outerInstance.bytesPerDoc));
}

shared_ptr<BytesRef> SimpleTextBKDWriter::OfflineSorterAnonymousInnerClass::
    ByteSequencesReaderAnonymousInnerClass::next() 
{
  if (in_->getFilePointer() >= end) {
    return nullptr;
  }
  in_->readBytes(outerInstance->outerInstance.scratch.bytes, 0,
                 outerInstance->outerInstance.bytesPerDoc);
  return outerInstance->outerInstance.scratch;
}

void SimpleTextBKDWriter::checkMaxLeafNodeCount(int numLeaves)
{
  if ((1 + bytesPerDim) * static_cast<int64_t>(numLeaves) >
      ArrayUtil::MAX_ARRAY_LENGTH) {
    throw make_shared<IllegalStateException>(
        L"too many nodes; increase maxPointsInLeafNode (currently " +
        to_wstring(maxPointsInLeafNode) + L") and reindex");
  }
}

int64_t
SimpleTextBKDWriter::finish(shared_ptr<IndexOutput> out) 
{
  // System.out.println("\nBKDTreeWriter.finish pointCount=" + pointCount + "
  // out=" + out + " heapWriter=" + heapPointWriter);

  // TODO: specialize the 1D case?  it's much faster at indexing time (no
  // partitioning on recurse...)

  // Catch user silliness:
  if (heapPointWriter == nullptr && tempInput == nullptr) {
    throw make_shared<IllegalStateException>(L"already finished");
  }

  if (offlinePointWriter != nullptr) {
    delete offlinePointWriter;
  }

  if (pointCount == 0) {
    throw make_shared<IllegalStateException>(L"must index at least one point");
  }

  shared_ptr<LongBitSet> ordBitSet;
  if (numDims > 1) {
    if (singleValuePerDoc) {
      ordBitSet = make_shared<LongBitSet>(maxDoc);
    } else {
      ordBitSet = make_shared<LongBitSet>(pointCount);
    }
  } else {
    ordBitSet.reset();
  }

  int64_t countPerLeaf = pointCount;
  int64_t innerNodeCount = 1;

  while (countPerLeaf > maxPointsInLeafNode) {
    countPerLeaf = (countPerLeaf + 1) / 2;
    innerNodeCount *= 2;
  }

  int numLeaves = static_cast<int>(innerNodeCount);

  checkMaxLeafNodeCount(numLeaves);

  // NOTE: we could save the 1+ here, to use a bit less heap at search time, but
  // then we'd need a somewhat costly check at each step of the recursion to
  // recompute the split dim:

  // Indexed by nodeID, but first (root) nodeID is 1.  We do 1+ because the lead
  // byte at each recursion says which dim we split on.
  std::deque<char> splitPackedValues(
      Math::toIntExact(numLeaves * (1 + bytesPerDim)));

  // +1 because leaf count is power of 2 (e.g. 8), and innerNodeCount is power
  // of 2 minus 1 (e.g. 7)
  std::deque<int64_t> leafBlockFPs(numLeaves);

  // Make sure the math above "worked":
  assert((pointCount / numLeaves <= maxPointsInLeafNode,
          L"pointCount=" + to_wstring(pointCount) + L" numLeaves=" +
              to_wstring(numLeaves) + L" maxPointsInLeafNode=" +
              to_wstring(maxPointsInLeafNode)));

  // Sort all docs once by each dimension:
  std::deque<std::shared_ptr<BKDWriter::PathSlice>> sortedPointWriters(
      numDims);

  // This is only used on exception; on normal code paths we close all files we
  // opened:
  deque<std::shared_ptr<Closeable>> toCloseHeroically =
      deque<std::shared_ptr<Closeable>>();

  bool success = false;
  try {
    // long t0 = System.nanoTime();
    for (int dim = 0; dim < numDims; dim++) {
      sortedPointWriters[dim] =
          make_shared<BKDWriter::PathSlice>(sort(dim), 0, pointCount);
    }
    // long t1 = System.nanoTime();
    // System.out.println("sort time: " + ((t1-t0)/1000000.0) + " msec");

    if (tempInput != nullptr) {
      tempDir->deleteFile(tempInput->getName());
      tempInput.reset();
    } else {
      assert(heapPointWriter != nullptr);
      heapPointWriter.reset();
    }

    build(1, numLeaves, sortedPointWriters, ordBitSet, out, minPackedValue,
          maxPackedValue, splitPackedValues, leafBlockFPs, toCloseHeroically);

    for (auto slice : sortedPointWriters) {
      slice->writer->destroy();
    }

    // If no exception, we should have cleaned everything up:
    assert(tempDir->getCreatedFiles()->isEmpty());
    // long t2 = System.nanoTime();
    // System.out.println("write time: " + ((t2-t1)/1000000.0) + " msec");

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      IOUtils::deleteFilesIgnoringExceptions(
          {tempDir, tempDir->getCreatedFiles()});
      IOUtils::closeWhileHandlingException(toCloseHeroically);
    }
  }

  // System.out.println("Total nodes: " + innerNodeCount);

  // Write index:
  int64_t indexFP = out->getFilePointer();
  writeIndex(out, leafBlockFPs, splitPackedValues);
  return indexFP;
}

void SimpleTextBKDWriter::writeIndex(
    shared_ptr<IndexOutput> out, std::deque<int64_t> &leafBlockFPs,
    std::deque<char> &splitPackedValues) 
{
  write(out, NUM_DIMS);
  writeInt(out, numDims);
  newline(out);

  write(out, BYTES_PER_DIM);
  writeInt(out, bytesPerDim);
  newline(out);

  write(out, MAX_LEAF_POINTS);
  writeInt(out, maxPointsInLeafNode);
  newline(out);

  write(out, INDEX_COUNT);
  writeInt(out, leafBlockFPs.size());
  newline(out);

  write(out, MIN_VALUE);
  shared_ptr<BytesRef> br =
      make_shared<BytesRef>(minPackedValue, 0, minPackedValue.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(out, br->toString());
  newline(out);

  write(out, MAX_VALUE);
  br = make_shared<BytesRef>(maxPackedValue, 0, maxPackedValue.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(out, br->toString());
  newline(out);

  write(out, POINT_COUNT);
  writeLong(out, pointCount);
  newline(out);

  write(out, DOC_COUNT);
  writeInt(out, docsSeen->cardinality());
  newline(out);

  for (int i = 0; i < leafBlockFPs.size(); i++) {
    write(out, BLOCK_FP);
    writeLong(out, leafBlockFPs[i]);
    newline(out);
  }

  assert(splitPackedValues.size() % (1 + bytesPerDim)) == 0;
  int count = splitPackedValues.size() / (1 + bytesPerDim);
  assert(count == leafBlockFPs.size());

  write(out, SPLIT_COUNT);
  writeInt(out, count);
  newline(out);

  for (int i = 0; i < count; i++) {
    write(out, SPLIT_DIM);
    writeInt(out, splitPackedValues[i * (1 + bytesPerDim)] & 0xff);
    newline(out);
    write(out, SPLIT_VALUE);
    br = make_shared<BytesRef>(splitPackedValues, 1 + (i * (1 + bytesPerDim)),
                               bytesPerDim);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    write(out, br->toString());
    newline(out);
  }
}

void SimpleTextBKDWriter::writeLeafBlockDocs(shared_ptr<IndexOutput> out,
                                             std::deque<int> &docIDs,
                                             int start,
                                             int count) 
{
  write(out, BLOCK_COUNT);
  writeInt(out, count);
  newline(out);
  for (int i = 0; i < count; i++) {
    write(out, BLOCK_DOC_ID);
    writeInt(out, docIDs[start + i]);
    newline(out);
  }
}

void SimpleTextBKDWriter::writeLeafBlockPackedValues(
    shared_ptr<IndexOutput> out, std::deque<int> &commonPrefixLengths,
    int count, int sortedDim,
    function<BytesRef *(int)> &packedValues) 
{
  for (int i = 0; i < count; ++i) {
    shared_ptr<BytesRef> packedValue = packedValues(i);
    // NOTE: we don't do prefix coding, so we ignore commonPrefixLengths
    write(out, BLOCK_VALUE);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    write(out, packedValue->toString());
    newline(out);
  }
}

void SimpleTextBKDWriter::writeLeafBlockPackedValuesRange(
    shared_ptr<IndexOutput> out, std::deque<int> &commonPrefixLengths,
    int start, int end,
    function<BytesRef *(int)> &packedValues) 
{
  for (int i = start; i < end; ++i) {
    shared_ptr<BytesRef> ref = packedValues(i);
    assert(ref->length == packedBytesLength);

    for (int dim = 0; dim < numDims; dim++) {
      int prefix = commonPrefixLengths[dim];
      out->writeBytes(ref->bytes, ref->offset + dim * bytesPerDim + prefix,
                      bytesPerDim - prefix);
    }
  }
}

int SimpleTextBKDWriter::runLen(function<BytesRef *(int)> &packedValues,
                                int start, int end, int byteOffset)
{
  shared_ptr<BytesRef> first = packedValues(start);
  char b = first->bytes[first->offset + byteOffset];
  for (int i = start + 1; i < end; ++i) {
    shared_ptr<BytesRef> ref = packedValues(i);
    char b2 = ref->bytes[ref->offset + byteOffset];
    assert(Byte::toUnsignedInt(b2) >= Byte::toUnsignedInt(b));
    if (b != b2) {
      return i - start;
    }
  }
  return end - start;
}

SimpleTextBKDWriter::~SimpleTextBKDWriter()
{
  if (tempInput != nullptr) {
    // NOTE: this should only happen on exception, e.g. caller calls close w/o
    // calling finish:
    try {
      delete tempInput;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      tempDir->deleteFile(tempInput->getName());
      tempInput.reset();
    }
  }
}

SimpleTextBKDWriter::PathSlice::PathSlice(shared_ptr<PointWriter> writer,
                                          int64_t start, int64_t count)
    : writer(writer), start(start), count(count)
{
}

wstring SimpleTextBKDWriter::PathSlice::toString()
{
  return L"PathSlice(start=" + to_wstring(start) + L" count=" +
         to_wstring(count) + L" writer=" + writer + L")";
}

shared_ptr<Error> SimpleTextBKDWriter::verifyChecksum(
    runtime_error priorException,
    shared_ptr<PointWriter> writer) 
{
  assert(priorException != nullptr);
  // TODO: we could improve this, to always validate checksum as we recurse, if
  // we shared left and right reader after recursing to children, and possibly
  // within recursed children, since all together they make a single pass
  // through the file.  But this is a sizable re-org, and would mean leaving
  // readers (IndexInputs) open for longer:
  if (std::dynamic_pointer_cast<OfflinePointWriter>(writer) != nullptr) {
    // We are reading from a temp file; go verify the checksum:
    wstring tempFileName =
        (std::static_pointer_cast<OfflinePointWriter>(writer))->name;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
    // in = tempDir.openChecksumInput(tempFileName,
    // org.apache.lucene.store.IOContext.READONCE))
    {
      org::apache::lucene::store::ChecksumIndexInput in_ =
          tempDir->openChecksumInput(
              tempFileName, org::apache::lucene::store::IOContext::READONCE);
      CodecUtil::checkFooter(in_, priorException);
    }
  }

  // We are reading from heap; nothing to add:
  throw IOUtils::rethrowAlways(priorException);
}

std::deque<char> SimpleTextBKDWriter::markRightTree(
    int64_t rightCount, int splitDim, shared_ptr<BKDWriter::PathSlice> source,
    shared_ptr<LongBitSet> ordBitSet) 
{

  // Now we mark ords that fall into the right half, so we can partition on all
  // other dims that are not the split dim:

  // Read the split value, then mark all ords in the right tree (larger than the
  // split value):

  // TODO: find a way to also checksum this reader?  If we changed to
  // markLeftTree, and scanned the final chunk, it could work?
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.util.bkd.PointReader
  // reader = source.writer.getReader(source.start + source.count - rightCount,
  // rightCount))
  {
    org::apache::lucene::util::bkd::PointReader reader =
        source->writer->getReader(source->start + source->count - rightCount,
                                  rightCount);
    try {
      bool result = reader->next();
      assert(result);
      System::arraycopy(reader->packedValue(), splitDim * bytesPerDim, scratch1,
                        0, bytesPerDim);
      if (numDims > 1) {
        assert(ordBitSet->get(reader->ord()) == false);
        ordBitSet->set(reader->ord());
        // Subtract 1 from rightCount because we already did the first value
        // above (so we could record the split value):
        reader->markOrds(rightCount - 1, ordBitSet);
      }
    } catch (const runtime_error &t) {
      throw verifyChecksum(t, source->writer);
    }
  }

  return scratch1;
}

bool SimpleTextBKDWriter::valueInBounds(shared_ptr<BytesRef> packedValue,
                                        std::deque<char> &minPackedValue,
                                        std::deque<char> &maxPackedValue)
{
  for (int dim = 0; dim < numDims; dim++) {
    int offset = bytesPerDim * dim;
    if (StringHelper::compare(bytesPerDim, packedValue->bytes,
                              packedValue->offset + offset, minPackedValue,
                              offset) < 0) {
      return false;
    }
    if (StringHelper::compare(bytesPerDim, packedValue->bytes,
                              packedValue->offset + offset, maxPackedValue,
                              offset) > 0) {
      return false;
    }
  }

  return true;
}

int SimpleTextBKDWriter::split(std::deque<char> &minPackedValue,
                               std::deque<char> &maxPackedValue)
{
  // Find which dim has the largest span so we can split on it:
  int splitDim = -1;
  for (int dim = 0; dim < numDims; dim++) {
    NumericUtils::subtract(bytesPerDim, dim, maxPackedValue, minPackedValue,
                           scratchDiff);
    if (splitDim == -1 ||
        StringHelper::compare(bytesPerDim, scratchDiff, 0, scratch1, 0) > 0) {
      System::arraycopy(scratchDiff, 0, scratch1, 0, bytesPerDim);
      splitDim = dim;
    }
  }

  // System.out.println("SPLIT: " + splitDim);
  return splitDim;
}

shared_ptr<BKDWriter::PathSlice> SimpleTextBKDWriter::switchToHeap(
    shared_ptr<BKDWriter::PathSlice> source,
    deque<std::shared_ptr<Closeable>> &toCloseHeroically) 
{
  int count = Math::toIntExact(source->count);
  // Not inside the try because we don't want to close it here:
  shared_ptr<PointReader> reader = source->writer->getSharedReader(
      source->start, source->count, toCloseHeroically);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.util.bkd.PointWriter
  // writer = new org.apache.lucene.util.bkd.HeapPointWriter(count, count,
  // packedBytesLength, longOrds, singleValuePerDoc))
  {
    org::apache::lucene::util::bkd::PointWriter writer =
        org::apache::lucene::util::bkd::HeapPointWriter(
            count, count, packedBytesLength, longOrds, singleValuePerDoc);
    try {
      for (int i = 0; i < count; i++) {
        bool hasNext = reader->next();
        assert(hasNext);
        writer->append(reader->packedValue(), reader->ord(), reader->docID());
      }
      return make_shared<BKDWriter::PathSlice>(writer, 0, count);
    } catch (const runtime_error &t) {
      throw verifyChecksum(t, source->writer);
    }
  }
}

void SimpleTextBKDWriter::build(
    int nodeID, int leafNodeOffset, shared_ptr<MutablePointValues> reader,
    int from, int to, shared_ptr<IndexOutput> out,
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue,
    std::deque<char> &splitPackedValues, std::deque<int64_t> &leafBlockFPs,
    std::deque<int> &spareDocIds) 
{

  if (nodeID >= leafNodeOffset) {
    // leaf node
    constexpr int count = to - from;
    assert(count <= maxPointsInLeafNode);

    // Compute common prefixes
    Arrays::fill(commonPrefixLengths, bytesPerDim);
    reader->getValue(from, scratchBytesRef1);
    for (int i = from + 1; i < to; ++i) {
      reader->getValue(i, scratchBytesRef2);
      for (int dim = 0; dim < numDims; dim++) {
        constexpr int offset = dim * bytesPerDim;
        for (int j = 0; j < commonPrefixLengths[dim]; j++) {
          if (scratchBytesRef1->bytes[scratchBytesRef1->offset + offset + j] !=
              scratchBytesRef2->bytes[scratchBytesRef2->offset + offset + j]) {
            commonPrefixLengths[dim] = j;
            break;
          }
        }
      }
    }

    // Find the dimension that has the least number of unique bytes at
    // commonPrefixLengths[dim]
    std::deque<std::shared_ptr<FixedBitSet>> usedBytes(numDims);
    for (int dim = 0; dim < numDims; ++dim) {
      if (commonPrefixLengths[dim] < bytesPerDim) {
        usedBytes[dim] = make_shared<FixedBitSet>(256);
      }
    }
    for (int i = from + 1; i < to; ++i) {
      for (int dim = 0; dim < numDims; dim++) {
        if (usedBytes[dim] != nullptr) {
          char b = reader->getByteAt(i, dim * bytesPerDim +
                                            commonPrefixLengths[dim]);
          usedBytes[dim]->set(Byte::toUnsignedInt(b));
        }
      }
    }
    int sortedDim = 0;
    int sortedDimCardinality = numeric_limits<int>::max();
    for (int dim = 0; dim < numDims; ++dim) {
      if (usedBytes[dim] != nullptr) {
        constexpr int cardinality = usedBytes[dim]->cardinality();
        if (cardinality < sortedDimCardinality) {
          sortedDim = dim;
          sortedDimCardinality = cardinality;
        }
      }
    }

    // sort by sortedDim
    MutablePointsReaderUtils::sortByDim(sortedDim, bytesPerDim,
                                        commonPrefixLengths, reader, from, to,
                                        scratchBytesRef1, scratchBytesRef2);

    // Save the block file pointer:
    leafBlockFPs[nodeID - leafNodeOffset] = out->getFilePointer();

    // Write doc IDs
    std::deque<int> docIDs = spareDocIds;
    for (int i = from; i < to; ++i) {
      docIDs[i - from] = reader->getDocID(i);
    }
    writeLeafBlockDocs(out, docIDs, 0, count);

    // Write the common prefixes:
    reader->getValue(from, scratchBytesRef1);
    System::arraycopy(scratchBytesRef1->bytes, scratchBytesRef1->offset,
                      scratch1, 0, packedBytesLength);

    // Write the full values:
    function<BytesRef *(int)> packedValues = [&](int i) {
      reader->getValue(from + i, scratchBytesRef1);
      return scratchBytesRef1;
    };
    assert((valuesInOrderAndBounds(count, sortedDim, minPackedValue,
                                   maxPackedValue, packedValues, docIDs, 0)));
    writeLeafBlockPackedValues(out, commonPrefixLengths, count, sortedDim,
                               packedValues);

  } else {
    // inner node

    // compute the split dimension and partition around it
    constexpr int splitDim = split(minPackedValue, maxPackedValue);
    constexpr int mid =
        static_cast<int>(static_cast<unsigned int>((from + to + 1)) >> 1);

    int commonPrefixLen = bytesPerDim;
    for (int i = 0; i < bytesPerDim; ++i) {
      if (minPackedValue[splitDim * bytesPerDim + i] !=
          maxPackedValue[splitDim * bytesPerDim + i]) {
        commonPrefixLen = i;
        break;
      }
    }
    MutablePointsReaderUtils::partition(maxDoc, splitDim, bytesPerDim,
                                        commonPrefixLen, reader, from, to, mid,
                                        scratchBytesRef1, scratchBytesRef2);

    // set the split value
    constexpr int address = nodeID * (1 + bytesPerDim);
    splitPackedValues[address] = static_cast<char>(splitDim);
    reader->getValue(mid, scratchBytesRef1);
    System::arraycopy(scratchBytesRef1->bytes,
                      scratchBytesRef1->offset + splitDim * bytesPerDim,
                      splitPackedValues, address + 1, bytesPerDim);

    std::deque<char> minSplitPackedValue =
        Arrays::copyOf(minPackedValue, packedBytesLength);
    std::deque<char> maxSplitPackedValue =
        Arrays::copyOf(maxPackedValue, packedBytesLength);
    System::arraycopy(scratchBytesRef1->bytes,
                      scratchBytesRef1->offset + splitDim * bytesPerDim,
                      minSplitPackedValue, splitDim * bytesPerDim, bytesPerDim);
    System::arraycopy(scratchBytesRef1->bytes,
                      scratchBytesRef1->offset + splitDim * bytesPerDim,
                      maxSplitPackedValue, splitDim * bytesPerDim, bytesPerDim);

    // recurse
    build(nodeID * 2, leafNodeOffset, reader, from, mid, out, minPackedValue,
          maxSplitPackedValue, splitPackedValues, leafBlockFPs, spareDocIds);
    build(nodeID * 2 + 1, leafNodeOffset, reader, mid, to, out,
          minSplitPackedValue, maxPackedValue, splitPackedValues, leafBlockFPs,
          spareDocIds);
  }
}

void SimpleTextBKDWriter::build(
    int nodeID, int leafNodeOffset,
    std::deque<std::shared_ptr<BKDWriter::PathSlice>> &slices,
    shared_ptr<LongBitSet> ordBitSet, shared_ptr<IndexOutput> out,
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue,
    std::deque<char> &splitPackedValues, std::deque<int64_t> &leafBlockFPs,
    deque<std::shared_ptr<Closeable>> &toCloseHeroically) 
{

  for (auto slice : slices) {
    assert(slice->count == slices[0]->count);
  }

  if (numDims == 1 &&
      std::dynamic_pointer_cast<OfflinePointWriter>(slices[0]->writer) !=
          nullptr &&
      slices[0]->count <= maxPointsSortInHeap) {
    // Special case for 1D, to cutover to heap once we recurse deeply enough:
    slices[0] = switchToHeap(slices[0], toCloseHeroically);
  }

  if (nodeID >= leafNodeOffset) {

    // Leaf node: write block
    // We can write the block in any order so by default we write it sorted by
    // the dimension that has the least number of unique bytes at
    // commonPrefixLengths[dim], which makes compression more efficient
    int sortedDim = 0;
    int sortedDimCardinality = numeric_limits<int>::max();

    for (int dim = 0; dim < numDims; dim++) {
      if (std::dynamic_pointer_cast<HeapPointWriter>(slices[dim]->writer) !=
          nullptr == false) {
        // Adversarial cases can cause this, e.g. very lopsided data, all equal
        // points, such that we started offline, but then kept splitting only in
        // one dimension, and so never had to rewrite into heap writer
        slices[dim] = switchToHeap(slices[dim], toCloseHeroically);
      }

      shared_ptr<BKDWriter::PathSlice> source = slices[dim];

      shared_ptr<HeapPointWriter> heapSource =
          std::static_pointer_cast<HeapPointWriter>(source->writer);

      // Find common prefix by comparing first and last values, already sorted
      // in this dimension:
      heapSource->readPackedValue(Math::toIntExact(source->start), scratch1);
      heapSource->readPackedValue(
          Math::toIntExact(source->start + source->count - 1), scratch2);

      int offset = dim * bytesPerDim;
      commonPrefixLengths[dim] = bytesPerDim;
      for (int j = 0; j < bytesPerDim; j++) {
        if (scratch1[offset + j] != scratch2[offset + j]) {
          commonPrefixLengths[dim] = j;
          break;
        }
      }

      int prefix = commonPrefixLengths[dim];
      if (prefix < bytesPerDim) {
        int cardinality = 1;
        char previous = scratch1[offset + prefix];
        for (int64_t i = 1; i < source->count; ++i) {
          heapSource->readPackedValue(Math::toIntExact(source->start + i),
                                      scratch2);
          char b = scratch2[offset + prefix];
          assert(Byte::toUnsignedInt(previous) <= Byte::toUnsignedInt(b));
          if (b != previous) {
            cardinality++;
            previous = b;
          }
        }
        assert(cardinality <= 256);
        if (cardinality < sortedDimCardinality) {
          sortedDim = dim;
          sortedDimCardinality = cardinality;
        }
      }
    }

    shared_ptr<BKDWriter::PathSlice> source = slices[sortedDim];

    // We ensured that maxPointsSortInHeap was >= maxPointsInLeafNode, so we
    // better be in heap at this point:
    shared_ptr<HeapPointWriter> heapSource =
        std::static_pointer_cast<HeapPointWriter>(source->writer);

    // Save the block file pointer:
    leafBlockFPs[nodeID - leafNodeOffset] = out->getFilePointer();
    // System.out.println("  write leaf block @ fp=" + out.getFilePointer());

    // Write docIDs first, as their own chunk, so that at intersect time we can
    // add all docIDs w/o loading the values:
    int count = Math::toIntExact(source->count);
    assert((count > 0, L"nodeID=" + to_wstring(nodeID) + L" leafNodeOffset=" +
                           to_wstring(leafNodeOffset)));
    writeLeafBlockDocs(out, heapSource->docIDs, Math::toIntExact(source->start),
                       count);

    // TODO: minor opto: we don't really have to write the actual common
    // prefixes, because BKDReader on recursing can regenerate it for us from
    // the index, much like how terms dict does so from the FST:

    // Write the full values:
    function<BytesRef *(int)> packedValues =
        make_shared<functionAnonymousInnerClass>(shared_from_this(), int,
                                                 source, heapSource);
    assert((valuesInOrderAndBounds(
        count, sortedDim, minPackedValue, maxPackedValue, packedValues,
        heapSource->docIDs, Math::toIntExact(source->start))));
    writeLeafBlockPackedValues(out, commonPrefixLengths, count, sortedDim,
                               packedValues);

  } else {
    // Inner node: partition/recurse

    int splitDim;
    if (numDims > 1) {
      splitDim = split(minPackedValue, maxPackedValue);
    } else {
      splitDim = 0;
    }

    shared_ptr<BKDWriter::PathSlice> source = slices[splitDim];

    assert((nodeID < splitPackedValues.size(), L"nodeID=" + to_wstring(nodeID) +
                                                   L" splitValues.length=" +
                                                   splitPackedValues.size()));

    // How many points will be in the left tree:
    int64_t rightCount = source->count / 2;
    int64_t leftCount = source->count - rightCount;

    std::deque<char> splitValue =
        markRightTree(rightCount, splitDim, source, ordBitSet);
    int address = nodeID * (1 + bytesPerDim);
    splitPackedValues[address] = static_cast<char>(splitDim);
    System::arraycopy(splitValue, 0, splitPackedValues, address + 1,
                      bytesPerDim);

    // Partition all PathSlice that are not the split dim into sorted left and
    // right sets, so we can recurse:

    std::deque<std::shared_ptr<BKDWriter::PathSlice>> leftSlices(numDims);
    std::deque<std::shared_ptr<BKDWriter::PathSlice>> rightSlices(numDims);

    std::deque<char> minSplitPackedValue(packedBytesLength);
    System::arraycopy(minPackedValue, 0, minSplitPackedValue, 0,
                      packedBytesLength);

    std::deque<char> maxSplitPackedValue(packedBytesLength);
    System::arraycopy(maxPackedValue, 0, maxSplitPackedValue, 0,
                      packedBytesLength);

    // When we are on this dim, below, we clear the ordBitSet:
    int dimToClear;
    if (numDims - 1 == splitDim) {
      dimToClear = numDims - 2;
    } else {
      dimToClear = numDims - 1;
    }

    for (int dim = 0; dim < numDims; dim++) {

      if (dim == splitDim) {
        // No need to partition on this dim since it's a simple slice of the
        // incoming already sorted slice, and we will re-use its shared reader
        // when visiting it as we recurse:
        leftSlices[dim] = make_shared<BKDWriter::PathSlice>(
            source->writer, source->start, leftCount);
        rightSlices[dim] = make_shared<BKDWriter::PathSlice>(
            source->writer, source->start + leftCount, rightCount);
        System::arraycopy(splitValue, 0, minSplitPackedValue, dim * bytesPerDim,
                          bytesPerDim);
        System::arraycopy(splitValue, 0, maxSplitPackedValue, dim * bytesPerDim,
                          bytesPerDim);
        continue;
      }

      // Not inside the try because we don't want to close this one now, so that
      // after recursion is done, we will have done a singel full sweep of the
      // file:
      shared_ptr<PointReader> reader = slices[dim]->writer.getSharedReader(
          slices[dim]->start, slices[dim]->count, toCloseHeroically);

      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try
      // (org.apache.lucene.util.bkd.PointWriter leftPointWriter =
      // getPointWriter(leftCount, "left" + dim);
      // org.apache.lucene.util.bkd.PointWriter rightPointWriter =
      // getPointWriter(source.count - leftCount, "right" + dim))
      {
        org::apache::lucene::util::bkd::PointWriter leftPointWriter =
            getPointWriter(leftCount, L"left" + to_wstring(dim));
        org::apache::lucene::util::bkd::PointWriter rightPointWriter =
            getPointWriter(source->count - leftCount,
                           L"right" + to_wstring(dim));
        try {

          int64_t nextRightCount =
              reader->split(source->count, ordBitSet, leftPointWriter,
                            rightPointWriter, dim == dimToClear);
          if (rightCount != nextRightCount) {
            throw make_shared<IllegalStateException>(
                L"wrong number of points in split: expected=" +
                to_wstring(rightCount) + L" but actual=" +
                to_wstring(nextRightCount));
          }

          leftSlices[dim] =
              make_shared<BKDWriter::PathSlice>(leftPointWriter, 0, leftCount);
          rightSlices[dim] = make_shared<BKDWriter::PathSlice>(rightPointWriter,
                                                               0, rightCount);
        } catch (const runtime_error &t) {
          throw verifyChecksum(t, slices[dim]->writer);
        }
      }
    }

    // Recurse on left tree:
    build(2 * nodeID, leafNodeOffset, leftSlices, ordBitSet, out,
          minPackedValue, maxSplitPackedValue, splitPackedValues, leafBlockFPs,
          toCloseHeroically);
    for (int dim = 0; dim < numDims; dim++) {
      // Don't destroy the dim we split on because we just re-used what our
      // caller above gave us for that dim:
      if (dim != splitDim) {
        leftSlices[dim]->writer.destroy();
      }
    }

    // TODO: we could "tail recurse" here?  have our parent discard its refs as
    // we recurse right? Recurse on right tree:
    build(2 * nodeID + 1, leafNodeOffset, rightSlices, ordBitSet, out,
          minSplitPackedValue, maxPackedValue, splitPackedValues, leafBlockFPs,
          toCloseHeroically);
    for (int dim = 0; dim < numDims; dim++) {
      // Don't destroy the dim we split on because we just re-used what our
      // caller above gave us for that dim:
      if (dim != splitDim) {
        rightSlices[dim]->writer.destroy();
      }
    }
  }
}

SimpleTextBKDWriter::functionAnonymousInnerClass::functionAnonymousInnerClass(
    shared_ptr<SimpleTextBKDWriter> outerInstance, shared_ptr<UnknownType> int,
    shared_ptr<BKDWriter::PathSlice> source,
    shared_ptr<HeapPointWriter> heapSource)
    : function<org::apache::lucene::util::BytesRef *(int)>(int)
{
  this->outerInstance = outerInstance;
  this->source = source;
  this->heapSource = heapSource;
  outerInstance->scratch = make_shared<BytesRef>();

  outerInstance->scratch->length = outerInstance->packedBytesLength;
}

shared_ptr<BytesRef>
SimpleTextBKDWriter::functionAnonymousInnerClass::apply(int i)
{
  heapSource->getPackedValueSlice(Math::toIntExact(source->start + i),
                                  outerInstance->scratch);
  return outerInstance->scratch;
}

bool SimpleTextBKDWriter::valuesInOrderAndBounds(
    int count, int sortedDim, std::deque<char> &minPackedValue,
    std::deque<char> &maxPackedValue, function<BytesRef *(int)> &values,
    std::deque<int> &docs, int docsOffset) 
{
  std::deque<char> lastPackedValue(packedBytesLength);
  int lastDoc = -1;
  for (int i = 0; i < count; i++) {
    shared_ptr<BytesRef> packedValue = values(i);
    assert(packedValue->length == packedBytesLength);
    assert((valueInOrder(i, sortedDim, lastPackedValue, packedValue->bytes,
                         packedValue->offset, docs[docsOffset + i], lastDoc)));
    lastDoc = docs[docsOffset + i];

    // Make sure this value does in fact fall within this leaf cell:
    assert((valueInBounds(packedValue, minPackedValue, maxPackedValue)));
  }
  return true;
}

bool SimpleTextBKDWriter::valueInOrder(int64_t ord, int sortedDim,
                                       std::deque<char> &lastPackedValue,
                                       std::deque<char> &packedValue,
                                       int packedValueOffset, int doc,
                                       int lastDoc)
{
  int dimOffset = sortedDim * bytesPerDim;
  if (ord > 0) {
    int cmp = StringHelper::compare(bytesPerDim, lastPackedValue, dimOffset,
                                    packedValue, packedValueOffset + dimOffset);
    if (cmp > 0) {
      throw make_shared<AssertionError>(
          L"values out of order: last value=" +
          make_shared<BytesRef>(lastPackedValue) + L" current value=" +
          make_shared<BytesRef>(packedValue, packedValueOffset,
                                packedBytesLength) +
          L" ord=" + to_wstring(ord));
    }
    if (cmp == 0 && doc < lastDoc) {
      throw make_shared<AssertionError>(
          L"docs out of order: last doc=" + to_wstring(lastDoc) +
          L" current doc=" + to_wstring(doc) + L" ord=" + to_wstring(ord));
    }
  }
  System::arraycopy(packedValue, packedValueOffset, lastPackedValue, 0,
                    packedBytesLength);
  return true;
}

shared_ptr<PointWriter>
SimpleTextBKDWriter::getPointWriter(int64_t count,
                                    const wstring &desc) 
{
  if (count <= maxPointsSortInHeap) {
    int size = Math::toIntExact(count);
    return make_shared<HeapPointWriter>(size, size, packedBytesLength, longOrds,
                                        singleValuePerDoc);
  } else {
    return make_shared<OfflinePointWriter>(tempDir, tempFileNamePrefix,
                                           packedBytesLength, longOrds, desc,
                                           count, singleValuePerDoc);
  }
}

void SimpleTextBKDWriter::write(shared_ptr<IndexOutput> out,
                                const wstring &s) 
{
  SimpleTextUtil::write(out, s, scratch);
}

void SimpleTextBKDWriter::writeInt(shared_ptr<IndexOutput> out,
                                   int x) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(out, Integer::toString(x), scratch);
}

void SimpleTextBKDWriter::writeLong(shared_ptr<IndexOutput> out,
                                    int64_t x) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(out, Long::toString(x), scratch);
}

void SimpleTextBKDWriter::write(shared_ptr<IndexOutput> out,
                                shared_ptr<BytesRef> b) 
{
  SimpleTextUtil::write(out, b);
}

void SimpleTextBKDWriter::newline(shared_ptr<IndexOutput> out) throw(
    IOException)
{
  SimpleTextUtil::writeNewline(out);
}
} // namespace org::apache::lucene::codecs::simpletext