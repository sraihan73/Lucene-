using namespace std;

#include "CharVector.h"

namespace org::apache::lucene::analysis::compound::hyphenation
{

CharVector::CharVector() : CharVector(DEFAULT_BLOCK_SIZE) {}

CharVector::CharVector(int capacity)
{
  if (capacity > 0) {
    blockSize = capacity;
  } else {
    blockSize = DEFAULT_BLOCK_SIZE;
  }
  array_ = std::deque<wchar_t>(blockSize);
  n = 0;
}

CharVector::CharVector(std::deque<wchar_t> &a)
{
  blockSize = DEFAULT_BLOCK_SIZE;
  array_ = a;
  n = a.size();
}

CharVector::CharVector(std::deque<wchar_t> &a, int capacity)
{
  if (capacity > 0) {
    blockSize = capacity;
  } else {
    blockSize = DEFAULT_BLOCK_SIZE;
  }
  array_ = a;
  n = a.size();
}

void CharVector::clear() { n = 0; }

shared_ptr<CharVector> CharVector::clone()
{
  shared_ptr<CharVector> cv =
      make_shared<CharVector>(array_.clone(), blockSize);
  cv->n = this->n;
  return cv;
}

std::deque<wchar_t> CharVector::getArray() { return array_; }

int CharVector::length() { return n; }

int CharVector::capacity() { return array_.size(); }

void CharVector::put(int index, wchar_t val) { array_[index] = val; }

wchar_t CharVector::get(int index) { return array_[index]; }

int CharVector::alloc(int size)
{
  int index = n;
  int len = array_.size();
  if (n + size >= len) {
    std::deque<wchar_t> aux(len + blockSize);
    System::arraycopy(array_, 0, aux, 0, len);
    array_ = aux;
  }
  n += size;
  return index;
}

void CharVector::trimToSize()
{
  if (n < array_.size()) {
    std::deque<wchar_t> aux(n);
    System::arraycopy(array_, 0, aux, 0, n);
    array_ = aux;
  }
}
} // namespace org::apache::lucene::analysis::compound::hyphenation