using namespace std;

#include "TestDoubleRange.h"

namespace org::apache::lucene::document
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDoubleRange::testToString()
{
  shared_ptr<DoubleRange> range =
      make_shared<DoubleRange>(L"foo", std::deque<double>{0.1, 1.1, 2.1, 3.1},
                               std::deque<double>{.2, 1.2, 2.2, 3.2});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"DoubleRange <foo: [0.1 : 0.2] [1.1 : 1.2] [2.1 : 2.2] [3.1 : 3.2]>",
      range->toString());
}
} // namespace org::apache::lucene::document