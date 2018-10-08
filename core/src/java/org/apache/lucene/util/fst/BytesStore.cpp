using namespace std;

#include "BytesStore.h"

namespace org::apache::lucene::util::fst
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

BytesStore::BytesStore(int blockBits)
    : blockSize(1 << blockBits), blockBits(blockBits), blockMask(blockSize - 1)
{
  nextWrite = blockSize;
}

BytesStore::BytesStore(shared_ptr<DataInput> in_, int64_t numBytes,
                       int maxBlockSize) 
    : blockSize(blockSize), blockBits(blockBits), blockMask(blockSize - 1)
{
  int blockSize = 2;
  int blockBits = 1;
  while (blockSize < numBytes && blockSize < maxBlockSize) {
    blockSize *= 2;
    blockBits++;
  }
  int64_t left = numBytes;
  while (left > 0) {
    constexpr int chunk = static_cast<int>(min(blockSize, left));
    std::deque<char> block(chunk);
    in_->readBytes(block, 0, block.size());
    blocks.push_back(block);
    left -= chunk;
  }

  // So .getPosition still works
  nextWrite = blocks[blocks.size() - 1]->size();
}

void BytesStore::writeByte(int dest, char b)
{
  int blockIndex = dest >> blockBits;
  std::deque<char> block = blocks[blockIndex];
  block[dest & blockMask] = b;
}

void BytesStore::writeByte(char b)
{
  if (nextWrite == blockSize) {
    current = std::deque<char>(blockSize);
    blocks.push_back(current);
    nextWrite = 0;
  }
  current[nextWrite++] = b;
}

void BytesStore::writeBytes(std::deque<char> &b, int offset, int len)
{
  while (len > 0) {
    int chunk = blockSize - nextWrite;
    if (len <= chunk) {
      assert(b.size() > 0);
      assert(current.size() > 0);
      System::arraycopy(b, offset, current, nextWrite, len);
      nextWrite += len;
      break;
    } else {
      if (chunk > 0) {
        System::arraycopy(b, offset, current, nextWrite, chunk);
        offset += chunk;
        len -= chunk;
      }
      current = std::deque<char>(blockSize);
      blocks.push_back(current);
      nextWrite = 0;
    }
  }
}

int BytesStore::getBlockBits() { return blockBits; }

void BytesStore::writeBytes(int64_t dest, std::deque<char> &b, int offset,
                            int len)
{
  // System.out.println("  BS.writeBytes dest=" + dest + " offset=" + offset + "
  // len=" + len);
  assert((dest + len <= getPosition(), L"dest=" + to_wstring(dest) + L" pos=" +
                                           to_wstring(getPosition()) +
                                           L" len=" + to_wstring(len)));

  // Note: weird: must go "backwards" because copyBytes
  // calls us with overlapping src/dest.  If we
  // go forwards then we overwrite bytes before we can
  // copy them:

  /*
  int blockIndex = dest >> blockBits;
  int upto = dest & blockMask;
  byte[] block = blocks.get(blockIndex);
  while (len > 0) {
    int chunk = blockSize - upto;
    System.out.println("    cycle chunk=" + chunk + " len=" + len);
    if (len <= chunk) {
      System.arraycopy(b, offset, block, upto, len);
      break;
    } else {
      System.arraycopy(b, offset, block, upto, chunk);
      offset += chunk;
      len -= chunk;
      blockIndex++;
      block = blocks.get(blockIndex);
      upto = 0;
    }
  }
  */

  constexpr int64_t end = dest + len;
  int blockIndex = static_cast<int>(end >> blockBits);
  int downTo = static_cast<int>(end & blockMask);
  if (downTo == 0) {
    blockIndex--;
    downTo = blockSize;
  }
  std::deque<char> block = blocks[blockIndex];

  while (len > 0) {
    // System.out.println("    cycle downTo=" + downTo + " len=" + len);
    if (len <= downTo) {
      // System.out.println("      final: offset=" + offset + " len=" + len + "
      // dest=" + (downTo-len));
      System::arraycopy(b, offset, block, downTo - len, len);
      break;
    } else {
      len -= downTo;
      // System.out.println("      partial: offset=" + (offset + len) + " len="
      // + downTo + " dest=0");
      System::arraycopy(b, offset + len, block, 0, downTo);
      blockIndex--;
      block = blocks[blockIndex];
      downTo = blockSize;
    }
  }
}

