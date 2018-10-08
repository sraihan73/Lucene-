using namespace std;

#include "BKDReader.h"

namespace org::apache::lucene::util::bkd
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using PointValues = org::apache::lucene::index::PointValues;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using MathUtil = org::apache::lucene::util::MathUtil;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;

BKDReader::BKDReader(shared_ptr<IndexInput> in_) 
    : leafNodeOffset(numLeaves), numDims(in_->readVInt()),
      bytesPerDim(in_->readVInt()), numLeaves(in_->readVInt()), in_(in_),
      maxPointsInLeafNode(in_->readVInt()),
      minPackedValue(std::deque<char>(packedBytesLength)),
      maxPackedValue(std::deque<char>(packedBytesLength)),
      pointCount(in_->readVLong()), docCount(in_->readVInt()),
      version(CodecUtil::checkHeader(in_, BKDWriter::CODEC_NAME,
                                     BKDWriter::VERSION_START,
                                     BKDWriter::VERSION_CURRENT)),
      packedBytesLength(numDims * bytesPerDim),
      bytesPerIndexEntry(
          numDims == 1 && version >= BKDWriter::VERSION_IMPLICIT_SPLIT_DIM_1D
              ? bytesPerDim
              : bytesPerDim + 1)
{

  // Read index:
  assert(numLeaves > 0);

  in_->readBytes(minPackedValue, 0, packedBytesLength);
  in_->readBytes(maxPackedValue, 0, packedBytesLength);

  for (int dim = 0; dim < numDims; dim++) {
    if (StringHelper::compare(bytesPerDim, minPackedValue, dim * bytesPerDim,
                              maxPackedValue, dim * bytesPerDim) > 0) {
      throw make_shared<CorruptIndexException>(
          L"minPackedValue " + make_shared<BytesRef>(minPackedValue) +
              L" is > maxPackedValue " + make_shared<BytesRef>(maxPackedValue) +
              L" for dim=" + to_wstring(dim),
          in_);
    }
  }

  if (version >= BKDWriter::VERSION_PACKED_INDEX) {
    int numBytes = in_->readVInt();
    packedIndex = std::deque<char>(numBytes);
    in_->readBytes(packedIndex, 0, numBytes);
    leafBlockFPs.clear();
    splitPackedValues.clear();
  } else {
    // legacy un-packed index

    splitPackedValues = std::deque<char>(bytesPerIndexEntry * numLeaves);

    in_->readBytes(splitPackedValues, 0, splitPackedValues.size());

    // Read the file pointers to the start of each leaf block:
    std::deque<int64_t> leafBlockFPs(numLeaves);
    int64_t lastFP = 0;
    for (int i = 0; i < numLeaves; i++) {
      int64_t delta = in_->readVLong();
      leafBlockFPs[i] = lastFP + delta;
      lastFP += delta;
    }

    // Possibly rotate the leaf block FPs, if the index not fully balanced
    // binary tree (only happens if it was created by BKDWriter.merge or
    // OneDimWriter).  In this case the leaf nodes may straddle the two bottom
    // levels of the binary tree:
    if (numDims == 1 && numLeaves > 1) {
      int levelCount = 2;
      while (true) {
        if (numLeaves >= levelCount && numLeaves <= 2 * levelCount) {
          int lastLevel = 2 * (numLeaves - levelCount);
          assert(lastLevel >= 0);
          if (lastLevel != 0) {
            // Last level is partially filled, so we must rotate the leaf FPs to
            // match.  We do this here, after loading at read-time, so that we
            // can still delta code them on disk at write:
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

    this->leafBlockFPs = leafBlockFPs;
    packedIndex.clear();
  }
}

int64_t BKDReader::getMinLeafBlockFP()
{
  if (packedIndex.size() > 0) {
    return (make_shared<ByteArrayDataInput>(packedIndex))->readVLong();
  } else {
    int64_t minFP = numeric_limits<int64_t>::max();
    for (auto fp : leafBlockFPs) {
      minFP = min(minFP, fp);
    }
    return minFP;
  }
}

BKDReader::IndexTree::IndexTree(shared_ptr<BKDReader> outerInstance)
    : splitPackedValueStack(std::deque<std::deque<char>>(treeDepth + 1)),
      outerInstance(outerInstance)
{
  int treeDepth = outerInstance->getTreeDepth();
  nodeID = 1;
  level = 1;
  splitPackedValueStack[level] =
      std::deque<char>(outerInstance->packedBytesLength);
}

void BKDReader::IndexTree::pushLeft()
{
  nodeID *= 2;
  level++;
  if (splitPackedValueStack[level] == nullptr) {
    splitPackedValueStack[level] =
        std::deque<char>(outerInstance->packedBytesLength);
  }
}

void BKDReader::IndexTree::pushRight()
{
  nodeID = nodeID * 2 + 1;
  level++;
  if (splitPackedValueStack[level] == nullptr) {
    splitPackedValueStack[level] =
        std::deque<char>(outerInstance->packedBytesLength);
  }
}

void BKDReader::IndexTree::pop()
{
  nodeID /= 2;
  level--;
  splitDim = -1;
  // System.out.println("  pop nodeID=" + nodeID);
}

bool BKDReader::IndexTree::isLeafNode()
{
  return nodeID >= outerInstance->leafNodeOffset;
}

bool BKDReader::IndexTree::nodeExists()
{
  return nodeID - outerInstance->leafNodeOffset < outerInstance->leafNodeOffset;
}

int BKDReader::IndexTree::getNodeID() { return nodeID; }

std::deque<char> BKDReader::IndexTree::getSplitPackedValue()
{
  assert(isLeafNode() == false);
  assert(
      (splitPackedValueStack[level] != nullptr, L"level=" + to_wstring(level)));
  return splitPackedValueStack[level];
}

int BKDReader::IndexTree::getSplitDim()
{
  assert(isLeafNode() == false);
  return splitDim;
}

int BKDReader::IndexTree::getNumLeaves()
{
  int leftMostLeafNode = nodeID;
  while (leftMostLeafNode < outerInstance->leafNodeOffset) {
    leftMostLeafNode = leftMostLeafNode * 2;
  }
  int rightMostLeafNode = nodeID;
  while (rightMostLeafNode < outerInstance->leafNodeOffset) {
    rightMostLeafNode = rightMostLeafNode * 2 + 1;
  }
  constexpr int numLeaves;
  if (rightMostLeafNode >= leftMostLeafNode) {
    // both are on the same level
    numLeaves = rightMostLeafNode - leftMostLeafNode + 1;
  } else {
    // left is one level deeper than right
    numLeaves = rightMostLeafNode - leftMostLeafNode + 1 +
                outerInstance->leafNodeOffset;
  }
  assert((numLeaves == getNumLeavesSlow(nodeID),
          to_wstring(numLeaves) + L" " + to_wstring(getNumLeavesSlow(nodeID))));
  return numLeaves;
}

int BKDReader::IndexTree::getNumLeavesSlow(int node)
{
  if (node >= 2 * outerInstance->leafNodeOffset) {
    return 0;
  } else if (node >= outerInstance->leafNodeOffset) {
    return 1;
  } else {
    constexpr int leftCount = getNumLeavesSlow(node * 2);
    constexpr int rightCount = getNumLeavesSlow(node * 2 + 1);
    return leftCount + rightCount;
  }
}

BKDReader::LegacyIndexTree::LegacyIndexTree(shared_ptr<BKDReader> outerInstance)
    : IndexTree(outerInstance), outerInstance(outerInstance)
{
  setNodeData();
  scratch->bytes = splitDimValue;
  scratch->length = outerInstance->bytesPerDim;
}

shared_ptr<LegacyIndexTree> BKDReader::LegacyIndexTree::clone()
{
  shared_ptr<LegacyIndexTree> index =
      make_shared<LegacyIndexTree>(outerInstance);
  index->nodeID = nodeID;
  index->level = level;
  index->splitDim = splitDim;
  index->leafBlockFP = leafBlockFP;
  index->splitPackedValueStack[index->level] =
      splitPackedValueStack[index->level].clone();

  return index;
}

void BKDReader::LegacyIndexTree::pushLeft()
{
  IndexTree::pushLeft();
  setNodeData();
}

void BKDReader::LegacyIndexTree::pushRight()
{
  IndexTree::pushRight();
  setNodeData();
}

void BKDReader::LegacyIndexTree::setNodeData()
{
  if (isLeafNode()) {
    leafBlockFP =
        outerInstance->leafBlockFPs[nodeID - outerInstance->leafNodeOffset];
    splitDim = -1;
  } else {
    leafBlockFP = -1;
    int address = nodeID * outerInstance->bytesPerIndexEntry;
    if (outerInstance->numDims == 1) {
      splitDim = 0;
      if (outerInstance->version < BKDWriter::VERSION_IMPLICIT_SPLIT_DIM_1D) {
        // skip over wastefully encoded 0 splitDim:
        assert(outerInstance->splitPackedValues[address] == 0);
        address++;
      }
    } else {
      splitDim = outerInstance->splitPackedValues[address++] & 0xff;
    }
    System::arraycopy(outerInstance->splitPackedValues, address, splitDimValue,
                      0, outerInstance->bytesPerDim);
  }
}

int64_t BKDReader::LegacyIndexTree::getLeafBlockFP()
{
  assert(isLeafNode());
  return leafBlockFP;
}

shared_ptr<BytesRef> BKDReader::LegacyIndexTree::getSplitDimValue()
{
  assert(isLeafNode() == false);
  return scratch;
}

void BKDReader::LegacyIndexTree::pop()
{
  IndexTree::pop();
  leafBlockFP = -1;
}

BKDReader::PackedIndexTree::PackedIndexTree(shared_ptr<BKDReader> outerInstance)
    : IndexTree(outerInstance),
      in_(make_shared<ByteArrayDataInput>(outerInstance->packedIndex)),
      leafBlockFPStack(std::deque<int64_t>(treeDepth + 1)),
      leftNodePositions(std::deque<int>(treeDepth + 1)),
      rightNodePositions(std::deque<int>(treeDepth + 1)),
      splitDims(std::deque<int>(treeDepth + 1)),
      negativeDeltas(
          std::deque<bool>(outerInstance->numDims * (treeDepth + 1))),
      splitValuesStack(std::deque<std::deque<char>>(treeDepth + 1)),
      scratch(make_shared<BytesRef>()), outerInstance(outerInstance)
{
  int treeDepth = outerInstance->getTreeDepth();

  splitValuesStack[0] = std::deque<char>(outerInstance->packedBytesLength);
  readNodeData(false);
  scratch->length = outerInstance->bytesPerDim;
}

shared_ptr<PackedIndexTree> BKDReader::PackedIndexTree::clone()
{
  shared_ptr<PackedIndexTree> index =
      make_shared<PackedIndexTree>(outerInstance);
  index->nodeID = nodeID;
  index->level = level;
  index->splitDim = splitDim;
  index->leafBlockFPStack[level] = leafBlockFPStack[level];
  index->leftNodePositions[level] = leftNodePositions[level];
  index->rightNodePositions[level] = rightNodePositions[level];
  index->splitValuesStack[index->level] =
      splitValuesStack[index->level].clone();
  System::arraycopy(negativeDeltas, level * outerInstance->numDims,
                    index->negativeDeltas, level * outerInstance->numDims,
                    outerInstance->numDims);
  index->splitDims[level] = splitDims[level];
  return index;
}

void BKDReader::PackedIndexTree::pushLeft()
{
  int nodePosition = leftNodePositions[level];
  IndexTree::pushLeft();
  System::arraycopy(negativeDeltas, (level - 1) * outerInstance->numDims,
                    negativeDeltas, level * outerInstance->numDims,
                    outerInstance->numDims);
  assert(splitDim != -1);
  negativeDeltas[level * outerInstance->numDims + splitDim] = true;
  in_->setPosition(nodePosition);
  readNodeData(true);
}

void BKDReader::PackedIndexTree::pushRight()
{
  int nodePosition = rightNodePositions[level];
  IndexTree::pushRight();
  System::arraycopy(negativeDeltas, (level - 1) * outerInstance->numDims,
                    negativeDeltas, level * outerInstance->numDims,
                    outerInstance->numDims);
  assert(splitDim != -1);
  negativeDeltas[level * outerInstance->numDims + splitDim] = false;
  in_->setPosition(nodePosition);
  readNodeData(false);
}

void BKDReader::PackedIndexTree::pop()
{
  IndexTree::pop();
  splitDim = splitDims[level];
}

int64_t BKDReader::PackedIndexTree::getLeafBlockFP()
{
  assert((isLeafNode(), L"nodeID=" + to_wstring(nodeID) + L" is not a leaf"));
  return leafBlockFPStack[level];
}

shared_ptr<BytesRef> BKDReader::PackedIndexTree::getSplitDimValue()
{
  assert(isLeafNode() == false);
  scratch->bytes = splitValuesStack[level];
  scratch->offset = splitDim * outerInstance->bytesPerDim;
  return scratch;
}

void BKDReader::PackedIndexTree::readNodeData(bool isLeft)
{

  leafBlockFPStack[level] = leafBlockFPStack[level - 1];

  // read leaf block FP delta
  if (isLeft == false) {
    leafBlockFPStack[level] += in_->readVLong();
  }

  if (isLeafNode()) {
    splitDim = -1;
  } else {

    // read split dim, prefix, firstDiffByteDelta encoded as int:
    int code = in_->readVInt();
    splitDim = code % outerInstance->numDims;
    splitDims[level] = splitDim;
    code /= outerInstance->numDims;
    int prefix = code % (1 + outerInstance->bytesPerDim);
    int suffix = outerInstance->bytesPerDim - prefix;

    if (splitValuesStack[level] == nullptr) {
      splitValuesStack[level] =
          std::deque<char>(outerInstance->packedBytesLength);
    }
    System::arraycopy(splitValuesStack[level - 1], 0, splitValuesStack[level],
                      0, outerInstance->packedBytesLength);
    if (suffix > 0) {
      int firstDiffByteDelta = code / (1 + outerInstance->bytesPerDim);
      if (negativeDeltas[level * outerInstance->numDims + splitDim]) {
        firstDiffByteDelta = -firstDiffByteDelta;
      }
      int oldByte =
          splitValuesStack[level]
                          [splitDim * outerInstance->bytesPerDim + prefix] &
          0xFF;
      splitValuesStack[level][splitDim * outerInstance->bytesPerDim + prefix] =
          static_cast<char>(oldByte + firstDiffByteDelta);
      in_->readBytes(splitValuesStack[level],
                     splitDim * outerInstance->bytesPerDim + prefix + 1,
                     suffix - 1);
    } else {
      // our split value is == last split value in this dim, which can happen
      // when there are many duplicate values
    }

    int leftNumBytes;
    if (nodeID * 2 < outerInstance->leafNodeOffset) {
      leftNumBytes = in_->readVInt();
    } else {
      leftNumBytes = 0;
    }

    leftNodePositions[level] = in_->getPosition();
    rightNodePositions[level] = leftNodePositions[level] + leftNumBytes;
  }
}

int BKDReader::getTreeDepth()
{
  // First +1 because all the non-leave nodes makes another power
  // of 2; e.g. to have a fully balanced tree with 4 leaves you
  // need a depth=3 tree:

  // Second +1 because MathUtil.log computes floor of the logarithm; e.g.
  // with 5 leaves you need a depth=4 tree:
  return MathUtil::log(numLeaves, 2) + 2;
}

BKDReader::IntersectState::IntersectState(
    shared_ptr<IndexInput> in_, int numDims, int packedBytesLength,
    int maxPointsInLeafNode, shared_ptr<PointValues::IntersectVisitor> visitor,
    shared_ptr<IndexTree> indexVisitor)
    : in_(in_), scratchDocIDs(std::deque<int>(maxPointsInLeafNode)),
      scratchPackedValue(std::deque<char>(packedBytesLength)),
      commonPrefixLengths(std::deque<int>(numDims)), visitor(visitor),
      index(indexVisitor)
{
}

void BKDReader::intersect(
    shared_ptr<PointValues::IntersectVisitor> visitor) 
{
  intersect(getIntersectState(visitor), minPackedValue, maxPackedValue);
}

int64_t
BKDReader::estimatePointCount(shared_ptr<PointValues::IntersectVisitor> visitor)
{
  return estimatePointCount(getIntersectState(visitor), minPackedValue,
                            maxPackedValue);
}

void BKDReader::addAll(shared_ptr<IntersectState> state,
                       bool grown) 
{
  // System.out.println("R: addAll nodeID=" + nodeID);

  if (grown == false) {
    constexpr int64_t maxPointCount =
        static_cast<int64_t>(maxPointsInLeafNode) *
        state->index->getNumLeaves();
    if (maxPointCount <=
        numeric_limits<int>::max()) { // could be >MAX_VALUE if there are more
                                      // than 2B points in total
      state->visitor->grow(static_cast<int>(maxPointCount));
      grown = true;
    }
  }

  if (state->index->isLeafNode()) {
    assert(grown);
    // System.out.println("ADDALL");
    if (state->index->nodeExists()) {
      visitDocIDs(state->in_, state->index->getLeafBlockFP(), state->visitor);
    }
    // TODO: we can assert that the first value here in fact matches what the
    // index claimed?
  } else {
    state->index->pushLeft();
    addAll(state, grown);
    state->index->pop();

    state->index->pushRight();
    addAll(state, grown);
    state->index->pop();
  }
}

shared_ptr<IntersectState>
BKDReader::getIntersectState(shared_ptr<PointValues::IntersectVisitor> visitor)
{
  shared_ptr<IndexTree> index;
  if (packedIndex.size() > 0) {
    index = make_shared<PackedIndexTree>(shared_from_this());
  } else {
    index = make_shared<LegacyIndexTree>(shared_from_this());
  }
  return make_shared<IntersectState>(in_->clone(), numDims, packedBytesLength,
                                     maxPointsInLeafNode, visitor, index);
}

void BKDReader::visitLeafBlockValues(
    shared_ptr<IndexTree> index,
    shared_ptr<IntersectState> state) 
{

  // Leaf node; scan and filter all points in this block:
  int count =
      readDocIDs(state->in_, index->getLeafBlockFP(), state->scratchDocIDs);

  // Again, this time reading values and checking with the visitor
  visitDocValues(state->commonPrefixLengths, state->scratchPackedValue,
                 state->in_, state->scratchDocIDs, count, state->visitor);
}

void BKDReader::visitDocIDs(
    shared_ptr<IndexInput> in_, int64_t blockFP,
    shared_ptr<PointValues::IntersectVisitor> visitor) 
{
  // Leaf node
  in_->seek(blockFP);

  // How many points are stored in this leaf cell:
  int count = in_->readVInt();
  // No need to call grow(), it has been called up-front

  if (version < BKDWriter::VERSION_COMPRESSED_DOC_IDS) {
    DocIdsWriter::readInts32(in_, count, visitor);
  } else {
    DocIdsWriter::readInts(in_, count, visitor);
  }
}

int BKDReader::readDocIDs(shared_ptr<IndexInput> in_, int64_t blockFP,
                          std::deque<int> &docIDs) 
{
  in_->seek(blockFP);

  // How many points are stored in this leaf cell:
  int count = in_->readVInt();

  if (version < BKDWriter::VERSION_COMPRESSED_DOC_IDS) {
    DocIdsWriter::readInts32(in_, count, docIDs);
  } else {
    DocIdsWriter::readInts(in_, count, docIDs);
  }

  return count;
}

void BKDReader::visitDocValues(
    std::deque<int> &commonPrefixLengths,
    std::deque<char> &scratchPackedValue, shared_ptr<IndexInput> in_,
    std::deque<int> &docIDs, int count,
    shared_ptr<PointValues::IntersectVisitor> visitor) 
{
  visitor->grow(count);

  readCommonPrefixes(commonPrefixLengths, scratchPackedValue, in_);

  int compressedDim = version < BKDWriter::VERSION_COMPRESSED_VALUES
                          ? -1
                          : readCompressedDim(in_);

  if (compressedDim == -1) {
    visitRawDocValues(commonPrefixLengths, scratchPackedValue, in_, docIDs,
                      count, visitor);
  } else {
    visitCompressedDocValues(commonPrefixLengths, scratchPackedValue, in_,
                             docIDs, count, visitor, compressedDim);
  }
}

void BKDReader::visitRawDocValues(
    std::deque<int> &commonPrefixLengths,
    std::deque<char> &scratchPackedValue, shared_ptr<IndexInput> in_,
    std::deque<int> &docIDs, int count,
    shared_ptr<PointValues::IntersectVisitor> visitor) 
{
  for (int i = 0; i < count; ++i) {
    for (int dim = 0; dim < numDims; dim++) {
      int prefix = commonPrefixLengths[dim];
      in_->readBytes(scratchPackedValue, dim * bytesPerDim + prefix,
                     bytesPerDim - prefix);
    }
    visitor->visit(docIDs[i], scratchPackedValue);
  }
}

void BKDReader::visitCompressedDocValues(
    std::deque<int> &commonPrefixLengths,
    std::deque<char> &scratchPackedValue, shared_ptr<IndexInput> in_,
    std::deque<int> &docIDs, int count,
    shared_ptr<PointValues::IntersectVisitor> visitor,
    int compressedDim) 
{
  // the byte at `compressedByteOffset` is compressed using run-length
  // compression, other suffix bytes are stored verbatim
  constexpr int compressedByteOffset =
      compressedDim * bytesPerDim + commonPrefixLengths[compressedDim];
  commonPrefixLengths[compressedDim]++;
  int i;
  for (i = 0; i < count;) {
    scratchPackedValue[compressedByteOffset] = in_->readByte();
    constexpr int runLen = Byte::toUnsignedInt(in_->readByte());
    for (int j = 0; j < runLen; ++j) {
      for (int dim = 0; dim < numDims; dim++) {
        int prefix = commonPrefixLengths[dim];
        in_->readBytes(scratchPackedValue, dim * bytesPerDim + prefix,
                       bytesPerDim - prefix);
      }
      visitor->visit(docIDs[i + j], scratchPackedValue);
    }
    i += runLen;
  }
  if (i != count) {
    throw make_shared<CorruptIndexException>(
        L"Sub blocks do not add up to the expected count: " +
            to_wstring(count) + L" != " + to_wstring(i),
        in_);
  }
}

int BKDReader::readCompressedDim(shared_ptr<IndexInput> in_) 
{
  int compressedDim = in_->readByte();
  if (compressedDim < -1 || compressedDim >= numDims) {
    throw make_shared<CorruptIndexException>(
        L"Got compressedDim=" + to_wstring(compressedDim), in_);
  }
  return compressedDim;
}

void BKDReader::readCommonPrefixes(
    std::deque<int> &commonPrefixLengths,
    std::deque<char> &scratchPackedValue,
    shared_ptr<IndexInput> in_) 
{
  for (int dim = 0; dim < numDims; dim++) {
    int prefix = in_->readVInt();
    commonPrefixLengths[dim] = prefix;
    if (prefix > 0) {
      in_->readBytes(scratchPackedValue, dim * bytesPerDim, prefix);
    }
    // System.out.println("R: " + dim + " of " + numDims + " prefix=" + prefix);
  }
}

void BKDReader::intersect(shared_ptr<IntersectState> state,
                          std::deque<char> &cellMinPacked,
                          std::deque<char> &cellMaxPacked) 
{

  /*
  System.out.println("\nR: intersect nodeID=" + state.index.getNodeID());
  for(int dim=0;dim<numDims;dim++) {
    System.out.println("  dim=" + dim + "\n    cellMin=" + new
  BytesRef(cellMinPacked, dim*bytesPerDim, bytesPerDim) + "\n    cellMax=" + new
  BytesRef(cellMaxPacked, dim*bytesPerDim, bytesPerDim));
  }
  */

  Relation r = state->visitor->compare(cellMinPacked, cellMaxPacked);

  if (r == Relation::CELL_OUTSIDE_QUERY) {
    // This cell is fully outside of the query shape: stop recursing
  } else if (r == Relation::CELL_INSIDE_QUERY) {
    // This cell is fully inside of the query shape: recursively add all points
    // in this cell without filtering
    addAll(state, false);
    // The cell crosses the shape boundary, or the cell fully contains the
    // query, so we fall through and do full filtering:
  } else if (state->index->isLeafNode()) {

    // TODO: we can assert that the first value here in fact matches what the
    // index claimed?

    // In the unbalanced case it's possible the left most node only has one
    // child:
    if (state->index->nodeExists()) {
      // Leaf node; scan and filter all points in this block:
      int count = readDocIDs(state->in_, state->index->getLeafBlockFP(),
                             state->scratchDocIDs);

      // Again, this time reading values and checking with the visitor
      visitDocValues(state->commonPrefixLengths, state->scratchPackedValue,
                     state->in_, state->scratchDocIDs, count, state->visitor);
    }

  } else {

    // Non-leaf node: recurse on the split left and right nodes
    int splitDim = state->index->getSplitDim();
    assert((splitDim >= 0, L"splitDim=" + to_wstring(splitDim)));
    assert(splitDim < numDims);

    std::deque<char> splitPackedValue = state->index->getSplitPackedValue();
    shared_ptr<BytesRef> splitDimValue = state->index->getSplitDimValue();
    assert(splitDimValue->length == bytesPerDim);
    // System.out.println("  splitDimValue=" + splitDimValue + " splitDim=" +
    // splitDim);

    // make sure cellMin <= splitValue <= cellMax:
    assert((StringHelper::compare(bytesPerDim, cellMinPacked,
                                  splitDim * bytesPerDim, splitDimValue->bytes,
                                  splitDimValue->offset) <= 0,
            L"bytesPerDim=" + to_wstring(bytesPerDim) + L" splitDim=" +
                to_wstring(splitDim) + L" numDims=" + to_wstring(numDims)));
    assert((StringHelper::compare(bytesPerDim, cellMaxPacked,
                                  splitDim * bytesPerDim, splitDimValue->bytes,
                                  splitDimValue->offset) >= 0,
            L"bytesPerDim=" + to_wstring(bytesPerDim) + L" splitDim=" +
                to_wstring(splitDim) + L" numDims=" + to_wstring(numDims)));

    // Recurse on left sub-tree:
    System::arraycopy(cellMaxPacked, 0, splitPackedValue, 0, packedBytesLength);
    System::arraycopy(splitDimValue->bytes, splitDimValue->offset,
                      splitPackedValue, splitDim * bytesPerDim, bytesPerDim);
    state->index->pushLeft();
    intersect(state, cellMinPacked, splitPackedValue);
    state->index->pop();

    // Restore the split dim value since it may have been overwritten while
    // recursing:
    System::arraycopy(splitPackedValue, splitDim * bytesPerDim,
                      splitDimValue->bytes, splitDimValue->offset, bytesPerDim);

    // Recurse on right sub-tree:
    System::arraycopy(cellMinPacked, 0, splitPackedValue, 0, packedBytesLength);
    System::arraycopy(splitDimValue->bytes, splitDimValue->offset,
                      splitPackedValue, splitDim * bytesPerDim, bytesPerDim);
    state->index->pushRight();
    intersect(state, splitPackedValue, cellMaxPacked);
    state->index->pop();
  }
}

int64_t BKDReader::estimatePointCount(shared_ptr<IntersectState> state,
                                        std::deque<char> &cellMinPacked,
                                        std::deque<char> &cellMaxPacked)
{

  /*
  System.out.println("\nR: intersect nodeID=" + state.index.getNodeID());
  for(int dim=0;dim<numDims;dim++) {
    System.out.println("  dim=" + dim + "\n    cellMin=" + new
  BytesRef(cellMinPacked, dim*bytesPerDim, bytesPerDim) + "\n    cellMax=" + new
  BytesRef(cellMaxPacked, dim*bytesPerDim, bytesPerDim));
  }
  */

  Relation r = state->visitor->compare(cellMinPacked, cellMaxPacked);

  if (r == Relation::CELL_OUTSIDE_QUERY) {
    // This cell is fully outside of the query shape: stop recursing
    return 0LL;
  } else if (r == Relation::CELL_INSIDE_QUERY) {
    return static_cast<int64_t>(maxPointsInLeafNode) *
           state->index->getNumLeaves();
  } else if (state->index->isLeafNode()) {
    // Assume half the points matched
    return (maxPointsInLeafNode + 1) / 2;
  } else {

    // Non-leaf node: recurse on the split left and right nodes
    int splitDim = state->index->getSplitDim();
    assert((splitDim >= 0, L"splitDim=" + to_wstring(splitDim)));
    assert(splitDim < numDims);

    std::deque<char> splitPackedValue = state->index->getSplitPackedValue();
    shared_ptr<BytesRef> splitDimValue = state->index->getSplitDimValue();
    assert(splitDimValue->length == bytesPerDim);
    // System.out.println("  splitDimValue=" + splitDimValue + " splitDim=" +
    // splitDim);

    // make sure cellMin <= splitValue <= cellMax:
    assert((StringHelper::compare(bytesPerDim, cellMinPacked,
                                  splitDim * bytesPerDim, splitDimValue->bytes,
                                  splitDimValue->offset) <= 0,
            L"bytesPerDim=" + to_wstring(bytesPerDim) + L" splitDim=" +
                to_wstring(splitDim) + L" numDims=" + to_wstring(numDims)));
    assert((StringHelper::compare(bytesPerDim, cellMaxPacked,
                                  splitDim * bytesPerDim, splitDimValue->bytes,
                                  splitDimValue->offset) >= 0,
            L"bytesPerDim=" + to_wstring(bytesPerDim) + L" splitDim=" +
                to_wstring(splitDim) + L" numDims=" + to_wstring(numDims)));

    // Recurse on left sub-tree:
    System::arraycopy(cellMaxPacked, 0, splitPackedValue, 0, packedBytesLength);
    System::arraycopy(splitDimValue->bytes, splitDimValue->offset,
                      splitPackedValue, splitDim * bytesPerDim, bytesPerDim);
    state->index->pushLeft();
    constexpr int64_t leftCost =
        estimatePointCount(state, cellMinPacked, splitPackedValue);
    state->index->pop();

    // Restore the split dim value since it may have been overwritten while
    // recursing:
    System::arraycopy(splitPackedValue, splitDim * bytesPerDim,
                      splitDimValue->bytes, splitDimValue->offset, bytesPerDim);

    // Recurse on right sub-tree:
    System::arraycopy(cellMinPacked, 0, splitPackedValue, 0, packedBytesLength);
    System::arraycopy(splitDimValue->bytes, splitDimValue->offset,
                      splitPackedValue, splitDim * bytesPerDim, bytesPerDim);
    state->index->pushRight();
    constexpr int64_t rightCost =
        estimatePointCount(state, splitPackedValue, cellMaxPacked);
    state->index->pop();
    return leftCost + rightCost;
  }
}

int64_t BKDReader::ramBytesUsed()
{
  if (packedIndex.size() > 0) {
    return packedIndex.size();
  } else {
    return RamUsageEstimator::sizeOf(splitPackedValues) +
           RamUsageEstimator::sizeOf(leafBlockFPs);
  }
}

std::deque<char> BKDReader::getMinPackedValue()
{
  return minPackedValue.clone();
}

std::deque<char> BKDReader::getMaxPackedValue()
{
  return maxPackedValue.clone();
}

int BKDReader::getNumDimensions() { return numDims; }

int BKDReader::getBytesPerDimension() { return bytesPerDim; }

int64_t BKDReader::size() { return pointCount; }

int BKDReader::getDocCount() { return docCount; }

bool BKDReader::isLeafNode(int nodeID) { return nodeID >= leafNodeOffset; }
} // namespace org::apache::lucene::util::bkd