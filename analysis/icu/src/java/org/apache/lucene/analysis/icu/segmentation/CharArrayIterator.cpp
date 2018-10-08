using namespace std;

#include "CharArrayIterator.h"

namespace org::apache::lucene::analysis::icu::segmentation
{

std::deque<wchar_t> CharArrayIterator::getText() { return array_; }

int CharArrayIterator::getStart() { return start; }

int CharArrayIterator::getLength() { return length; }

void CharArrayIterator::setText(std::deque<wchar_t> &array_, int start,
                                int length)
{
  this->array_ = array_;
  this->start = start;
  this->index = start;
  this->length = length;
  this->limit = start + length;
}

wchar_t CharArrayIterator::current()
{
  return (index == limit) ? DONE : array_[index];
}

wchar_t CharArrayIterator::first()
{
  index = start;
  return current();
}

int CharArrayIterator::getBeginIndex() { return 0; }

int CharArrayIterator::getEndIndex() { return length; }

int CharArrayIterator::getIndex() { return index - start; }

wchar_t CharArrayIterator::last()
{
  index = (limit == start) ? limit : limit - 1;
  return current();
}

wchar_t CharArrayIterator::next()
{
  if (++index >= limit) {
    index = limit;
    return DONE;
  } else {
    return current();
  }
}

wchar_t CharArrayIterator::previous()
{
  if (--index < start) {
    index = start;
    return DONE;
  } else {
    return current();
  }
}

wchar_t CharArrayIterator::setIndex(int position)
{
  if (position < getBeginIndex() || position > getEndIndex()) {
    throw invalid_argument(L"Illegal Position: " + to_wstring(position));
  }
  index = start + position;
  return current();
}

shared_ptr<CharArrayIterator> CharArrayIterator::clone()
{
  shared_ptr<CharArrayIterator> clone = make_shared<CharArrayIterator>();
  clone->setText(array_, start, length);
  clone->index = index;
  return clone;
}
} // namespace org::apache::lucene::analysis::icu::segmentation