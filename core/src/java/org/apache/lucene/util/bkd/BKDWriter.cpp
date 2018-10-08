using namespace std;

#include "BKDWriter.h"

namespace org::apache::lucene::util::bkd
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using MergeState = org::apache::lucene::index::MergeState;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using GrowableByteArrayDataOutput =
    org::apache::lucene::store::GrowableByteArrayDataOutput;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefComparator = org::apache::lucene::util::BytesRefComparator;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using MSBRadixSorter = org::apache::lucene::util::MSBRadixSorter;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using StringHelper = org::apache::lucene::util::StringHelper;
const wstring BKDWriter::CODEC_NAME = L"BKD";

BKDWriter::BKDWriter(int maxDoc, shared_ptr<Directory> tempDir,
                     const wstring &tempFileNamePrefix, int numDims,
                     int bytesPerDim, int maxPointsInLeafNode,
                     double maxMBSortInHeap, int64_t totalPointCount,
                     bool singleValuePerDoc) 
    : BKDWriter(maxDoc, tempDir, tempFileNamePrefix, numDims, bytesPerDim,
                maxPointsInLeafNode, maxMBSortInHeap, totalPointCount,
                singleValuePerDoc, totalPointCount > numeric_limits<int>::max(),
                Math::max(1, (long)maxMBSortInHeap),
                OfflineSorter::MAX_TEMPFILES)
{
}

BKDWriter::BKDWriter(int maxDoc, shared_ptr<Directory> tempDir,
                     const wstring &tempFileNamePrefix, int numDims,
                     int bytesPerDim, int maxPointsInLeafNode,
                     double maxMBSortInHeap, int64_t totalPointCount,
                     bool singleValuePerDoc, bool longOrds,
                     int64_t offlineSorterBufferMB,
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

void BKDWriter::verifyParams(int numDims, int maxPointsInLeafNode,
                             double maxMBSortInHeap, int64_t totalPointCount)
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

void BKDWriter::spillToOffline() 
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

void BKDWriter::add(std::deque<char> &packedValue,
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

int64_t BKDWriter::getPointCount() { return pointCount; }

BKDWriter::MergeReader::MergeReader(
    shared_ptr<BKDReader> bkd,
    shared_ptr<MergeState::DocMap> docMap) 
    : bkd(bkd), state(make_shared<BKDReader::IntersectState>(
                    bkd->in_->clone(), bkd->numDims, bkd->packedBytesLength,
                    bkd->maxPointsInLeafNode, nullptr, nullptr)),
      docMap(docMap), packedValues(std::deque<char>(bkd->maxPointsInLeafNode *
                                                     bkd->packedBytesLength))
{
  state->in_->seek(bkd->getMinLeafBlockFP());
}

bool BKDWriter::MergeReader::next() 
{
  // System.out.println("MR.next this=" + this);
  while (true) {
    if (docBlockUpto == docsInBlock) {
      if (blockID == bkd->leafNodeOffset) {
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

BKDWriter::MergeReader::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(shared_ptr<MergeReader> outerInstance)
{
  this->outerInstance = outerInstance;
  i = 0;
}

void BKDWriter::MergeReader::IntersectVisitorAnonymousInnerClass::visit(
    int docID) 
{
  throw make_shared<UnsupportedOperationException>();
}

void BKDWriter::MergeReader::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue) 
{
  assert(docID == outerInstance->state->scratchDocIDs[i]);
  System::arraycopy(packedValue, 0, outerInstance->packedValues,
                    i * outerInstance->bkd->packedBytesLength,
                    outerInstance->bkd->packedBytesLength);
  i++;
}

Relation BKDWriter::MergeReader::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  throw make_shared<UnsupportedOperationException>();
}

BKDWriter::BKDMergeQueue::BKDMergeQueue(int bytesPerDim, int maxSize)
    : org::apache::lucene::util::PriorityQueue<MergeReader>(maxSize),
      bytesPerDim(bytesPerDim)
{
}

bool BKDWriter::BKDMergeQueue::lessThan(shared_ptr<MergeReader> a,
                                        shared_ptr<MergeReader> b)
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

int64_t
BKDWriter::writeField(shared_ptr<IndexOutput> out, const wstring &fieldName,
                      shared_ptr<MutablePointValues> reader) 
{
  if (numDims == 1) {
    return writeField1Dim(out, fieldName, reader);
  } else {
    return writeFieldNDims(out, fieldName, reader);
  }
}

int64_t BKDWriter::writeFieldNDims(
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

  const std::deque<int> parentSplits = std::deque<int>(numDims);
  build(1, numLeaves, values, 0, Math::toIntExact(pointCount), out,
        minPackedValue, maxPackedValue, parentSplits, splitPackedValues,
        leafBlockFPs, std::deque<int>(maxPointsInLeafNode));
  assert((Arrays::equals(parentSplits, std::deque<int>(numDims))));

  int64_t indexFP = out->getFilePointer();
  writeIndex(out, Math::toIntExact(countPerLeaf), leafBlockFPs,
             splitPackedValues);
  return indexFP;
}

int64_t BKDWriter::writeField1Dim(
    shared_ptr<IndexOutput> out, const wstring &fieldName,
    shared_ptr<MutablePointValues> reader) 
{
  MutablePointsReaderUtils::sort(maxDoc, packedBytesLength, reader, 0,
                                 Math::toIntExact(reader->size()));

  shared_ptr<OneDimensionBKDWriter> *const oneDimWriter =
      make_shared<OneDimensionBKDWriter>(shared_from_this(), out);

  reader->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
      shared_from_this(), oneDimWriter));

  return oneDimWriter->finish();
}

BKDWriter::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<BKDWriter> outerInstance,
        shared_ptr<
            org::apache::lucene::util::bkd::BKDWriter::OneDimensionBKDWriter>
            oneDimWriter)
{
  this->outerInstance = outerInstance;
  this->oneDimWriter = oneDimWriter;
}

void BKDWriter::IntersectVisitorAnonymousInnerClass::visit(
    int docID, std::deque<char> &packedValue) 
{
  oneDimWriter->add(packedValue, docID);
}

void BKDWriter::IntersectVisitorAnonymousInnerClass::visit(int docID) throw(
    IOException)
{
  throw make_shared<IllegalStateException>();
}

Relation BKDWriter::IntersectVisitorAnonymousInnerClass::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return Relation::CELL_CROSSES_QUERY;
}

