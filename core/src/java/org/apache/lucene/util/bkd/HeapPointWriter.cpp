using namespace std;

#include "HeapPointWriter.h"

namespace org::apache::lucene::util::bkd
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;

HeapPointWriter::HeapPointWriter(int initSize, int maxSize,
                                 int packedBytesLength, bool longOrds,
                                 bool singleValuePerDoc)
    : maxSize(maxSize), valuesPerBlock(max(1, 4096 / packedBytesLength)),
      packedBytesLength(packedBytesLength), singleValuePerDoc(singleValuePerDoc)
{
  docIDs = std::deque<int>(initSize);
  if (singleValuePerDoc) {
    this->ordsLong.clear();
    this->ords.clear();
  } else {
    if (longOrds) {
      this->ordsLong = std::deque<int64_t>(initSize);
    } else {
      this->ords = std::deque<int>(initSize);
    }
  }
  // 4K per page, unless each value is > 4K:
}

void HeapPointWriter::copyFrom(shared_ptr<HeapPointWriter> other)
{
  if (docIDs.size() < other->nextWrite) {
    throw make_shared<IllegalStateException>(L"docIDs.length=" + docIDs.size() +
                                             L" other.nextWrite=" +
                                             to_wstring(other->nextWrite));
  }
  System::arraycopy(other->docIDs, 0, docIDs, 0, other->nextWrite);
  if (singleValuePerDoc == false) {
    if (other->ords.size() > 0) {
      assert(this->ords.size() > 0);
      System::arraycopy(other->ords, 0, ords, 0, other->nextWrite);
    } else {
      assert(this->ordsLong.size() > 0);
      System::arraycopy(other->ordsLong, 0, ordsLong, 0, other->nextWrite);
    }
  }

  for (auto block : other->blocks) {
    blocks.push_back(block.clone());
  }
  nextWrite = other->nextWrite;
}

void HeapPointWriter::readPackedValue(int index, std::deque<char> &bytes)
{
  assert(bytes.size() == packedBytesLength);
  int block = index / valuesPerBlock;
  int blockIndex = index % valuesPerBlock;
  System::arraycopy(blocks[block], blockIndex * packedBytesLength, bytes, 0,
                    packedBytesLength);
}

void HeapPointWriter::getPackedValueSlice(int index,
                                          shared_ptr<BytesRef> result)
{
  int block = index / valuesPerBlock;
  int blockIndex = index % valuesPerBlock;
  result->bytes = blocks[block];
  result->offset = blockIndex * packedBytesLength;
  assert(result->length == packedBytesLength);
}

void HeapPointWriter::writePackedValue(int index, std::deque<char> &bytes)
{
  assert(bytes.size() == packedBytesLength);
  int block = index / valuesPerBlock;
  int blockIndex = index % valuesPerBlock;
  // System.out.println("writePackedValue: index=" + index + " bytes.length=" +
  // bytes.length + " block=" + block + " blockIndex=" + blockIndex + "
  // valuesPerBlock=" + valuesPerBlock);
  while (blocks.size() <= block) {
    // If this is the last block, only allocate as large as necessary for
    // maxSize:
    int valuesInBlock =
        min(valuesPerBlock, maxSize - (blocks.size() * valuesPerBlock));
    blocks.push_back(std::deque<char>(valuesInBlock * packedBytesLength));
  }
  System::arraycopy(bytes, 0, blocks[block], blockIndex * packedBytesLength,
                    packedBytesLength);
}

void HeapPointWriter::append(std::deque<char> &packedValue, int64_t ord,
                             int docID)
{
  assert(closed == false);
  assert(packedValue.size() == packedBytesLength);
  if (docIDs.size() == nextWrite) {
    int nextSize =
        min(maxSize, ArrayUtil::oversize(nextWrite + 1, Integer::BYTES));
    assert((nextSize > nextWrite, L"nextSize=" + to_wstring(nextSize) +
                                      L" vs nextWrite=" +
                                      to_wstring(nextWrite)));
    docIDs = Arrays::copyOf(docIDs, nextSize);
    if (singleValuePerDoc == false) {
      if (ordsLong.size() > 0) {
        ordsLong = Arrays::copyOf(ordsLong, nextSize);
      } else {
        ords = Arrays::copyOf(ords, nextSize);
      }
    }
  }
  writePackedValue(nextWrite, packedValue);
  if (singleValuePerDoc == false) {
    if (ordsLong.size() > 0) {
      ordsLong[nextWrite] = ord;
    } else {
      assert((ord <= std, : numeric_limits<int>::max()));
      ords[nextWrite] = static_cast<int>(ord);
    }
  }
  docIDs[nextWrite] = docID;
  nextWrite++;
}

shared_ptr<PointReader> HeapPointWriter::getReader(int64_t start,
                                                   int64_t length)
{
  assert((start + length <= docIDs.size(),
          L"start=" + to_wstring(start) + L" length=" + to_wstring(length) +
              L" docIDs.length=" + docIDs.size()));
  assert((start + length <= nextWrite,
          L"start=" + to_wstring(start) + L" length=" + to_wstring(length) +
              L" nextWrite=" + to_wstring(nextWrite)));
  return make_shared<HeapPointReader>(
      blocks, valuesPerBlock, packedBytesLength, ords, ordsLong, docIDs,
      static_cast<int>(start), Math::toIntExact(start + length),
      singleValuePerDoc);
}

shared_ptr<PointReader> HeapPointWriter::getSharedReader(
    int64_t start, int64_t length,
    deque<std::shared_ptr<Closeable>> &toCloseHeroically)
{
  return make_shared<HeapPointReader>(
      blocks, valuesPerBlock, packedBytesLength, ords, ordsLong, docIDs,
      static_cast<int>(start), nextWrite, singleValuePerDoc);
}

HeapPointWriter::~HeapPointWriter() { closed = true; }

void HeapPointWriter::destroy() {}

wstring HeapPointWriter::toString()
{
  return L"HeapPointWriter(count=" + to_wstring(nextWrite) + L" alloc=" +
         docIDs.size() + L")";
}
} // namespace org::apache::lucene::util::bkd