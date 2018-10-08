using namespace std;

#include "PagedBytes.h"

namespace org::apache::lucene::util
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;
std::deque<char> const PagedBytes::EMPTY_BYTES = std::deque<char>(0);

PagedBytes::Reader::Reader(shared_ptr<PagedBytes> pagedBytes)
    : blocks(Arrays::copyOf(pagedBytes->blocks, pagedBytes->numBlocks)),
      blockBits(pagedBytes->blockBits), blockMask(pagedBytes->blockMask),
      blockSize(pagedBytes->blockSize),
      bytesUsedPerBlock(pagedBytes->bytesUsedPerBlock)
{
}

void PagedBytes::Reader::fillSlice(shared_ptr<BytesRef> b, int64_t start,
                                   int length)
{
  assert((length >= 0, L"length=" + to_wstring(length)));
  assert((length <= blockSize + 1, L"length=" + to_wstring(length)));
  b->length = length;
  if (length == 0) {
    return;
  }
  constexpr int index = static_cast<int>(start >> blockBits);
  constexpr int offset = static_cast<int>(start & blockMask);
  if (blockSize - offset >= length) {
    // Within block
    b->bytes = blocks[index];
    b->offset = offset;
  } else {
    // Split
    b->bytes = std::deque<char>(length);
    b->offset = 0;
    System::arraycopy(blocks[index], offset, b->bytes, 0, blockSize - offset);
    System::arraycopy(blocks[1 + index], 0, b->bytes, blockSize - offset,
                      length - (blockSize - offset));
  }
}

void PagedBytes::Reader::fill(shared_ptr<BytesRef> b, int64_t start)
{
  constexpr int index = static_cast<int>(start >> blockBits);
  constexpr int offset = static_cast<int>(start & blockMask);
  const std::deque<char> block = b->bytes = blocks[index];

  if ((block[offset] & 128) == 0) {
    b->length = block[offset];
    b->offset = offset + 1;
  } else {
    b->length = ((block[offset] & 0x7f) << 8) | (block[1 + offset] & 0xff);
    b->offset = offset + 2;
    assert(b->length > 0);
  }
}

int64_t PagedBytes::Reader::ramBytesUsed()
{
  int64_t size =
      BASE_RAM_BYTES_USED + RamUsageEstimator::shallowSizeOf(blocks);
  if (blocks.size() > 0) {
    size += (blocks.size() - 1) * bytesUsedPerBlock;
    size += RamUsageEstimator::sizeOf(blocks[blocks.size() - 1]);
  }
  return size;
}

wstring PagedBytes::Reader::toString()
{
  return L"PagedBytes(blocksize=" + to_wstring(blockSize) + L")";
}

PagedBytes::PagedBytes(int blockBits)
    : blockSize(1 << blockBits), blockBits(blockBits), blockMask(blockSize - 1),
      bytesUsedPerBlock(RamUsageEstimator::alignObjectSize(
          blockSize + RamUsageEstimator::NUM_BYTES_ARRAY_HEADER))
{
  assert((blockBits > 0 && blockBits <= 31, blockBits));
  upto = blockSize;
  numBlocks = 0;
}

void PagedBytes::addBlock(std::deque<char> &block)
{
  if (blocks.size() == numBlocks) {
    blocks = Arrays::copyOf(
        blocks, ArrayUtil::oversize(numBlocks,
                                    RamUsageEstimator::NUM_BYTES_OBJECT_REF));
  }
  blocks[numBlocks++] = block;
}

void PagedBytes::copy(shared_ptr<IndexInput> in_,
                      int64_t byteCount) 
{
  while (byteCount > 0) {
    int left = blockSize - upto;
    if (left == 0) {
      if (currentBlock.size() > 0) {
        addBlock(currentBlock);
      }
      currentBlock = std::deque<char>(blockSize);
      upto = 0;
      left = blockSize;
    }
    if (left < byteCount) {
      in_->readBytes(currentBlock, upto, left, false);
      upto = blockSize;
      byteCount -= left;
    } else {
      in_->readBytes(currentBlock, upto, static_cast<int>(byteCount), false);
      upto += byteCount;
      break;
    }
  }
}

