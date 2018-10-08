using namespace std;

#include "SimpleTextBKDReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/bkd/BKDReader.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using SimpleTextUtil = org::apache::lucene::codecs::simpletext::SimpleTextUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using PointValues = org::apache::lucene::index::PointValues;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;
using BKDReader = org::apache::lucene::util::bkd::BKDReader;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_COUNT;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_DOC_ID;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextPointsWriter.BLOCK_VALUE;

SimpleTextBKDReader::SimpleTextBKDReader(
    shared_ptr<IndexInput> in_, int numDims, int maxPointsInLeafNode,
    int bytesPerDim, std::deque<int64_t> &leafBlockFPs,
    std::deque<char> &splitPackedValues, std::deque<char> &minPackedValue,
    std::deque<char> &maxPackedValue, int64_t pointCount,
    int docCount) 
    : splitPackedValues(splitPackedValues), leafBlockFPs(leafBlockFPs),
      leafNodeOffset(leafBlockFPs.size()), numDims(numDims),
      bytesPerDim(bytesPerDim),
      bytesPerIndexEntry(numDims == 1 ? bytesPerDim : bytesPerDim + 1),
      in_(in_), maxPointsInLeafNode(maxPointsInLeafNode),
      minPackedValue(minPackedValue), maxPackedValue(maxPackedValue),
      pointCount(pointCount), docCount(docCount),
      version(SimpleTextBKDWriter::VERSION_CURRENT),
      packedBytesLength(numDims * bytesPerDim)
{
  // no version check here because callers of this API (SimpleText) have no back
  // compat:
  assert(minPackedValue.size() == packedBytesLength);
  assert(maxPackedValue.size() == packedBytesLength);
}

SimpleTextBKDReader::IntersectState::IntersectState(
    shared_ptr<IndexInput> in_, int numDims, int packedBytesLength,
    int maxPointsInLeafNode, shared_ptr<PointValues::IntersectVisitor> visitor)
    : in_(in_), scratchDocIDs(std::deque<int>(maxPointsInLeafNode)),
      scratchPackedValue(std::deque<char>(packedBytesLength)),
      commonPrefixLengths(std::deque<int>(numDims)), visitor(visitor)
{
}

void SimpleTextBKDReader::intersect(
    shared_ptr<PointValues::IntersectVisitor> visitor) 
{
  intersect(getIntersectState(visitor), 1, minPackedValue, maxPackedValue);
}

void SimpleTextBKDReader::addAll(shared_ptr<BKDReader::IntersectState> state,
                                 int nodeID) 
{
  // System.out.println("R: addAll nodeID=" + nodeID);

  if (nodeID >= leafNodeOffset) {
    // System.out.println("ADDALL");
    visitDocIDs(state->in_, leafBlockFPs[nodeID - leafNodeOffset],
                state->visitor);
    // TODO: we can assert that the first value here in fact matches what the
    // index claimed?
  } else {
    addAll(state, 2 * nodeID);
    addAll(state, 2 * nodeID + 1);
  }
}

shared_ptr<BKDReader::IntersectState> SimpleTextBKDReader::getIntersectState(
    shared_ptr<PointValues::IntersectVisitor> visitor)
{
  return make_shared<BKDReader::IntersectState>(
      in_->clone(), numDims, packedBytesLength, maxPointsInLeafNode, visitor);
}

void SimpleTextBKDReader::visitLeafBlockValues(
    int nodeID, shared_ptr<BKDReader::IntersectState> state) 
{
  int leafID = nodeID - leafNodeOffset;

  // Leaf node; scan and filter all points in this block:
  int count =
      readDocIDs(state->in_, leafBlockFPs[leafID], state->scratchDocIDs);

  // Again, this time reading values and checking with the visitor
  visitDocValues(state->commonPrefixLengths, state->scratchPackedValue,
                 state->in_, state->scratchDocIDs, count, state->visitor);
}

void SimpleTextBKDReader::visitDocIDs(
    shared_ptr<IndexInput> in_, int64_t blockFP,
    shared_ptr<PointValues::IntersectVisitor> visitor) 
{
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  in_->seek(blockFP);
  readLine(in_, scratch);
  int count = parseInt(scratch, BLOCK_COUNT);
  visitor->grow(count);
  for (int i = 0; i < count; i++) {
    readLine(in_, scratch);
    visitor->visit(parseInt(scratch, BLOCK_DOC_ID));
  }
}

int SimpleTextBKDReader::readDocIDs(shared_ptr<IndexInput> in_,
                                    int64_t blockFP,
                                    std::deque<int> &docIDs) 
{
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  in_->seek(blockFP);
  readLine(in_, scratch);
  int count = parseInt(scratch, BLOCK_COUNT);
  for (int i = 0; i < count; i++) {
    readLine(in_, scratch);
    docIDs[i] = parseInt(scratch, BLOCK_DOC_ID);
  }
  return count;
}