int64_t
BKDWriter::merge(shared_ptr<IndexOutput> out,
                 deque<std::shared_ptr<MergeState::DocMap>> &docMaps,
                 deque<std::shared_ptr<BKDReader>> &readers) 
{
  assert(docMaps.empty() || readers.size() == docMaps.size());

  shared_ptr<BKDMergeQueue> queue =
      make_shared<BKDMergeQueue>(bytesPerDim, readers.size());

  for (int i = 0; i < readers.size(); i++) {
    shared_ptr<BKDReader> bkd = readers[i];
    shared_ptr<MergeState::DocMap> docMap;
    if (docMaps.empty()) {
      docMap.reset();
    } else {
      docMap = docMaps[i];
    }
    shared_ptr<MergeReader> reader = make_shared<MergeReader>(bkd, docMap);
    if (reader->next()) {
      queue->push_back(reader);
    }
  }

  shared_ptr<OneDimensionBKDWriter> oneDimWriter =
      make_shared<OneDimensionBKDWriter>(shared_from_this(), out);

  while (queue->size() != 0) {
    shared_ptr<MergeReader> reader = queue->top();
    // System.out.println("iter reader=" + reader);

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

BKDWriter::OneDimensionBKDWriter::OneDimensionBKDWriter(
    shared_ptr<BKDWriter> outerInstance, shared_ptr<IndexOutput> out)
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

void BKDWriter::OneDimensionBKDWriter::add(std::deque<char> &packedValue,
                                           int docID) 
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
    // from when we write N > 1 dimensional points where we write between max/2
    // and max per leaf block
    writeLeafBlock();
    leafCount = 0;
  }

  assert(lastDocID = docID) >= 0; // only assign when asserts are enabled
}

int64_t BKDWriter::OneDimensionBKDWriter::finish() 
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
  outerInstance->writeIndex(out, outerInstance->maxPointsInLeafNode, arr,
                            index);
  return indexFP;
}

void BKDWriter::OneDimensionBKDWriter::writeLeafBlock() 
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

  // Find per-dim common prefix:
  int prefix = outerInstance->bytesPerDim;
  int offset = (leafCount - 1) * outerInstance->packedBytesLength;
  for (int j = 0; j < outerInstance->bytesPerDim; j++) {
    if (leafValues[j] != leafValues[offset + j]) {
      prefix = j;
      break;
    }
  }

  outerInstance->commonPrefixLengths[0] = prefix;

  assert(outerInstance->scratchOut->getPosition() == 0);
  outerInstance->writeLeafBlockDocs(outerInstance->scratchOut, leafDocs, 0,
                                    leafCount);
  outerInstance->writeCommonPrefixes(outerInstance->scratchOut,
                                     outerInstance->commonPrefixLengths,
                                     leafValues);

  outerInstance->scratchBytesRef1->length = outerInstance->packedBytesLength;
  outerInstance->scratchBytesRef1->bytes = leafValues;

  const function<BytesRef *(int)> packedValues = [&](int i) {
    outerInstance->scratchBytesRef1->offset =
        outerInstance->packedBytesLength * i;
    return outerInstance->scratchBytesRef1;
  };
  assert((outerInstance->valuesInOrderAndBounds(
      leafCount, 0,
      Arrays::copyOf(leafValues, outerInstance->packedBytesLength),
      Arrays::copyOfRange(leafValues,
                          (leafCount - 1) * outerInstance->packedBytesLength,
                          leafCount * outerInstance->packedBytesLength),
      packedValues, leafDocs, 0)));
  outerInstance->writeLeafBlockPackedValues(outerInstance->scratchOut,
                                            outerInstance->commonPrefixLengths,
                                            leafCount, 0, packedValues);
  out->writeBytes(outerInstance->scratchOut->getBytes(), 0,
                  outerInstance->scratchOut->getPosition());
  outerInstance->scratchOut->reset();
}