void BytesStore::copyBytes(int64_t src, int64_t dest, int len)
{
  // System.out.println("BS.copyBytes src=" + src + " dest=" + dest + " len=" +
  // len);
  assert(src < dest);

  // Note: weird: must go "backwards" because copyBytes
  // calls us with overlapping src/dest.  If we
  // go forwards then we overwrite bytes before we can
  // copy them:

  /*
  int blockIndex = src >> blockBits;
  int upto = src & blockMask;
  byte[] block = blocks.get(blockIndex);
  while (len > 0) {
    int chunk = blockSize - upto;
    System.out.println("  cycle: chunk=" + chunk + " len=" + len);
    if (len <= chunk) {
      writeBytes(dest, block, upto, len);
      break;
    } else {
      writeBytes(dest, block, upto, chunk);
      blockIndex++;
      block = blocks.get(blockIndex);
      upto = 0;
      len -= chunk;
      dest += chunk;
    }
  }
  */

  int64_t end = src + len;

  int blockIndex = static_cast<int>(end >> blockBits);
  int downTo = static_cast<int>(end & blockMask);
  if (downTo == 0) {
    blockIndex--;
    downTo = blockSize;
  }
  std::deque<char> block = blocks[blockIndex];

  while (len > 0) {
    // System.out.println("  cycle downTo=" + downTo);
    if (len <= downTo) {
      // System.out.println("    finish");
      writeBytes(dest, block, downTo - len, len);
      break;
    } else {
      // System.out.println("    partial");
      len -= downTo;
      writeBytes(dest + len, block, 0, downTo);
      blockIndex--;
      block = blocks[blockIndex];
      downTo = blockSize;
    }
  }
}

void BytesStore::writeInt(int64_t pos, int value)
{
  int blockIndex = static_cast<int>(pos >> blockBits);
  int upto = static_cast<int>(pos & blockMask);
  std::deque<char> block = blocks[blockIndex];
  int shift = 24;
  for (int i = 0; i < 4; i++) {
    block[upto++] = static_cast<char>(value >> shift);
    shift -= 8;
    if (upto == blockSize) {
      upto = 0;
      blockIndex++;
      block = blocks[blockIndex];
    }
  }
}

void BytesStore::reverse(int64_t srcPos, int64_t destPos)
{
  assert(srcPos < destPos);
  assert(destPos < getPosition());
  // System.out.println("reverse src=" + srcPos + " dest=" + destPos);

  int srcBlockIndex = static_cast<int>(srcPos >> blockBits);
  int src = static_cast<int>(srcPos & blockMask);
  std::deque<char> srcBlock = blocks[srcBlockIndex];

  int destBlockIndex = static_cast<int>(destPos >> blockBits);
  int dest = static_cast<int>(destPos & blockMask);
  std::deque<char> destBlock = blocks[destBlockIndex];
  // System.out.println("  srcBlock=" + srcBlockIndex + " destBlock=" +
  // destBlockIndex);

  int limit = static_cast<int>(destPos - srcPos + 1) / 2;
  for (int i = 0; i < limit; i++) {
    // System.out.println("  cycle src=" + src + " dest=" + dest);
    char b = srcBlock[src];
    srcBlock[src] = destBlock[dest];
    destBlock[dest] = b;
    src++;
    if (src == blockSize) {
      srcBlockIndex++;
      srcBlock = blocks[srcBlockIndex];
      // System.out.println("  set destBlock=" + destBlock + " srcBlock=" +
      // srcBlock);
      src = 0;
    }

    dest--;
    if (dest == -1) {
      destBlockIndex--;
      destBlock = blocks[destBlockIndex];
      // System.out.println("  set destBlock=" + destBlock + " srcBlock=" +
      // srcBlock);
      dest = blockSize - 1;
    }
  }
}

void BytesStore::skipBytes(int len)
{
  while (len > 0) {
    int chunk = blockSize - nextWrite;
    if (len <= chunk) {
      nextWrite += len;
      break;
    } else {
      len -= chunk;
      current = std::deque<char>(blockSize);
      blocks.push_back(current);
      nextWrite = 0;
    }
  }
}

int64_t BytesStore::getPosition()
{
  return (static_cast<int64_t>(blocks.size()) - 1) * blockSize + nextWrite;
}

void BytesStore::truncate(int64_t newLen)
{
  assert(newLen <= getPosition());
  assert(newLen >= 0);
  int blockIndex = static_cast<int>(newLen >> blockBits);
  nextWrite = static_cast<int>(newLen & blockMask);
  if (nextWrite == 0) {
    blockIndex--;
    nextWrite = blockSize;
  }
  blocks.subList(blockIndex + 1, blocks.size())->clear();
  if (newLen == 0) {
    current.clear();
  } else {
    current = blocks[blockIndex];
  }
  assert(newLen == getPosition());
}

void BytesStore::finish()
{
  if (current.size() > 0) {
    std::deque<char> lastBuffer(nextWrite);
    System::arraycopy(current, 0, lastBuffer, 0, nextWrite);
    blocks[blocks.size() - 1] = lastBuffer;
    current.clear();
  }
}

void BytesStore::writeTo(shared_ptr<DataOutput> out) 
{
  for (auto block : blocks) {
    out->writeBytes(block, 0, block.size());
  }
}

shared_ptr<FST::BytesReader> BytesStore::getForwardReader()
{
  if (blocks.size() == 1) {
    return make_shared<ForwardBytesReader>(blocks[0]);
  }
  return make_shared<BytesReaderAnonymousInnerClass>(shared_from_this());
}