void SimpleTextBKDReader::visitDocValues(
    std::deque<int> &commonPrefixLengths,
    std::deque<char> &scratchPackedValue, shared_ptr<IndexInput> in_,
    std::deque<int> &docIDs, int count,
    shared_ptr<PointValues::IntersectVisitor> visitor) 
{
  visitor->grow(count);
  // NOTE: we don't do prefix coding, so we ignore commonPrefixLengths
  assert(scratchPackedValue.size() == packedBytesLength);
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  for (int i = 0; i < count; i++) {
    readLine(in_, scratch);
    assert((startsWith(scratch, BLOCK_VALUE)));
    shared_ptr<BytesRef> br =
        SimpleTextUtil::fromBytesRefString(stripPrefix(scratch, BLOCK_VALUE));
    assert(br->length == packedBytesLength);
    System::arraycopy(br->bytes, br->offset, scratchPackedValue, 0,
                      packedBytesLength);
    visitor->visit(docIDs[i], scratchPackedValue);
  }
}

void SimpleTextBKDReader::visitCompressedDocValues(
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

int SimpleTextBKDReader::readCompressedDim(shared_ptr<IndexInput> in_) throw(
    IOException)
{
  int compressedDim = in_->readByte();
  if (compressedDim < -1 || compressedDim >= numDims) {
    throw make_shared<CorruptIndexException>(
        L"Got compressedDim=" + to_wstring(compressedDim), in_);
  }
  return compressedDim;
}

void SimpleTextBKDReader::readCommonPrefixes(
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

void SimpleTextBKDReader::intersect(
    shared_ptr<BKDReader::IntersectState> state, int nodeID,
    std::deque<char> &cellMinPacked,
    std::deque<char> &cellMaxPacked) 
{

  /*
  System.out.println("\nR: intersect nodeID=" + nodeID);
  for(int dim=0;dim<numDims;dim++) {
    System.out.println("  dim=" + dim + "\n    cellMin=" + new
  BytesRef(cellMinPacked, dim*bytesPerDim, bytesPerDim) + "\n    cellMax=" + new
  BytesRef(cellMaxPacked, dim*bytesPerDim, bytesPerDim));
  }
  */

  Relation r = state->visitor->compare(cellMinPacked, cellMaxPacked);

  if (r == Relation::CELL_OUTSIDE_QUERY) {
    // This cell is fully outside of the query shape: stop recursing
    return;
  } else if (r == Relation::CELL_INSIDE_QUERY) {
    // This cell is fully inside of the query shape: recursively add all points
    // in this cell without filtering
    addAll(state, nodeID);
    return;
  } else {
    // The cell crosses the shape boundary, or the cell fully contains the
    // query, so we fall through and do full filtering
  }

  if (nodeID >= leafNodeOffset) {
    // TODO: we can assert that the first value here in fact matches what the
    // index claimed?

    int leafID = nodeID - leafNodeOffset;

    // In the unbalanced case it's possible the left most node only has one
    // child:
    if (leafID < leafBlockFPs.size()) {
      // Leaf node; scan and filter all points in this block:
      int count =
          readDocIDs(state->in_, leafBlockFPs[leafID], state->scratchDocIDs);

      // Again, this time reading values and checking with the visitor
      visitDocValues(state->commonPrefixLengths, state->scratchPackedValue,
                     state->in_, state->scratchDocIDs, count, state->visitor);
    }

  } else {

    // Non-leaf node: recurse on the split left and right nodes

    int address = nodeID * bytesPerIndexEntry;
    int splitDim;
    if (numDims == 1) {
      splitDim = 0;
    } else {
      splitDim = splitPackedValues[address++] & 0xff;
    }

    assert(splitDim < numDims);

    // TODO: can we alloc & reuse this up front?

    std::deque<char> splitPackedValue(packedBytesLength);

    // Recurse on left sub-tree:
    System::arraycopy(cellMaxPacked, 0, splitPackedValue, 0, packedBytesLength);
    System::arraycopy(splitPackedValues, address, splitPackedValue,
                      splitDim * bytesPerDim, bytesPerDim);
    intersect(state, 2 * nodeID, cellMinPacked, splitPackedValue);

    // Recurse on right sub-tree:
    System::arraycopy(cellMinPacked, 0, splitPackedValue, 0, packedBytesLength);
    System::arraycopy(splitPackedValues, address, splitPackedValue,
                      splitDim * bytesPerDim, bytesPerDim);
    intersect(state, 2 * nodeID + 1, splitPackedValue, cellMaxPacked);
  }
}

int64_t SimpleTextBKDReader::estimatePointCount(
    shared_ptr<PointValues::IntersectVisitor> visitor)
{
  return estimatePointCount(getIntersectState(visitor), 1, minPackedValue,
                            maxPackedValue);
}

int64_t SimpleTextBKDReader::estimatePointCount(
    shared_ptr<BKDReader::IntersectState> state, int nodeID,
    std::deque<char> &cellMinPacked, std::deque<char> &cellMaxPacked)
{
  Relation r = state->visitor->compare(cellMinPacked, cellMaxPacked);

  if (r == Relation::CELL_OUTSIDE_QUERY) {
    // This cell is fully outside of the query shape: stop recursing
    return 0LL;
  } else if (nodeID >= leafNodeOffset) {
    // Assume all points match and there are no dups
    return maxPointsInLeafNode;
  } else {

    // Non-leaf node: recurse on the split left and right nodes

    int address = nodeID * bytesPerIndexEntry;
    int splitDim;
    if (numDims == 1) {
      splitDim = 0;
    } else {
      splitDim = splitPackedValues[address++] & 0xff;
    }

    assert(splitDim < numDims);

    // TODO: can we alloc & reuse this up front?

    std::deque<char> splitPackedValue(packedBytesLength);

    // Recurse on left sub-tree:
    System::arraycopy(cellMaxPacked, 0, splitPackedValue, 0, packedBytesLength);
    System::arraycopy(splitPackedValues, address, splitPackedValue,
                      splitDim * bytesPerDim, bytesPerDim);
    constexpr int64_t leftCost =
        estimatePointCount(state, 2 * nodeID, cellMinPacked, splitPackedValue);

    // Recurse on right sub-tree:
    System::arraycopy(cellMinPacked, 0, splitPackedValue, 0, packedBytesLength);
    System::arraycopy(splitPackedValues, address, splitPackedValue,
                      splitDim * bytesPerDim, bytesPerDim);
    constexpr int64_t rightCost = estimatePointCount(
        state, 2 * nodeID + 1, splitPackedValue, cellMaxPacked);
    return leftCost + rightCost;
  }
}

void SimpleTextBKDReader::copySplitValue(int nodeID,
                                         std::deque<char> &splitPackedValue)
{
  int address = nodeID * bytesPerIndexEntry;
  int splitDim;
  if (numDims == 1) {
    splitDim = 0;
  } else {
    splitDim = splitPackedValues[address++] & 0xff;
  }

  assert(splitDim < numDims);
  System::arraycopy(splitPackedValues, address, splitPackedValue,
                    splitDim * bytesPerDim, bytesPerDim);
}

int64_t SimpleTextBKDReader::ramBytesUsed()
{
  return RamUsageEstimator::sizeOf(splitPackedValues) +
         RamUsageEstimator::sizeOf(leafBlockFPs);
}

std::deque<char> SimpleTextBKDReader::getMinPackedValue()
{
  return minPackedValue.clone();
}

std::deque<char> SimpleTextBKDReader::getMaxPackedValue()
{
  return maxPackedValue.clone();
}

int SimpleTextBKDReader::getNumDimensions() { return numDims; }

int SimpleTextBKDReader::getBytesPerDimension() { return bytesPerDim; }

int64_t SimpleTextBKDReader::size() { return pointCount; }

int SimpleTextBKDReader::getDocCount() { return docCount; }

bool SimpleTextBKDReader::isLeafNode(int nodeID)
{
  return nodeID >= leafNodeOffset;
}

int SimpleTextBKDReader::parseInt(shared_ptr<BytesRefBuilder> scratch,
                                  shared_ptr<BytesRef> prefix)
{
  assert((startsWith(scratch, prefix)));
  return stoi(stripPrefix(scratch, prefix));
}

wstring SimpleTextBKDReader::stripPrefix(shared_ptr<BytesRefBuilder> scratch,
                                         shared_ptr<BytesRef> prefix)
{
  return wstring(scratch->bytes(), prefix->length,
                 scratch->length() - prefix->length, StandardCharsets::UTF_8);
}

bool SimpleTextBKDReader::startsWith(shared_ptr<BytesRefBuilder> scratch,
                                     shared_ptr<BytesRef> prefix)
{
  return StringHelper::startsWith(scratch->get(), prefix);
}

void SimpleTextBKDReader::readLine(
    shared_ptr<IndexInput> in_,
    shared_ptr<BytesRefBuilder> scratch) 
{
  SimpleTextUtil::readLine(in_, scratch);
}
} // namespace org::apache::lucene::codecs::simpletext