using namespace std;

#include "PointsStackTracker.h"

namespace org::apache::lucene::index
{
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using StringHelper = org::apache::lucene::util::StringHelper;

PointsStackTracker::Cell::Cell(shared_ptr<PointsStackTracker> outerInstance,
                               std::deque<char> &minPackedValue,
                               std::deque<char> &maxPackedValue)
    : minPackedValue(minPackedValue.clone()),
      maxPackedValue(maxPackedValue.clone()), outerInstance(outerInstance)
{
}

bool PointsStackTracker::Cell::contains(shared_ptr<Cell> other)
{
  for (int dim = 0; dim < outerInstance->numDims; dim++) {
    int offset = dim * outerInstance->bytesPerDim;
    // other.min < min?
    if (StringHelper::compare(outerInstance->bytesPerDim, other->minPackedValue,
                              offset, minPackedValue, offset) < 0) {
      return false;
    }
    // other.max > max?
    if (StringHelper::compare(outerInstance->bytesPerDim, other->maxPackedValue,
                              offset, maxPackedValue, offset) > 0) {
      return false;
    }
  }

  return true;
}

PointsStackTracker::PointsStackTracker(int numDims, int bytesPerDim)
    : numDims(numDims), bytesPerDim(bytesPerDim)
{
}

void PointsStackTracker::onCompare(std::deque<char> &minPackedValue,
                                   std::deque<char> &maxPackedValue)
{
  shared_ptr<Cell> cell =
      make_shared<Cell>(shared_from_this(), minPackedValue, maxPackedValue);

  // Pop stack:
  while (stack.size() > 0 && stack[stack.size() - 1]->contains(cell) == false) {
    stack.pop_back();
    // System.out.println("  pop");
  }

  // Push stack:
  stack.push_back(cell);
}
} // namespace org::apache::lucene::index