void PagedBytes::copy(shared_ptr<BytesRef> bytes, shared_ptr<BytesRef> out)
{
  int left = blockSize - upto;
  if (bytes->length > left || currentBlock.empty()) {
    if (currentBlock.size() > 0) {
      addBlock(currentBlock);
      didSkipBytes = true;
    }
    currentBlock = std::deque<char>(blockSize);
    upto = 0;
    left = blockSize;
    assert(bytes->length <= blockSize);
    // TODO: we could also support variable block sizes
  }

  out->bytes = currentBlock;
  out->offset = upto;
  out->length = bytes->length;

  System::arraycopy(bytes->bytes, bytes->offset, currentBlock, upto,
                    bytes->length);
  upto += bytes->length;
}

shared_ptr<Reader> PagedBytes::freeze(bool trim)
{
  if (frozen) {
    throw make_shared<IllegalStateException>(L"already frozen");
  }
  if (didSkipBytes) {
    throw make_shared<IllegalStateException>(
        L"cannot freeze when copy(BytesRef, BytesRef) was used");
  }
  if (trim && upto < blockSize) {
    const std::deque<char> newBlock = std::deque<char>(upto);
    System::arraycopy(currentBlock, 0, newBlock, 0, upto);
    currentBlock = newBlock;
  }
  if (currentBlock.empty()) {
    currentBlock = EMPTY_BYTES;
  }
  addBlock(currentBlock);
  frozen = true;
  currentBlock.clear();
  return make_shared<PagedBytes::Reader>(shared_from_this());
}

int64_t PagedBytes::getPointer()
{
  if (currentBlock.empty()) {
    return 0;
  } else {
    return (numBlocks * (static_cast<int64_t>(blockSize))) + upto;
  }
}

int64_t PagedBytes::ramBytesUsed()
{
  int64_t size =
      BASE_RAM_BYTES_USED + RamUsageEstimator::shallowSizeOf(blocks);
  if (numBlocks > 0) {
    size += (numBlocks - 1) * bytesUsedPerBlock;
    size += RamUsageEstimator::sizeOf(blocks[numBlocks - 1]);
  }
  if (currentBlock.size() > 0) {
    size += RamUsageEstimator::sizeOf(currentBlock);
  }
  return size;
}

int64_t PagedBytes::copyUsingLengthPrefix(shared_ptr<BytesRef> bytes)
{
  if (bytes->length >= 32768) {
    throw invalid_argument(L"max length is 32767 (got " +
                           to_wstring(bytes->length) + L")");
  }

  if (upto + bytes->length + 2 > blockSize) {
    if (bytes->length + 2 > blockSize) {
      throw invalid_argument(L"block size " + to_wstring(blockSize) +
                             L" is too small to store length " +
                             to_wstring(bytes->length) + L" bytes");
    }
    if (currentBlock.size() > 0) {
      addBlock(currentBlock);
    }
    currentBlock = std::deque<char>(blockSize);
    upto = 0;
  }

  constexpr int64_t pointer = getPointer();

  if (bytes->length < 128) {
    currentBlock[upto++] = static_cast<char>(bytes->length);
  } else {
    currentBlock[upto++] = static_cast<char>(0x80 | (bytes->length >> 8));
    currentBlock[upto++] = static_cast<char>(bytes->length & 0xff);
  }
  System::arraycopy(bytes->bytes, bytes->offset, currentBlock, upto,
                    bytes->length);
  upto += bytes->length;

  return pointer;
}

PagedBytes::PagedBytesDataInput::PagedBytesDataInput(
    shared_ptr<PagedBytes> outerInstance)
    : outerInstance(outerInstance)
{
  currentBlock = outerInstance->blocks[0];
}

shared_ptr<PagedBytesDataInput> PagedBytes::PagedBytesDataInput::clone()
{
  shared_ptr<PagedBytesDataInput> clone = outerInstance->getDataInput();
  clone->setPosition(getPosition());
  return clone;
}

int64_t PagedBytes::PagedBytesDataInput::getPosition()
{
  return static_cast<int64_t>(currentBlockIndex) * outerInstance->blockSize +
         currentBlockUpto;
}

