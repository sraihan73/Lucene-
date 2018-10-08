using namespace std;

#include "HeapPointReader.h"

namespace org::apache::lucene::util::bkd
{

HeapPointReader::HeapPointReader(deque<std::deque<char>> &blocks,
                                 int valuesPerBlock, int packedBytesLength,
                                 std::deque<int> &ords,
                                 std::deque<int64_t> &ordsLong,
                                 std::deque<int> &docIDs, int start, int end,
                                 bool singleValuePerDoc)
    : blocks(blocks), valuesPerBlock(valuesPerBlock),
      packedBytesLength(packedBytesLength), ordsLong(ordsLong), ords(ords),
      docIDs(docIDs), end(end), scratch(std::deque<char>(packedBytesLength)),
      singleValuePerDoc(singleValuePerDoc)
{
  curRead = start - 1;
}

void HeapPointReader::writePackedValue(int index, std::deque<char> &bytes)
{
  int block = index / valuesPerBlock;
  int blockIndex = index % valuesPerBlock;
  while (blocks.size() <= block) {
    blocks.push_back(std::deque<char>(valuesPerBlock * packedBytesLength));
  }
  System::arraycopy(bytes, 0, blocks[blockIndex],
                    blockIndex * packedBytesLength, packedBytesLength);
}

void HeapPointReader::readPackedValue(int index, std::deque<char> &bytes)
{
  int block = index / valuesPerBlock;
  int blockIndex = index % valuesPerBlock;
  System::arraycopy(blocks[block], blockIndex * packedBytesLength, bytes, 0,
                    packedBytesLength);
}

bool HeapPointReader::next()
{
  curRead++;
  return curRead < end;
}

std::deque<char> HeapPointReader::packedValue()
{
  readPackedValue(curRead, scratch);
  return scratch;
}

int HeapPointReader::docID() { return docIDs[curRead]; }

int64_t HeapPointReader::ord()
{
  if (singleValuePerDoc) {
    return docIDs[curRead];
  } else if (ordsLong.size() > 0) {
    return ordsLong[curRead];
  } else {
    return ords[curRead];
  }
}

HeapPointReader::~HeapPointReader() {}
} // namespace org::apache::lucene::util::bkd