using namespace std;

#include "TestTimSorter.h"

namespace org::apache::lucene::util
{

TestTimSorter::TestTimSorter() : BaseSortTestCase(true) {}

shared_ptr<Sorter>
TestTimSorter::newSorter(std::deque<std::shared_ptr<Entry>> &arr)
{
  return make_shared<ArrayTimSorter<>>(
      arr, Comparator::naturalOrder(),
      TestUtil::nextInt(random(), 0, arr.size()));
}
} // namespace org::apache::lucene::util