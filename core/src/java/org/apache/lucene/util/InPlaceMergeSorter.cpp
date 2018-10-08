using namespace std;

#include "InPlaceMergeSorter.h"

namespace org::apache::lucene::util
{

InPlaceMergeSorter::InPlaceMergeSorter() {}

void InPlaceMergeSorter::sort(int from, int to)
{
  checkRange(from, to);
  mergeSort(from, to);
}

void InPlaceMergeSorter::mergeSort(int from, int to)
{
  if (to - from < BINARY_SORT_THRESHOLD) {
    binarySort(from, to);
  } else {
    constexpr int mid =
        static_cast<int>(static_cast<unsigned int>((from + to)) >> 1);
    mergeSort(from, mid);
    mergeSort(mid, to);
    mergeInPlace(from, mid, to);
  }
}
} // namespace org::apache::lucene::util