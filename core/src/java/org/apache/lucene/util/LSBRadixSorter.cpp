using namespace std;

#include "LSBRadixSorter.h"

namespace org::apache::lucene::util
{

void LSBRadixSorter::buildHistogram(std::deque<int> &array_, int len,
                                    std::deque<int> &histogram, int shift)
{
  for (int i = 0; i < len; ++i) {
    constexpr int b =
        (static_cast<int>(static_cast<unsigned int>(array_[i]) >> shift)) &
        0xFF;
    histogram[b] += 1;
  }
}

void LSBRadixSorter::sumHistogram(std::deque<int> &histogram)
{
  int accum = 0;
  for (int i = 0; i < HISTOGRAM_SIZE; ++i) {
    constexpr int count = histogram[i];
    histogram[i] = accum;
    accum += count;
  }
}

void LSBRadixSorter::reorder(std::deque<int> &array_, int len,
                             std::deque<int> &histogram, int shift,
                             std::deque<int> &dest)
{
  for (int i = 0; i < len; ++i) {
    constexpr int v = array_[i];
    constexpr int b =
        (static_cast<int>(static_cast<unsigned int>(v) >> shift)) & 0xFF;
    dest[histogram[b]++] = v;
  }
}

bool LSBRadixSorter::sort(std::deque<int> &array_, int len,
                          std::deque<int> &histogram, int shift,
                          std::deque<int> &dest)
{
  Arrays::fill(histogram, 0);
  buildHistogram(array_, len, histogram, shift);
  if (histogram[0] == len) {
    return false;
  }
  sumHistogram(histogram);
  reorder(array_, len, histogram, shift, dest);
  return true;
}

void LSBRadixSorter::insertionSort(std::deque<int> &array_, int off, int len)
{
  for (int i = off + 1, end = off + len; i < end; ++i) {
    for (int j = i; j > off; --j) {
      if (array_[j - 1] > array_[j]) {
        int tmp = array_[j - 1];
        array_[j - 1] = array_[j];
        array_[j] = tmp;
      } else {
        break;
      }
    }
  }
}

void LSBRadixSorter::sort(int numBits, std::deque<int> &array_, int len)
{
  if (len < INSERTION_SORT_THRESHOLD) {
    insertionSort(array_, 0, len);
    return;
  }

  buffer = ArrayUtil::grow(buffer, len);

  std::deque<int> arr = array_;

  std::deque<int> buf = buffer;

  for (int shift = 0; shift < numBits; shift += 8) {
    if (sort(arr, len, histogram, shift, buf)) {
      // swap arrays
      std::deque<int> tmp = arr;
      arr = buf;
      buf = tmp;
    }
  }

  if (array_ == buf) {
    System::arraycopy(arr, 0, array_, 0, len);
  }
}
} // namespace org::apache::lucene::util