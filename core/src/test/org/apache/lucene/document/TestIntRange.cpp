using namespace std;

#include "TestIntRange.h"

namespace org::apache::lucene::document
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestIntRange::testToString()
{
  shared_ptr<IntRange> range = make_shared<IntRange>(
      L"foo", std::deque<int>{1, 11, 21, 31}, std::deque<int>{2, 12, 22, 32});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"IntRange <foo: [1 : 2] [11 : 12] [21 : 22] [31 : 32]>",
               range->toString());
}
} // namespace org::apache::lucene::document