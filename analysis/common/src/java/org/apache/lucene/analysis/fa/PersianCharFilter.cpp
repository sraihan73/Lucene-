using namespace std;

#include "PersianCharFilter.h"

namespace org::apache::lucene::analysis::fa
{
using CharFilter = org::apache::lucene::analysis::CharFilter;

PersianCharFilter::PersianCharFilter(shared_ptr<Reader> in_)
    : org::apache::lucene::analysis::CharFilter(in_)
{
}

int PersianCharFilter::read(std::deque<wchar_t> &cbuf, int off,
                            int len) 
{
  constexpr int charsRead = input->read(cbuf, off, len);
  if (charsRead > 0) {
    constexpr int end = off + charsRead;
    while (off < end) {
      if (cbuf[off] == L'\u200C') {
        cbuf[off] = L' ';
      }
      off++;
    }
  }
  return charsRead;
}

int PersianCharFilter::read() 
{
  int ch = input->read();
  if (ch == L'\u200C') {
    return L' ';
  } else {
    return ch;
  }
}

int PersianCharFilter::correct(int currentOff)
{
  return currentOff; // we don't change the length of the string
}
} // namespace org::apache::lucene::analysis::fa