using namespace std;

#include "BaseCharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"

namespace org::apache::lucene::analysis::charfilter
{
using CharFilter = org::apache::lucene::analysis::CharFilter;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

BaseCharFilter::BaseCharFilter(shared_ptr<Reader> in_)
    : org::apache::lucene::analysis::CharFilter(in_)
{
}

int BaseCharFilter::correct(int currentOff)
{
  if (offsets.empty()) {
    return currentOff;
  }

  int index = Arrays::binarySearch(offsets, 0, size, currentOff);
  if (index < -1) {
    index = -2 - index;
  }

  constexpr int diff = index < 0 ? 0 : diffs[index];
  return currentOff + diff;
}

int BaseCharFilter::getLastCumulativeDiff()
{
  return offsets.empty() ? 0 : diffs[size - 1];
}

void BaseCharFilter::addOffCorrectMap(int off, int cumulativeDiff)
{
  if (offsets.empty()) {
    offsets = std::deque<int>(64);
    diffs = std::deque<int>(64);
  } else if (size == offsets.size()) {
    offsets = ArrayUtil::grow(offsets);
    diffs = ArrayUtil::grow(diffs);
  }

  assert(size == 0 || off >= offsets[size - 1])
      : L"Offset #" + to_wstring(size) + L"(" + to_wstring(off) +
        L") is less than the last recorded offset "
        // C++ TODO: There is no native C++ equivalent to 'toString':
        + to_wstring(offsets[size - 1]) + L"\n" + Arrays->toString(offsets) +
        L"\n" + Arrays->toString(diffs);

  if (size == 0 || off != offsets[size - 1]) {
    offsets[size] = off;
    diffs[size++] = cumulativeDiff;
  } else { // Overwrite the diff at the last recorded offset
    diffs[size - 1] = cumulativeDiff;
  }
}
} // namespace org::apache::lucene::analysis::charfilter