BytesStore::BytesReaderAnonymousInnerClass::BytesReaderAnonymousInnerClass(
    shared_ptr<BytesStore> outerInstance)
{
  this->outerInstance = outerInstance;
  nextRead = outerInstance->blockSize;
}

char BytesStore::BytesReaderAnonymousInnerClass::readByte()
{
  if (nextRead == outerInstance->blockSize) {
    outerInstance->current = outerInstance->blocks[nextBuffer++];
    nextRead = 0;
  }
  return outerInstance->current[nextRead++];
}

void BytesStore::BytesReaderAnonymousInnerClass::skipBytes(int64_t count)
{
  setPosition(outerInstance->getPosition() + count);
}

void BytesStore::BytesReaderAnonymousInnerClass::readBytes(std::deque<char> &b,
                                                           int offset, int len)
{
  while (len > 0) {
    int chunkLeft = outerInstance->blockSize - nextRead;
    if (len <= chunkLeft) {
      System::arraycopy(outerInstance->current, nextRead, b, offset, len);
      nextRead += len;
      break;
    } else {
      if (chunkLeft > 0) {
        System::arraycopy(outerInstance->current, nextRead, b, offset,
                          chunkLeft);
        offset += chunkLeft;
        len -= chunkLeft;
      }
      outerInstance->current = outerInstance->blocks[nextBuffer++];
      nextRead = 0;
    }
  }
}

int64_t BytesStore::BytesReaderAnonymousInnerClass::getPosition()
{
  return (static_cast<int64_t>(nextBuffer) - 1) * outerInstance->blockSize +
         nextRead;
}

void BytesStore::BytesReaderAnonymousInnerClass::setPosition(int64_t pos)
{
  int bufferIndex = static_cast<int>(pos >> outerInstance->blockBits);
  nextBuffer = bufferIndex + 1;
  outerInstance->current = outerInstance->blocks[bufferIndex];
  nextRead = static_cast<int>(pos & outerInstance->blockMask);
  assert(outerInstance->getPosition() == pos);
}

bool BytesStore::BytesReaderAnonymousInnerClass::reversed() { return false; }

shared_ptr<FST::BytesReader> BytesStore::getReverseReader()
{
  return getReverseReader(true);
}

shared_ptr<FST::BytesReader> BytesStore::getReverseReader(bool allowSingle)
{
  if (allowSingle && blocks.size() == 1) {
    return make_shared<ReverseBytesReader>(blocks[0]);
  }
  return make_shared<BytesReaderAnonymousInnerClass2>(shared_from_this());
}

BytesStore::BytesReaderAnonymousInnerClass2::BytesReaderAnonymousInnerClass2(
    shared_ptr<BytesStore> outerInstance)
{
  this->outerInstance = outerInstance;
  outerInstance->current =
      outerInstance->blocks.empty() ? nullptr : outerInstance->blocks[0];
  nextBuffer = -1;
  nextRead = 0;
}

char BytesStore::BytesReaderAnonymousInnerClass2::readByte()
{
  if (nextRead == -1) {
    outerInstance->current = outerInstance->blocks[nextBuffer--];
    nextRead = outerInstance->blockSize - 1;
  }
  return outerInstance->current[nextRead--];
}

void BytesStore::BytesReaderAnonymousInnerClass2::skipBytes(int64_t count)
{
  setPosition(outerInstance->getPosition() - count);
}

void BytesStore::BytesReaderAnonymousInnerClass2::readBytes(
    std::deque<char> &b, int offset, int len)
{
  for (int i = 0; i < len; i++) {
    b[offset + i] = readByte();
  }
}

int64_t BytesStore::BytesReaderAnonymousInnerClass2::getPosition()
{
  return (static_cast<int64_t>(nextBuffer) + 1) * outerInstance->blockSize +
         nextRead;
}

void BytesStore::BytesReaderAnonymousInnerClass2::setPosition(int64_t pos)
{
  // NOTE: a little weird because if you
  // setPosition(0), the next byte you read is
  // bytes[0] ... but I would expect bytes[-1] (ie,
  // EOF)...?
  int bufferIndex = static_cast<int>(pos >> outerInstance->blockBits);
  nextBuffer = bufferIndex - 1;
  outerInstance->current = outerInstance->blocks[bufferIndex];
  nextRead = static_cast<int>(pos & outerInstance->blockMask);
  assert((outerInstance->getPosition() == pos,
          L"pos=" + to_wstring(pos) + L" getPos()=" +
              to_wstring(outerInstance->getPosition())));
}

bool BytesStore::BytesReaderAnonymousInnerClass2::reversed() { return true; }

int64_t BytesStore::ramBytesUsed()
{
  int64_t size = BASE_RAM_BYTES_USED;
  for (auto block : blocks) {
    size += RamUsageEstimator::sizeOf(block);
  }
  return size;
}

wstring BytesStore::toString()
{
  return getClass().getSimpleName() + L"(numBlocks=" + blocks.size() + L")";
}
} // namespace org::apache::lucene::util::fst