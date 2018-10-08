using namespace std;

#include "IntroSelector.h"

namespace org::apache::lucene::util
{

void IntroSelector::select(int from, int to, int k)
{
  checkArgs(from, to, k);
  constexpr int maxDepth = 2 * MathUtil::log(to - from, 2);
  quickSelect(from, to, k, maxDepth);
}

void IntroSelector::slowSelect(int from, int to, int k)
{
  make_shared<SorterAnonymousInnerClass>(shared_from_this(), from, to)
      .sort(from, to);
}

IntroSelector::SorterAnonymousInnerClass::SorterAnonymousInnerClass(
    shared_ptr<IntroSelector> outerInstance, int from, int to)
{
  this->outerInstance = outerInstance;
  this->from = from;
  this->to = to;
}

void IntroSelector::SorterAnonymousInnerClass::swap(int i, int j)
{
  outerInstance->swap(i, j);
}

int IntroSelector::SorterAnonymousInnerClass::compare(int i, int j)
{
  return outerInstance->compare(i, j);
}

void IntroSelector::SorterAnonymousInnerClass::sort(int from, int to)
{
  heapSort(from, to);
}

void IntroSelector::quickSelect(int from, int to, int k, int maxDepth)
{
  assert(from <= k);
  assert(k < to);
  if (to - from == 1) {
    return;
  }
  if (--maxDepth < 0) {
    slowSelect(from, to, k);
    return;
  }

  constexpr int mid =
      static_cast<int>(static_cast<unsigned int>((from + to)) >> 1);
  // heuristic: we use the median of the values at from, to-1 and mid as a pivot
  if (compare(from, to - 1) > 0) {
    swap(from, to - 1);
  }
  if (compare(to - 1, mid) > 0) {
    swap(to - 1, mid);
    if (compare(from, to - 1) > 0) {
      swap(from, to - 1);
    }
  }

  setPivot(to - 1);

  int left = from + 1;
  int right = to - 2;

  for (;;) {
    while (comparePivot(left) > 0) {
      ++left;
    }

    while (left < right && comparePivot(right) <= 0) {
      --right;
    }

    if (left < right) {
      swap(left, right);
      --right;
    } else {
      break;
    }
  }
  swap(left, to - 1);

  if (left == k) {
    return;
  } else if (left < k) {
    quickSelect(left + 1, to, k, maxDepth);
  } else {
    quickSelect(from, left, k, maxDepth);
  }
}

int IntroSelector::compare(int i, int j)
{
  setPivot(i);
  return comparePivot(j);
}
} // namespace org::apache::lucene::util