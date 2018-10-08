using namespace std;

#include "TimSorter.h"

namespace org::apache::lucene::util
{

TimSorter::TimSorter(int maxTempSlots) : Sorter(), maxTempSlots(maxTempSlots)
{
  runEnds = std::deque<int>(1 + STACKSIZE);
}

int TimSorter::minRun(int length)
{
  assert(length >= MINRUN);
  int n = length;
  int r = 0;
  while (n >= 64) {
    r |= n & 1;
    n = static_cast<int>(static_cast<unsigned int>(n) >> 1);
  }
  constexpr int minRun = n + r;
  assert(minRun >= MINRUN && minRun <= THRESHOLD);
  return minRun;
}

int TimSorter::runLen(int i)
{
  constexpr int off = stackSize - i;
  return runEnds[off] - runEnds[off - 1];
}

int TimSorter::runBase(int i) { return runEnds[stackSize - i - 1]; }

int TimSorter::runEnd(int i) { return runEnds[stackSize - i]; }

void TimSorter::setRunEnd(int i, int runEnd)
{
  runEnds[stackSize - i] = runEnd;
}

void TimSorter::pushRunLen(int len)
{
  runEnds[stackSize + 1] = runEnds[stackSize] + len;
  ++stackSize;
}

int TimSorter::nextRun()
{
  constexpr int runBase = runEnd(0);
  assert(runBase < to);
  if (runBase == to - 1) {
    return 1;
  }
  int o = runBase + 2;
  if (compare(runBase, runBase + 1) > 0) {
    // run must be strictly descending
    while (o < to && compare(o - 1, o) > 0) {
      ++o;
    }
    reverse(runBase, o);
  } else {
    // run must be non-descending
    while (o < to && compare(o - 1, o) <= 0) {
      ++o;
    }
  }
  constexpr int runHi = max(o, min(to, runBase + minRun_));
  binarySort(runBase, runHi, o);
  return runHi - runBase;
}

void TimSorter::ensureInvariants()
{
  while (stackSize > 1) {
    constexpr int runLen0 = runLen(0);
    constexpr int runLen1 = runLen(1);

    if (stackSize > 2) {
      constexpr int runLen2 = runLen(2);

      if (runLen2 <= runLen1 + runLen0) {
        // merge the smaller of 0 and 2 with 1
        if (runLen2 < runLen0) {
          mergeAt(1);
        } else {
          mergeAt(0);
        }
        continue;
      }
    }

    if (runLen1 <= runLen0) {
      mergeAt(0);
      continue;
    }

    break;
  }
}

void TimSorter::exhaustStack()
{
  while (stackSize > 1) {
    mergeAt(0);
  }
}

void TimSorter::reset(int from, int to)
{
  stackSize = 0;
  Arrays::fill(runEnds, 0);
  runEnds[0] = from;
  this->to = to;
  constexpr int length = to - from;
  this->minRun_ = length <= THRESHOLD ? length : minRun(length);
}

void TimSorter::mergeAt(int n)
{
  assert(stackSize >= 2);
  merge(runBase(n + 1), runBase(n), runEnd(n));
  for (int j = n + 1; j > 0; --j) {
    setRunEnd(j, runEnd(j - 1));
  }
  --stackSize;
}

void TimSorter::merge(int lo, int mid, int hi)
{
  if (compare(mid - 1, mid) <= 0) {
    return;
  }
  lo = upper2(lo, mid, mid);
  hi = lower2(mid, hi, mid - 1);

  if (hi - mid <= mid - lo && hi - mid <= maxTempSlots) {
    mergeHi(lo, mid, hi);
  } else if (mid - lo <= maxTempSlots) {
    mergeLo(lo, mid, hi);
  } else {
    mergeInPlace(lo, mid, hi);
  }
}

void TimSorter::sort(int from, int to)
{
  checkRange(from, to);
  if (to - from <= 1) {
    return;
  }
  reset(from, to);
  do {
    ensureInvariants();
    pushRunLen(nextRun());
  } while (runEnd(0) < to);
  exhaustStack();
  assert(runEnd(0) == to);
}

void TimSorter::doRotate(int lo, int mid, int hi)
{
  constexpr int len1 = mid - lo;
  constexpr int len2 = hi - mid;
  if (len1 == len2) {
    while (mid < hi) {
      swap(lo++, mid++);
    }
  } else if (len2 < len1 && len2 <= maxTempSlots) {
    save(mid, len2);
    for (int i = lo + len1 - 1, j = hi - 1; i >= lo; --i, --j) {
      copy(i, j);
    }
    for (int i = 0, j = lo; i < len2; ++i, ++j) {
      restore(i, j);
    }
  } else if (len1 <= maxTempSlots) {
    save(lo, len1);
    for (int i = mid, j = lo; i < hi; ++i, ++j) {
      copy(i, j);
    }
    for (int i = 0, j = lo + len2; j < hi; ++i, ++j) {
      restore(i, j);
    }
  } else {
    reverse(lo, mid);
    reverse(mid, hi);
    reverse(lo, hi);
  }
}

void TimSorter::mergeLo(int lo, int mid, int hi)
{
  assert((compare(lo, mid) > 0));
  int len1 = mid - lo;
  save(lo, len1);
  copy(mid, lo);
  int i = 0, j = mid + 1, dest = lo + 1;
  for (;;) {
    for (int count = 0; count < MIN_GALLOP;) {
      if (i >= len1 || j >= hi) {
        goto outerBreak;
      } else if (compareSaved(i, j) <= 0) {
        restore(i++, dest++);
        count = 0;
      } else {
        copy(j++, dest++);
        ++count;
      }
    }
    // galloping...
    int next = lowerSaved3(j, hi, i);
    for (; j < next; ++dest) {
      copy(j++, dest);
    }
    restore(i++, dest++);
  outerContinue:;
  }
outerBreak:
  for (; i < len1; ++dest) {
    restore(i++, dest);
  }
  assert(j == dest);
}

void TimSorter::mergeHi(int lo, int mid, int hi)
{
  assert((compare(mid - 1, hi - 1) > 0));
  int len2 = hi - mid;
  save(mid, len2);
  copy(mid - 1, hi - 1);
  int i = mid - 2, j = len2 - 1, dest = hi - 2;
  for (;;) {
    for (int count = 0; count < MIN_GALLOP;) {
      if (i < lo || j < 0) {
        goto outerBreak;
      } else if (compareSaved(j, i) >= 0) {
        restore(j--, dest--);
        count = 0;
      } else {
        copy(i--, dest--);
        ++count;
      }
    }
    // galloping
    int next = upperSaved3(lo, i + 1, j);
    while (i >= next) {
      copy(i--, dest--);
    }
    restore(j--, dest--);
  outerContinue:;
  }
outerBreak:
  for (; j >= 0; --dest) {
    restore(j--, dest);
  }
  assert(i == dest);
}

int TimSorter::lowerSaved(int from, int to, int val)
{
  int len = to - from;
  while (len > 0) {
    constexpr int half = static_cast<int>(static_cast<unsigned int>(len) >> 1);
    constexpr int mid = from + half;
    if (compareSaved(val, mid) > 0) {
      from = mid + 1;
      len = len - half - 1;
    } else {
      len = half;
    }
  }
  return from;
}

int TimSorter::upperSaved(int from, int to, int val)
{
  int len = to - from;
  while (len > 0) {
    constexpr int half = static_cast<int>(static_cast<unsigned int>(len) >> 1);
    constexpr int mid = from + half;
    if (compareSaved(val, mid) < 0) {
      len = half;
    } else {
      from = mid + 1;
      len = len - half - 1;
    }
  }
  return from;
}

int TimSorter::lowerSaved3(int from, int to, int val)
{
  int f = from, t = f + 1;
  while (t < to) {
    if (compareSaved(val, t) <= 0) {
      return lowerSaved(f, t, val);
    }
    int delta = t - f;
    f = t;
    t += delta << 1;
  }
  return lowerSaved(f, to, val);
}

int TimSorter::upperSaved3(int from, int to, int val)
{
  int f = to - 1, t = to;
  while (f > from) {
    if (compareSaved(val, f) >= 0) {
      return upperSaved(f, t, val);
    }
    constexpr int delta = t - f;
    t = f;
    f -= delta << 1;
  }
  return upperSaved(from, t, val);
}
} // namespace org::apache::lucene::util