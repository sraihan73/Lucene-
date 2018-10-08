using namespace std;

#include "TestInPlaceMergeSorter.h"

namespace org::apache::lucene::util
{
using com::carrotsearch::randomizedtesting::RandomizedRunner;
using org::junit::runner::RunWith;

TestInPlaceMergeSorter::TestInPlaceMergeSorter() : BaseSortTestCase(true) {}

shared_ptr<Sorter>
TestInPlaceMergeSorter::newSorter(std::deque<std::shared_ptr<Entry>> &arr)
{
  return make_shared<ArrayInPlaceMergeSorter<>>(arr,
                                                Comparator::naturalOrder());
}
} // namespace org::apache::lucene::util