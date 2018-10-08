using namespace std;

#include "CompressingStoredFieldsIndexReader.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/SegmentInfo.h"
#include "../../store/IndexInput.h"
#include "../../util/Accountables.h"
#include "../../util/ArrayUtil.h"
#include "../../util/packed/PackedInts.h"

namespace org::apache::lucene::codecs::compressing
{
//    import static org.apache.lucene.util.BitUtil.zigZagDecode;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

CompressingStoredFieldsIndexReader::CompressingStoredFieldsIndexReader(
    shared_ptr<IndexInput> fieldsIndexIn,
    shared_ptr<SegmentInfo> si) 
    : maxDoc(si->maxDoc())
{
  std::deque<int> docBases(16);
  std::deque<int64_t> startPointers(16);
  std::deque<int> avgChunkDocs(16);
  std::deque<int64_t> avgChunkSizes(16);
  std::deque<std::shared_ptr<PackedInts::Reader>> docBasesDeltas(16);
  std::deque<std::shared_ptr<PackedInts::Reader>> startPointersDeltas(16);

  constexpr int packedIntsVersion = fieldsIndexIn->readVInt();

  int blockCount = 0;

  for (;;) {
    constexpr int numChunks = fieldsIndexIn->readVInt();
    if (numChunks == 0) {
      break;
    }
    if (blockCount == docBases.size()) {
      constexpr int newSize = ArrayUtil::oversize(blockCount + 1, 8);
      docBases = Arrays::copyOf(docBases, newSize);
      startPointers = Arrays::copyOf(startPointers, newSize);
      avgChunkDocs = Arrays::copyOf(avgChunkDocs, newSize);
      avgChunkSizes = Arrays::copyOf(avgChunkSizes, newSize);
      docBasesDeltas = Arrays::copyOf(docBasesDeltas, newSize);
      startPointersDeltas = Arrays::copyOf(startPointersDeltas, newSize);
    }

    // doc bases
    docBases[blockCount] = fieldsIndexIn->readVInt();
    avgChunkDocs[blockCount] = fieldsIndexIn->readVInt();
    constexpr int bitsPerDocBase = fieldsIndexIn->readVInt();
    if (bitsPerDocBase > 32) {
      throw make_shared<CorruptIndexException>(L"Corrupted bitsPerDocBase: " +
                                                   to_wstring(bitsPerDocBase),
                                               fieldsIndexIn);
    }
    docBasesDeltas[blockCount] = PackedInts::getReaderNoHeader(
        fieldsIndexIn, PackedInts::Format::PACKED, packedIntsVersion, numChunks,
        bitsPerDocBase);

    // start pointers
    startPointers[blockCount] = fieldsIndexIn->readVLong();
    avgChunkSizes[blockCount] = fieldsIndexIn->readVLong();
    constexpr int bitsPerStartPointer = fieldsIndexIn->readVInt();
    if (bitsPerStartPointer > 64) {
      throw make_shared<CorruptIndexException>(
          L"Corrupted bitsPerStartPointer: " + to_wstring(bitsPerStartPointer),
          fieldsIndexIn);
    }
    startPointersDeltas[blockCount] = PackedInts::getReaderNoHeader(
        fieldsIndexIn, PackedInts::Format::PACKED, packedIntsVersion, numChunks,
        bitsPerStartPointer);

    ++blockCount;
  }

  this->docBases = Arrays::copyOf(docBases, blockCount);
  this->startPointers = Arrays::copyOf(startPointers, blockCount);
  this->avgChunkDocs = Arrays::copyOf(avgChunkDocs, blockCount);
  this->avgChunkSizes = Arrays::copyOf(avgChunkSizes, blockCount);
  this->docBasesDeltas = Arrays::copyOf(docBasesDeltas, blockCount);
  this->startPointersDeltas = Arrays::copyOf(startPointersDeltas, blockCount);
}

int CompressingStoredFieldsIndexReader::block(int docID)
{
  int lo = 0, hi = docBases.size() - 1;
  while (lo <= hi) {
    constexpr int mid =
        static_cast<int>(static_cast<unsigned int>((lo + hi)) >> 1);
    constexpr int midValue = docBases[mid];
    if (midValue == docID) {
      return mid;
    } else if (midValue < docID) {
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
  }
  return hi;
}

int CompressingStoredFieldsIndexReader::relativeDocBase(int block,
                                                        int relativeChunk)
{
  constexpr int expected = avgChunkDocs[block] * relativeChunk;
  constexpr int64_t delta =
      zigZagDecode(docBasesDeltas[block]->get(relativeChunk));
  return expected + static_cast<int>(delta);
}

int64_t
CompressingStoredFieldsIndexReader::relativeStartPointer(int block,
                                                         int relativeChunk)
{
  constexpr int64_t expected = avgChunkSizes[block] * relativeChunk;
  constexpr int64_t delta =
      zigZagDecode(startPointersDeltas[block]->get(relativeChunk));
  return expected + delta;
}

int CompressingStoredFieldsIndexReader::relativeChunk(int block,
                                                      int relativeDoc)
{
  int lo = 0, hi = docBasesDeltas[block]->size() - 1;
  while (lo <= hi) {
    constexpr int mid =
        static_cast<int>(static_cast<unsigned int>((lo + hi)) >> 1);
    constexpr int midValue = relativeDocBase(block, mid);
    if (midValue == relativeDoc) {
      return mid;
    } else if (midValue < relativeDoc) {
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
  }
  return hi;
}

int64_t CompressingStoredFieldsIndexReader::getStartPointer(int docID)
{
  if (docID < 0 || docID >= maxDoc) {
    throw invalid_argument(L"docID out of range [0-" + to_wstring(maxDoc) +
                           L"]: " + to_wstring(docID));
  }
  constexpr int block = this->block(docID);
  constexpr int relativeChunk =
      this->relativeChunk(block, docID - docBases[block]);
  return startPointers[block] + relativeStartPointer(block, relativeChunk);
}

shared_ptr<CompressingStoredFieldsIndexReader>
CompressingStoredFieldsIndexReader::clone()
{
  return shared_from_this();
}

int64_t CompressingStoredFieldsIndexReader::ramBytesUsed()
{
  int64_t res = BASE_RAM_BYTES_USED;

  res += RamUsageEstimator::shallowSizeOf(docBasesDeltas);
  for (auto r : docBasesDeltas) {
    res += r->ramBytesUsed();
  }
  res += RamUsageEstimator::shallowSizeOf(startPointersDeltas);
  for (auto r : startPointersDeltas) {
    res += r->ramBytesUsed();
  }

  res += RamUsageEstimator::sizeOf(docBases);
  res += RamUsageEstimator::sizeOf(startPointers);
  res += RamUsageEstimator::sizeOf(avgChunkDocs);
  res += RamUsageEstimator::sizeOf(avgChunkSizes);

  return res;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
CompressingStoredFieldsIndexReader::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();

  int64_t docBaseDeltaBytes =
      RamUsageEstimator::shallowSizeOf(docBasesDeltas);
  for (auto r : docBasesDeltas) {
    docBaseDeltaBytes += r->ramBytesUsed();
  }
  resources.push_back(
      Accountables::namedAccountable(L"doc base deltas", docBaseDeltaBytes));

  int64_t startPointerDeltaBytes =
      RamUsageEstimator::shallowSizeOf(startPointersDeltas);
  for (auto r : startPointersDeltas) {
    startPointerDeltaBytes += r->ramBytesUsed();
  }
  resources.push_back(Accountables::namedAccountable(L"start pointer deltas",
                                                     startPointerDeltaBytes));

  return Collections::unmodifiableList(resources);
}

wstring CompressingStoredFieldsIndexReader::toString()
{
  return getClass().getSimpleName() + L"(blocks=" + docBases.size() + L")";
}
} // namespace org::apache::lucene::codecs::compressing