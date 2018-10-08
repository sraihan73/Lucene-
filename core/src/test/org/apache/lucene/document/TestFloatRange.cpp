using namespace std;

#include "TestFloatRange.h"

namespace org::apache::lucene::document
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestFloatRange::testToString()
{
  shared_ptr<FloatRange> range = make_shared<FloatRange>(
      L"foo", std::deque<float>{0.1f, 1.1f, 2.1f, 3.1f},
      std::deque<float>{0.2f, 1.2f, 2.2f, 3.2f});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"FloatRange <foo: [0.1 : 0.2] [1.1 : 1.2] [2.1 : 2.2] [3.1 : 3.2]>",
      range->toString());
}
} // namespace org::apache::lucene::document