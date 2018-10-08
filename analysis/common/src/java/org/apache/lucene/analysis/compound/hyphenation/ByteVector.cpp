using namespace std;

#include "ByteVector.h"

namespace org::apache::lucene::analysis::compound::hyphenation
{

ByteVector::ByteVector() : ByteVector(DEFAULT_BLOCK_SIZE) {}

ByteVector::ByteVector(int capacity)
{
  if (capacity > 0) {
    blockSize = capacity;
  } else {
    blockSize = DEFAULT_BLOCK_SIZE;
  }
  array_ = std::deque<char>(blockSize);
  n = 0;
}

ByteVector::ByteVector(std::deque<char> &a)
{
  blockSize = DEFAULT_BLOCK_SIZE;
  array_ = a;
  n = 0;
}

ByteVector::ByteVector(std::deque<char> &a, int capacity)
{
  if (capacity > 0) {
    blockSize = capacity;
  } else {
    blockSize = DEFAULT_BLOCK_SIZE;
  }
  array_ = a;
  n = 0;
}

std::deque<char> ByteVector::getArray() { return array_; }

int ByteVector::length() { return n; }

int ByteVector::capacity() { return array_.size(); }

void ByteVector::put(int index, char val) { array_[index] = val; }

char ByteVector::get(int index) { return array_[index]; }

int ByteVector::alloc(int size)
{
  int index = n;
  int len = array_.size();
  if (n + size >= len) {
    std::deque<char> aux(len + blockSize);
    System::arraycopy(array_, 0, aux, 0, len);
    array_ = aux;
  }
  n += size;
  return index;
}

void ByteVector::trimToSize()
{
  if (n < array_.size()) {
    std::deque<char> aux(n);
    System::arraycopy(array_, 0, aux, 0, n);
    array_ = aux;
  }
}
} // namespace org::apache::lucene::analysis::compound::hyphenation