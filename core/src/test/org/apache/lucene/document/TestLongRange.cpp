using namespace std;

#include "TestLongRange.h"

namespace org::apache::lucene::document
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestLongRange::testToString()
{
  shared_ptr<LongRange> range =
      make_shared<LongRange>(L"foo", std::deque<int64_t>{1, 11, 21, 31},
                             std::deque<int64_t>{2, 12, 22, 32});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"LongRange <foo: [1 : 2] [11 : 12] [21 : 22] [31 : 32]>",
               range->toString());
}
} // namespace org::apache::lucene::document