void BKDWriter::rotateToTree(int nodeID, int offset, int count,
                             std::deque<char> &index,
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

void BKDWriter::sortHeapPointWriter(shared_ptr<HeapPointWriter> writer, int dim)
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

BKDWriter::MSBRadixSorterAnonymousInnerClass::MSBRadixSorterAnonymousInnerClass(
    shared_ptr<BKDWriter> outerInstance, shared_ptr<UnknownType> BYTES,
    shared_ptr<org::apache::lucene::util::bkd::HeapPointWriter> writer, int dim)
    : org::apache::lucene::util::MSBRadixSorter(BYTES)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
  this->dim = dim;
}

int BKDWriter::MSBRadixSorterAnonymousInnerClass::byteAt(int i, int k)
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

void BKDWriter::MSBRadixSorterAnonymousInnerClass::swap(int i, int j)
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

shared_ptr<PointWriter> BKDWriter::sort(int dim) 
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

BKDWriter::BytesRefComparatorAnonymousInnerClass::
    BytesRefComparatorAnonymousInnerClass(shared_ptr<BKDWriter> outerInstance,
                                          shared_ptr<UnknownType> BYTES,
                                          int offset)
    : org::apache::lucene::util::BytesRefComparator(BYTES)
{
  this->outerInstance = outerInstance;
  this->offset = offset;
}

int BKDWriter::BytesRefComparatorAnonymousInnerClass::byteAt(
    shared_ptr<BytesRef> ref, int i)
{
  return ref->bytes[ref->offset + offset + i] & 0xff;
}

BKDWriter::BytesRefComparatorAnonymousInnerClass2::
    BytesRefComparatorAnonymousInnerClass2(shared_ptr<BKDWriter> outerInstance,
                                           shared_ptr<UnknownType> BYTES,
                                           int offset)
    : org::apache::lucene::util::BytesRefComparator(BYTES)
{
  this->outerInstance = outerInstance;
  this->offset = offset;
}

