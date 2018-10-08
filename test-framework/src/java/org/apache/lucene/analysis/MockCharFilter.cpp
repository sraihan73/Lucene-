using namespace std;

#include "MockCharFilter.h"

namespace org::apache::lucene::analysis
{

MockCharFilter::MockCharFilter(shared_ptr<Reader> in_, int remainder)
    : CharFilter(in_), remainder(remainder)
{
  // TODO: instead of fixed remainder... maybe a fixed
  // random seed?
  if (remainder < 0 || remainder >= 10) {
    throw invalid_argument(L"invalid remainder parameter (must be 0..10): " +
                           to_wstring(remainder));
  }
}

MockCharFilter::MockCharFilter(shared_ptr<Reader> in_) : MockCharFilter(in_, 0)
{
}

int MockCharFilter::read() 
{
  // we have a buffered character, add an offset correction and return it
  if (bufferedCh >= 0) {
    int ch = bufferedCh;
    bufferedCh = -1;
    currentOffset++;

    addOffCorrectMap(currentOffset, delta - 1);
    delta--;
    return ch;
  }

  // otherwise actually read one
  int ch = input->read();
  if (ch < 0) {
    return ch;
  }

  currentOffset++;
  if ((ch % 10) != remainder ||
      Character::isHighSurrogate(static_cast<wchar_t>(ch)) ||
      Character::isLowSurrogate(static_cast<wchar_t>(ch))) {
    return ch;
  }

  // we will double this character, so buffer it.
  bufferedCh = ch;
  return ch;
}

int MockCharFilter::read(std::deque<wchar_t> &cbuf, int off,
                         int len) 
{
  int numRead = 0;
  for (int i = off; i < off + len; i++) {
    int c = read();
    if (c == -1) {
      break;
    }
    cbuf[i] = static_cast<wchar_t>(c);
    numRead++;
  }
  return numRead == 0 ? -1 : numRead;
}

int MockCharFilter::correct(int currentOff)
{
  unordered_map::Entry<int, int> lastEntry =
      corrections.lowerEntry(currentOff + 1);
  int ret =
      lastEntry == nullptr ? currentOff : currentOff + lastEntry.getValue();
  assert((ret >= 0, L"currentOff=" + to_wstring(currentOff) + L",diff=" +
                        to_wstring(ret - currentOff)));
  return ret;
}

void MockCharFilter::addOffCorrectMap(int off, int cumulativeDiff)
{
  corrections.emplace(off, cumulativeDiff);
}
} // namespace org::apache::lucene::analysis