using namespace std;

#include "CharBlockArray.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{

CharBlockArray::Block::Block(int size) : chars(std::deque<wchar_t>(size))
{
  this->length = 0;
}

CharBlockArray::CharBlockArray() : CharBlockArray(DefaultBlockSize) {}

CharBlockArray::CharBlockArray(int blockSize)
{
  this->blocks = deque<>();
  this->blockSize = blockSize;
  addBlock();
}

void CharBlockArray::addBlock()
{
  if (blockSize * static_cast<int64_t>(blocks.size() + 1) >
      numeric_limits<int>::max()) {
    throw make_shared<IllegalStateException>(
        L"cannot store more than 2 GB in CharBlockArray");
  }
  this->current = make_shared<Block>(this->blockSize);
  this->blocks.push_back(this->current);
}

int CharBlockArray::blockIndex(int index) { return index / blockSize; }

int CharBlockArray::indexInBlock(int index) { return index % blockSize; }

shared_ptr<CharBlockArray>
CharBlockArray::append(shared_ptr<std::wstring> chars)
{
  return append(chars, 0, chars->length());
}

shared_ptr<CharBlockArray> CharBlockArray::append(wchar_t c)
{
  if (this->current->length == this->blockSize) {
    addBlock();
  }
  this->current->chars[this->current->length++] = c;
  this->length_++;

  return shared_from_this();
}

shared_ptr<CharBlockArray>
CharBlockArray::append(shared_ptr<std::wstring> chars, int start, int length)
{
  int end = start + length;
  for (int i = start; i < end; i++) {
    append(chars->charAt(i));
  }
  return shared_from_this();
}

shared_ptr<CharBlockArray> CharBlockArray::append(std::deque<wchar_t> &chars,
                                                  int start, int length)
{
  int offset = start;
  int remain = length;
  while (remain > 0) {
    if (this->current->length == this->blockSize) {
      addBlock();
    }
    int toCopy = remain;
    int remainingInBlock = this->blockSize - this->current->length;
    if (remainingInBlock < toCopy) {
      toCopy = remainingInBlock;
    }
    System::arraycopy(chars, offset, this->current->chars,
                      this->current->length, toCopy);
    offset += toCopy;
    remain -= toCopy;
    this->current->length += toCopy;
  }

  this->length_ += length;
  return shared_from_this();
}

shared_ptr<CharBlockArray> CharBlockArray::append(const wstring &s)
{
  int remain = s.length();
  int offset = 0;
  while (remain > 0) {
    if (this->current->length == this->blockSize) {
      addBlock();
    }
    int toCopy = remain;
    int remainingInBlock = this->blockSize - this->current->length;
    if (remainingInBlock < toCopy) {
      toCopy = remainingInBlock;
    }
    s.getChars(offset, offset + toCopy, this->current->chars,
               this->current->length);
    offset += toCopy;
    remain -= toCopy;
    this->current->length += toCopy;
  }

  this->length_ += s.length();
  return shared_from_this();
}

wchar_t CharBlockArray::charAt(int index)
{
  shared_ptr<Block> b = blocks[blockIndex(index)];
  return b->chars[indexInBlock(index)];
}

int CharBlockArray::length() { return this->length_; }

shared_ptr<std::wstring> CharBlockArray::subSequence(int start, int end)
{
  int remaining = end - start;
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>(remaining);
  int blockIdx = blockIndex(start);
  int indexInBlock = this->indexInBlock(start);
  while (remaining > 0) {
    shared_ptr<Block> b = blocks[blockIdx++];
    int numToAppend = min(remaining, b->length - indexInBlock);
    sb->append(b->chars, indexInBlock, numToAppend);
    remaining -= numToAppend;
    indexInBlock = 0; // 2nd+ iterations read from start of the block
  }
  return sb->toString();
}

wstring CharBlockArray::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (auto b : blocks) {
    sb->append(b->chars, 0, b->length);
  }
  return sb->toString();
}

void CharBlockArray::flush(shared_ptr<OutputStream> out) 
{
  shared_ptr<ObjectOutputStream> oos = nullptr;
  try {
    oos = make_shared<ObjectOutputStream>(out);
    oos->writeObject(shared_from_this());
    oos->flush();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (oos != nullptr) {
      oos->close();
    }
  }
}

shared_ptr<CharBlockArray>
CharBlockArray::open(shared_ptr<InputStream> in_) throw(IOException,
                                                        ClassNotFoundException)
{
  shared_ptr<ObjectInputStream> ois = nullptr;
  try {
    ois = make_shared<ObjectInputStream>(in_);
    shared_ptr<CharBlockArray> a =
        std::static_pointer_cast<CharBlockArray>(ois->readObject());
    return a;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (ois != nullptr) {
      ois->close();
    }
  }
}
} // namespace org::apache::lucene::facet::taxonomy::writercache