int BKDWriter::BytesRefComparatorAnonymousInnerClass2::byteAt(
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

BKDWriter::OfflineSorterAnonymousInnerClass::OfflineSorterAnonymousInnerClass(
    shared_ptr<BKDWriter> outerInstance,
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
BKDWriter::OfflineSorterAnonymousInnerClass::getWriter(
    shared_ptr<IndexOutput> out, int64_t count)
{
  return make_shared<ByteSequencesWriterAnonymousInnerClass>(shared_from_this(),
                                                             out);
}

BKDWriter::OfflineSorterAnonymousInnerClass::
    ByteSequencesWriterAnonymousInnerClass::
        ByteSequencesWriterAnonymousInnerClass(
            shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance,
            shared_ptr<IndexOutput> out)
    : ByteSequencesWriter(out)
{
  this->outerInstance = outerInstance;
  this->out = out;
}

void BKDWriter::OfflineSorterAnonymousInnerClass::
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
BKDWriter::OfflineSorterAnonymousInnerClass::getReader(
    shared_ptr<ChecksumIndexInput> in_, const wstring &name) 
{
  return make_shared<ByteSequencesReaderAnonymousInnerClass>(shared_from_this(),
                                                             in_, name);
}

BKDWriter::OfflineSorterAnonymousInnerClass::
    ByteSequencesReaderAnonymousInnerClass::
        ByteSequencesReaderAnonymousInnerClass(
            shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance,
            shared_ptr<ChecksumIndexInput> in_, const wstring &name)
    : ByteSequencesReader(in_, name)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
  scratch = make_shared<BytesRef>(
      std::deque<char>(outerInstance->outerInstance.bytesPerDoc));
}

shared_ptr<BytesRef> BKDWriter::OfflineSorterAnonymousInnerClass::
    ByteSequencesReaderAnonymousInnerClass::next() 
{
  if (in_->getFilePointer() >= end) {
    return nullptr;
  }
  in_->readBytes(scratch::bytes, 0, outerInstance->outerInstance.bytesPerDoc);
  return scratch;
}

void BKDWriter::checkMaxLeafNodeCount(int numLeaves)
{
  if ((1 + bytesPerDim) * static_cast<int64_t>(numLeaves) >
      ArrayUtil::MAX_ARRAY_LENGTH) {
    throw make_shared<IllegalStateException>(
        L"too many nodes; increase maxPointsInLeafNode (currently " +
        to_wstring(maxPointsInLeafNode) + L") and reindex");
  }
}

int64_t BKDWriter::finish(shared_ptr<IndexOutput> out) 
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
  std::deque<std::shared_ptr<PathSlice>> sortedPointWriters(numDims);

  // This is only used on exception; on normal code paths we close all files we
  // opened:
  deque<std::shared_ptr<Closeable>> toCloseHeroically =
      deque<std::shared_ptr<Closeable>>();

  bool success = false;
  try {
    // long t0 = System.nanoTime();
    for (int dim = 0; dim < numDims; dim++) {
      sortedPointWriters[dim] =
          make_shared<PathSlice>(sort(dim), 0, pointCount);
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

    const std::deque<int> parentSplits = std::deque<int>(numDims);
    build(1, numLeaves, sortedPointWriters, ordBitSet, out, minPackedValue,
          maxPackedValue, parentSplits, splitPackedValues, leafBlockFPs,
          toCloseHeroically);
    assert((Arrays::equals(parentSplits, std::deque<int>(numDims))));

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
  writeIndex(out, Math::toIntExact(countPerLeaf), leafBlockFPs,
             splitPackedValues);
  return indexFP;
}

std::deque<char>
BKDWriter::packIndex(std::deque<int64_t> &leafBlockFPs,
                     std::deque<char> &splitPackedValues) 
{

  int numLeaves = leafBlockFPs.size();

  // Possibly rotate the leaf block FPs, if the index not fully balanced binary
  // tree (only happens if it was created by OneDimensionBKDWriter).  In this
  // case the leaf nodes may straddle the two bottom levels of the binary tree:
  if (numDims == 1 && numLeaves > 1) {
    int levelCount = 2;
    while (true) {
      if (numLeaves >= levelCount && numLeaves <= 2 * levelCount) {
        int lastLevel = 2 * (numLeaves - levelCount);
        assert(lastLevel >= 0);
        if (lastLevel != 0) {
          // Last level is partially filled, so we must rotate the leaf FPs to
          // match.  We do this here, after loading at read-time, so that we can
          // still delta code them on disk at write:
          std::deque<int64_t> newLeafBlockFPs(numLeaves);
          System::arraycopy(leafBlockFPs, lastLevel, newLeafBlockFPs, 0,
                            leafBlockFPs.size() - lastLevel);
          System::arraycopy(leafBlockFPs, 0, newLeafBlockFPs,
                            leafBlockFPs.size() - lastLevel, lastLevel);
          leafBlockFPs = newLeafBlockFPs;
        }
        break;
      }

      levelCount *= 2;
    }
  }

  /** Reused while packing the index */
  shared_ptr<RAMOutputStream> writeBuffer = make_shared<RAMOutputStream>();

  // This is the "file" we append the byte[] to:
  deque<std::deque<char>> blocks = deque<std::deque<char>>();
  std::deque<char> lastSplitValues(bytesPerDim * numDims);
  // System.out.println("\npack index");
  int totalSize = recursePackIndex(writeBuffer, leafBlockFPs, splitPackedValues,
                                   0LL, blocks, 1, lastSplitValues,
                                   std::deque<bool>(numDims), false);

  // Compact the byte[] blocks into single byte index:
  std::deque<char> index(totalSize);
  int upto = 0;
  for (auto block : blocks) {
    System::arraycopy(block, 0, index, upto, block.size());
    upto += block.size();
  }
  assert(upto == totalSize);

  return index;
}

int BKDWriter::appendBlock(shared_ptr<RAMOutputStream> writeBuffer,
                           deque<std::deque<char>> &blocks) 
{
  int pos = Math::toIntExact(writeBuffer->getFilePointer());
  std::deque<char> bytes(pos);
  writeBuffer->writeTo(bytes, 0);
  writeBuffer->reset();
  blocks.push_back(bytes);
  return pos;
}

int BKDWriter::recursePackIndex(shared_ptr<RAMOutputStream> writeBuffer,
                                std::deque<int64_t> &leafBlockFPs,
                                std::deque<char> &splitPackedValues,
                                int64_t minBlockFP,
                                deque<std::deque<char>> &blocks, int nodeID,
                                std::deque<char> &lastSplitValues,
                                std::deque<bool> &negativeDeltas,
                                bool isLeft) 
{
  if (nodeID >= leafBlockFPs.size()) {
    int leafID = nodeID - leafBlockFPs.size();
    // System.out.println("recursePack leaf nodeID=" + nodeID);

    // In the unbalanced case it's possible the left most node only has one
    // child:
    if (leafID < leafBlockFPs.size()) {
      int64_t delta = leafBlockFPs[leafID] - minBlockFP;
      if (isLeft) {
        assert(delta == 0);
        return 0;
      } else {
        assert((nodeID == 1 || delta > 0, L"nodeID=" + to_wstring(nodeID)));
        writeBuffer->writeVLong(delta);
        return appendBlock(writeBuffer, blocks);
      }
    } else {
      return 0;
    }
  } else {
    int64_t leftBlockFP;
    if (isLeft == false) {
      leftBlockFP = getLeftMostLeafBlockFP(leafBlockFPs, nodeID);
      int64_t delta = leftBlockFP - minBlockFP;
      assert(nodeID == 1 || delta > 0);
      writeBuffer->writeVLong(delta);
    } else {
      // The left tree's left most leaf block FP is always the minimal FP:
      leftBlockFP = minBlockFP;
    }

    int address = nodeID * (1 + bytesPerDim);
    int splitDim = splitPackedValues[address++] & 0xff;

    // System.out.println("recursePack inner nodeID=" + nodeID + " splitDim=" +
    // splitDim + " splitValue=" + new BytesRef(splitPackedValues, address,
    // bytesPerDim));

    // find common prefix with last split value in this dim:
    int prefix = 0;
    for (; prefix < bytesPerDim; prefix++) {
      if (splitPackedValues[address + prefix] !=
          lastSplitValues[splitDim * bytesPerDim + prefix]) {
        break;
      }
    }

    // System.out.println("writeNodeData nodeID=" + nodeID + " splitDim=" +
    // splitDim + " numDims=" + numDims + " bytesPerDim=" + bytesPerDim + "
    // prefix=" + prefix);

    int firstDiffByteDelta;
    if (prefix < bytesPerDim) {
      // System.out.println("  delta byte cur=" +
      // Integer.toHexString(splitPackedValues[address+prefix]&0xFF) + " prev=" +
      // Integer.toHexString(lastSplitValues[splitDim * bytesPerDim +
      // prefix]&0xFF) + " negated?=" + negativeDeltas[splitDim]);
      firstDiffByteDelta =
          (splitPackedValues[address + prefix] & 0xFF) -
          (lastSplitValues[splitDim * bytesPerDim + prefix] & 0xFF);
      if (negativeDeltas[splitDim]) {
        firstDiffByteDelta = -firstDiffByteDelta;
      }
      // System.out.println("  delta=" + firstDiffByteDelta);
      assert(firstDiffByteDelta > 0);
    } else {
      firstDiffByteDelta = 0;
    }

    // pack the prefix, splitDim and delta first diff byte into a single vInt:
    int code =
        (firstDiffByteDelta * (1 + bytesPerDim) + prefix) * numDims + splitDim;

    // System.out.println("  code=" + code);
    // System.out.println("  splitValue=" + new BytesRef(splitPackedValues,
    // address, bytesPerDim));

    writeBuffer->writeVInt(code);

    // write the split value, prefix coded vs. our parent's split value:
    int suffix = bytesPerDim - prefix;
    std::deque<char> savSplitValue(suffix);
    if (suffix > 1) {
      writeBuffer->writeBytes(splitPackedValues, address + prefix + 1,
                              suffix - 1);
    }

    std::deque<char> cmp = lastSplitValues.clone();

    System::arraycopy(lastSplitValues, splitDim * bytesPerDim + prefix,
                      savSplitValue, 0, suffix);

    // copy our split value into lastSplitValues for our children to prefix-code
    // against
    System::arraycopy(splitPackedValues, address + prefix, lastSplitValues,
                      splitDim * bytesPerDim + prefix, suffix);

    int numBytes = appendBlock(writeBuffer, blocks);

    // placeholder for left-tree numBytes; we need this so that at search time
    // if we only need to recurse into the right sub-tree we can quickly seek to
    // its starting point
    int idxSav = blocks.size();
    blocks.push_back(nullptr);

    bool savNegativeDelta = negativeDeltas[splitDim];
    negativeDeltas[splitDim] = true;

    int leftNumBytes = recursePackIndex(
        writeBuffer, leafBlockFPs, splitPackedValues, leftBlockFP, blocks,
        2 * nodeID, lastSplitValues, negativeDeltas, true);

    if (nodeID * 2 < leafBlockFPs.size()) {
      writeBuffer->writeVInt(leftNumBytes);
    } else {
      assert((leftNumBytes == 0, L"leftNumBytes=" + to_wstring(leftNumBytes)));
    }
    int numBytes2 = Math::toIntExact(writeBuffer->getFilePointer());
    std::deque<char> bytes2(numBytes2);
    writeBuffer->writeTo(bytes2, 0);
    writeBuffer->reset();
    // replace our placeholder:
    blocks[idxSav] = bytes2;

    negativeDeltas[splitDim] = false;
    int rightNumBytes = recursePackIndex(
        writeBuffer, leafBlockFPs, splitPackedValues, leftBlockFP, blocks,
        2 * nodeID + 1, lastSplitValues, negativeDeltas, false);

    negativeDeltas[splitDim] = savNegativeDelta;

    // restore lastSplitValues to what caller originally passed us:
    System::arraycopy(savSplitValue, 0, lastSplitValues,
                      splitDim * bytesPerDim + prefix, suffix);

    assert((Arrays::equals(lastSplitValues, cmp)));

    return numBytes + numBytes2 + leftNumBytes + rightNumBytes;
  }
}

int64_t
BKDWriter::getLeftMostLeafBlockFP(std::deque<int64_t> &leafBlockFPs,
                                  int nodeID)
{
  int nodeIDIn = nodeID;
  // TODO: can we do this cheaper, e.g. a closed form solution instead of while
  // loop?  Or change the recursion while packing the index to return this
  // left-most leaf block FP from each recursion instead?
  //
  // Still, the overall cost here is minor: this method's cost is O(log(N)), and
  // while writing we call it O(N) times (N = number of leaf blocks)
  while (nodeID < leafBlockFPs.size()) {
    nodeID *= 2;
  }
  int leafID = nodeID - leafBlockFPs.size();
  int64_t result = leafBlockFPs[leafID];
  if (result < 0) {
    throw make_shared<AssertionError>(to_wstring(result) + L" for leaf " +
                                      to_wstring(leafID));
  }
  return result;
}

void BKDWriter::writeIndex(
    shared_ptr<IndexOutput> out, int countPerLeaf,
    std::deque<int64_t> &leafBlockFPs,
    std::deque<char> &splitPackedValues) 
{
  std::deque<char> packedIndex = packIndex(leafBlockFPs, splitPackedValues);
  writeIndex(out, countPerLeaf, leafBlockFPs.size(), packedIndex);
}

void BKDWriter::writeIndex(shared_ptr<IndexOutput> out, int countPerLeaf,
                           int numLeaves,
                           std::deque<char> &packedIndex) 
{

  CodecUtil::writeHeader(out, CODEC_NAME, VERSION_CURRENT);
  out->writeVInt(numDims);
  out->writeVInt(countPerLeaf);
  out->writeVInt(bytesPerDim);

  assert(numLeaves > 0);
  out->writeVInt(numLeaves);
  out->writeBytes(minPackedValue, 0, packedBytesLength);
  out->writeBytes(maxPackedValue, 0, packedBytesLength);

  out->writeVLong(pointCount);
  out->writeVInt(docsSeen->cardinality());
  out->writeVInt(packedIndex.size());
  out->writeBytes(packedIndex, 0, packedIndex.size());
}

void BKDWriter::writeLeafBlockDocs(shared_ptr<DataOutput> out,
                                   std::deque<int> &docIDs, int start,
                                   int count) 
{
  assert(
      (count > 0, L"maxPointsInLeafNode=" + to_wstring(maxPointsInLeafNode)));
  out->writeVInt(count);
  DocIdsWriter::writeDocIds(docIDs, start, count, out);
}

void BKDWriter::writeLeafBlockPackedValues(
    shared_ptr<DataOutput> out, std::deque<int> &commonPrefixLengths,
    int count, int sortedDim,
    function<BytesRef *(int)> &packedValues) 
{
  int prefixLenSum = Arrays::stream(commonPrefixLengths).sum();
  if (prefixLenSum == packedBytesLength) {
    // all values in this block are equal
    out->writeByte((char)-1);
  } else {
    assert(commonPrefixLengths[sortedDim] < bytesPerDim);
    out->writeByte(static_cast<char>(sortedDim));
    int compressedByteOffset =
        sortedDim * bytesPerDim + commonPrefixLengths[sortedDim];
    commonPrefixLengths[sortedDim]++;
    for (int i = 0; i < count;) {
      // do run-length compression on the byte at compressedByteOffset
      int runLen = BKDWriter::runLen(packedValues, i, min(i + 0xff, count),
                                     compressedByteOffset);
      assert(runLen <= 0xff);
      shared_ptr<BytesRef> first = packedValues(i);
      char prefixByte = first->bytes[first->offset + compressedByteOffset];
      out->writeByte(prefixByte);
      out->writeByte(static_cast<char>(runLen));
      writeLeafBlockPackedValuesRange(out, commonPrefixLengths, i, i + runLen,
                                      packedValues);
      i += runLen;
      assert(i <= count);
    }
  }
}

void BKDWriter::writeLeafBlockPackedValuesRange(
    shared_ptr<DataOutput> out, std::deque<int> &commonPrefixLengths,
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

int BKDWriter::runLen(function<BytesRef *(int)> &packedValues, int start,
                      int end, int byteOffset)
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

void BKDWriter::writeCommonPrefixes(
    shared_ptr<DataOutput> out, std::deque<int> &commonPrefixes,
    std::deque<char> &packedValue) 
{
  for (int dim = 0; dim < numDims; dim++) {
    out->writeVInt(commonPrefixes[dim]);
    // System.out.println(commonPrefixes[dim] + " of " + bytesPerDim);
    out->writeBytes(packedValue, dim * bytesPerDim, commonPrefixes[dim]);
  }
}

BKDWriter::~BKDWriter()
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

BKDWriter::PathSlice::PathSlice(shared_ptr<PointWriter> writer, int64_t start,
                                int64_t count)
    : writer(writer), start(start), count(count)
{
}

wstring BKDWriter::PathSlice::toString()
{
  return L"PathSlice(start=" + to_wstring(start) + L" count=" +
         to_wstring(count) + L" writer=" + writer + L")";
}

shared_ptr<Error>
BKDWriter::verifyChecksum(runtime_error priorException,
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

std::deque<char>
BKDWriter::markRightTree(int64_t rightCount, int splitDim,
                         shared_ptr<PathSlice> source,
                         shared_ptr<LongBitSet> ordBitSet) 
{

  // Now we mark ords that fall into the right half, so we can partition on all
  // other dims that are not the split dim:

  // Read the split value, then mark all ords in the right tree (larger than the
  // split value):

  // TODO: find a way to also checksum this reader?  If we changed to
  // markLeftTree, and scanned the final chunk, it could work?
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (PointReader reader =
  // source.writer.getReader(source.start + source.count - rightCount,
  // rightCount))
  {
    PointReader reader = source->writer->getReader(
        source->start + source->count - rightCount, rightCount);
    try {
      bool result = reader->next();
      assert((result, L"rightCount=" + to_wstring(rightCount) +
                          L" source.count=" + to_wstring(source->count) +
                          L" source.writer=" + source->writer));
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

bool BKDWriter::valueInBounds(shared_ptr<BytesRef> packedValue,
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

int BKDWriter::split(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue,
                     std::deque<int> &parentSplits)
{
  // First look at whether there is a dimension that has split less than 2x less
  // than the dim that has most splits, and return it if there is such a
  // dimension and it does not only have equals values. This helps ensure all
  // dimensions are indexed.
  int maxNumSplits = 0;
  for (auto numSplits : parentSplits) {
    maxNumSplits = max(maxNumSplits, numSplits);
  }
  for (int dim = 0; dim < numDims; ++dim) {
    constexpr int offset = dim * bytesPerDim;
    if (parentSplits[dim] < maxNumSplits / 2 &&
        StringHelper::compare(bytesPerDim, minPackedValue, offset,
                              maxPackedValue, offset) != 0) {
      return dim;
    }
  }

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

shared_ptr<PathSlice> BKDWriter::switchToHeap(
    shared_ptr<PathSlice> source,
    deque<std::shared_ptr<Closeable>> &toCloseHeroically) 
{
  int count = Math::toIntExact(source->count);
  // Not inside the try because we don't want to close it here:
  shared_ptr<PointReader> reader = source->writer->getSharedReader(
      source->start, source->count, toCloseHeroically);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (PointWriter writer = new
  // HeapPointWriter(count, count, packedBytesLength, longOrds,
  // singleValuePerDoc))
  {
    PointWriter writer = HeapPointWriter(count, count, packedBytesLength,
                                         longOrds, singleValuePerDoc);
    try {
      for (int i = 0; i < count; i++) {
        bool hasNext = reader->next();
        assert(hasNext);
        writer->append(reader->packedValue(), reader->ord(), reader->docID());
      }
      return make_shared<PathSlice>(writer, 0, count);
    } catch (const runtime_error &t) {
      throw verifyChecksum(t, source->writer);
    }
  }
}

void BKDWriter::build(int nodeID, int leafNodeOffset,
                      shared_ptr<MutablePointValues> reader, int from, int to,
                      shared_ptr<IndexOutput> out,
                      std::deque<char> &minPackedValue,
                      std::deque<char> &maxPackedValue,
                      std::deque<int> &parentSplits,
                      std::deque<char> &splitPackedValues,
                      std::deque<int64_t> &leafBlockFPs,
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

    assert(scratchOut->getPosition() == 0);

    // Write doc IDs
    std::deque<int> docIDs = spareDocIds;
    for (int i = from; i < to; ++i) {
      docIDs[i - from] = reader->getDocID(i);
    }
    // System.out.println("writeLeafBlock pos=" + out.getFilePointer());
    writeLeafBlockDocs(scratchOut, docIDs, 0, count);

    // Write the common prefixes:
    reader->getValue(from, scratchBytesRef1);
    System::arraycopy(scratchBytesRef1->bytes, scratchBytesRef1->offset,
                      scratch1, 0, packedBytesLength);
    writeCommonPrefixes(scratchOut, commonPrefixLengths, scratch1);

    // Write the full values:
    function<BytesRef *(int)> packedValues = [&](int i) {
      reader->getValue(from + i, scratchBytesRef1);
      return scratchBytesRef1;
    };
    assert((valuesInOrderAndBounds(count, sortedDim, minPackedValue,
                                   maxPackedValue, packedValues, docIDs, 0)));
    writeLeafBlockPackedValues(scratchOut, commonPrefixLengths, count,
                               sortedDim, packedValues);

    out->writeBytes(scratchOut->getBytes(), 0, scratchOut->getPosition());
    scratchOut->reset();

  } else {
    // inner node

    // compute the split dimension and partition around it
    constexpr int splitDim =
        split(minPackedValue, maxPackedValue, parentSplits);
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
    parentSplits[splitDim]++;
    build(nodeID * 2, leafNodeOffset, reader, from, mid, out, minPackedValue,
          maxSplitPackedValue, parentSplits, splitPackedValues, leafBlockFPs,
          spareDocIds);
    build(nodeID * 2 + 1, leafNodeOffset, reader, mid, to, out,
          minSplitPackedValue, maxPackedValue, parentSplits, splitPackedValues,
          leafBlockFPs, spareDocIds);
    parentSplits[splitDim]--;
  }
}

void BKDWriter::build(
    int nodeID, int leafNodeOffset,
    std::deque<std::shared_ptr<PathSlice>> &slices,
    shared_ptr<LongBitSet> ordBitSet, shared_ptr<IndexOutput> out,
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue,
    std::deque<int> &parentSplits, std::deque<char> &splitPackedValues,
    std::deque<int64_t> &leafBlockFPs,
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

      shared_ptr<PathSlice> source = slices[dim];

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

    shared_ptr<PathSlice> source = slices[sortedDim];

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

    // Write the common prefixes:
    writeCommonPrefixes(out, commonPrefixLengths, scratch1);

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
      splitDim = split(minPackedValue, maxPackedValue, parentSplits);
    } else {
      splitDim = 0;
    }

    shared_ptr<PathSlice> source = slices[splitDim];

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

    std::deque<std::shared_ptr<PathSlice>> leftSlices(numDims);
    std::deque<std::shared_ptr<PathSlice>> rightSlices(numDims);

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
        leftSlices[dim] =
            make_shared<PathSlice>(source->writer, source->start, leftCount);
        rightSlices[dim] = make_shared<PathSlice>(
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
      // C++ equivalent: ORIGINAL LINE: try (PointWriter leftPointWriter =
      // getPointWriter(leftCount, "left" + dim); PointWriter rightPointWriter =
      // getPointWriter(source.count - leftCount, "right" + dim))
      {
        PointWriter leftPointWriter =
            getPointWriter(leftCount, L"left" + to_wstring(dim));
        PointWriter rightPointWriter = getPointWriter(
            source->count - leftCount, L"right" + to_wstring(dim));
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
              make_shared<PathSlice>(leftPointWriter, 0, leftCount);
          rightSlices[dim] =
              make_shared<PathSlice>(rightPointWriter, 0, rightCount);
        } catch (const runtime_error &t) {
          throw verifyChecksum(t, slices[dim]->writer);
        }
      }
    }

    parentSplits[splitDim]++;
    // Recurse on left tree:
    build(2 * nodeID, leafNodeOffset, leftSlices, ordBitSet, out,
          minPackedValue, maxSplitPackedValue, parentSplits, splitPackedValues,
          leafBlockFPs, toCloseHeroically);
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
          minSplitPackedValue, maxPackedValue, parentSplits, splitPackedValues,
          leafBlockFPs, toCloseHeroically);
    for (int dim = 0; dim < numDims; dim++) {
      // Don't destroy the dim we split on because we just re-used what our
      // caller above gave us for that dim:
      if (dim != splitDim) {
        rightSlices[dim]->writer.destroy();
      }
    }
    parentSplits[splitDim]--;
  }
}

BKDWriter::functionAnonymousInnerClass::functionAnonymousInnerClass(
    shared_ptr<BKDWriter> outerInstance, shared_ptr<UnknownType> int,
    shared_ptr<org::apache::lucene::util::bkd::BKDWriter::PathSlice> source,
    shared_ptr<org::apache::lucene::util::bkd::HeapPointWriter> heapSource)
    : function<org::apache::lucene::util::BytesRef *(int)>(int)
{
  this->outerInstance = outerInstance;
  this->source = source;
  this->heapSource = heapSource;
  scratch = make_shared<BytesRef>();

  scratch->length = outerInstance->packedBytesLength;
}

shared_ptr<BytesRef> BKDWriter::functionAnonymousInnerClass::apply(int i)
{
  heapSource->getPackedValueSlice(Math::toIntExact(source->start + i), scratch);
  return scratch;
}

bool BKDWriter::valuesInOrderAndBounds(int count, int sortedDim,
                                       std::deque<char> &minPackedValue,
                                       std::deque<char> &maxPackedValue,
                                       function<BytesRef *(int)> &values,
                                       std::deque<int> &docs,
                                       int docsOffset) 
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

bool BKDWriter::valueInOrder(int64_t ord, int sortedDim,
                             std::deque<char> &lastPackedValue,
                             std::deque<char> &packedValue,
                             int packedValueOffset, int doc, int lastDoc)
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
BKDWriter::getPointWriter(int64_t count,
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
} // namespace org::apache::lucene::util::bkd