void PagedBytes::PagedBytesDataInput::setPosition(int64_t pos)
{
  currentBlockIndex = static_cast<int>(pos >> outerInstance->blockBits);
  currentBlock = outerInstance->blocks[currentBlockIndex];
  currentBlockUpto = static_cast<int>(pos & outerInstance->blockMask);
}

char PagedBytes::PagedBytesDataInput::readByte()
{
  if (currentBlockUpto == outerInstance->blockSize) {
    nextBlock();
  }
  return currentBlock[currentBlockUpto++];
}

void PagedBytes::PagedBytesDataInput::readBytes(std::deque<char> &b,
                                                int offset, int len)
{
  assert(b.size() >= offset + len);
  constexpr int offsetEnd = offset + len;
  while (true) {
    constexpr int blockLeft = outerInstance->blockSize - currentBlockUpto;
    constexpr int left = offsetEnd - offset;
    if (blockLeft < left) {
      System::arraycopy(currentBlock, currentBlockUpto, b, offset, blockLeft);
      nextBlock();
      offset += blockLeft;
    } else {
      // Last block
      System::arraycopy(currentBlock, currentBlockUpto, b, offset, left);
      currentBlockUpto += left;
      break;
    }
  }
}

void PagedBytes::PagedBytesDataInput::nextBlock()
{
  currentBlockIndex++;
  currentBlockUpto = 0;
  currentBlock = outerInstance->blocks[currentBlockIndex];
}

PagedBytes::PagedBytesDataOutput::PagedBytesDataOutput(
    shared_ptr<PagedBytes> outerInstance)
    : outerInstance(outerInstance)
{
}

void PagedBytes::PagedBytesDataOutput::writeByte(char b)
{
  if (outerInstance->upto == outerInstance->blockSize) {
    if (outerInstance->currentBlock.size() > 0) {
      outerInstance->addBlock(outerInstance->currentBlock);
    }
    outerInstance->currentBlock = std::deque<char>(outerInstance->blockSize);
    outerInstance->upto = 0;
  }
  outerInstance->currentBlock[outerInstance->upto++] = b;
}

void PagedBytes::PagedBytesDataOutput::writeBytes(std::deque<char> &b,
                                                  int offset, int length)
{
  assert(b.size() >= offset + length);
  if (length == 0) {
    return;
  }

  if (outerInstance->upto == outerInstance->blockSize) {
    if (outerInstance->currentBlock.size() > 0) {
      outerInstance->addBlock(outerInstance->currentBlock);
    }
    outerInstance->currentBlock = std::deque<char>(outerInstance->blockSize);
    outerInstance->upto = 0;
  }

  constexpr int offsetEnd = offset + length;
  while (true) {
    constexpr int left = offsetEnd - offset;
    constexpr int blockLeft = outerInstance->blockSize - outerInstance->upto;
    if (blockLeft < left) {
      System::arraycopy(b, offset, outerInstance->currentBlock,
                        outerInstance->upto, blockLeft);
      outerInstance->addBlock(outerInstance->currentBlock);
      outerInstance->currentBlock = std::deque<char>(outerInstance->blockSize);
      outerInstance->upto = 0;
      offset += blockLeft;
    } else {
      // Last block
      System::arraycopy(b, offset, outerInstance->currentBlock,
                        outerInstance->upto, left);
      outerInstance->upto += left;
      break;
    }
  }
}

int64_t PagedBytes::PagedBytesDataOutput::getPosition()
{
  return outerInstance->getPointer();
}

shared_ptr<PagedBytesDataInput> PagedBytes::getDataInput()
{
  if (!frozen) {
    throw make_shared<IllegalStateException>(
        L"must call freeze() before getDataInput");
  }
  return make_shared<PagedBytesDataInput>(shared_from_this());
}

shared_ptr<PagedBytesDataOutput> PagedBytes::getDataOutput()
{
  if (frozen) {
    throw make_shared<IllegalStateException>(
        L"cannot get DataOutput after freeze()");
  }
  return make_shared<PagedBytesDataOutput>(shared_from_this());
}
} // namespace org::apache::lucene::util