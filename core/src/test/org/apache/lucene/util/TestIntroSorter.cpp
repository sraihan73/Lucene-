using namespace std;

#include "TestIntroSorter.h"

namespace org::apache::lucene::util
{

TestIntroSorter::TestIntroSorter() : BaseSortTestCase(false) {}

shared_ptr<Sorter>
TestIntroSorter::newSorter(std::deque<std::shared_ptr<Entry>> &arr)
{
  return make_shared<ArrayIntroSorter<>>(arr, Comparator::naturalOrder());
}
} // namespace org::apache::lucene::util