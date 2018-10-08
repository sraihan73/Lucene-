using namespace std;

#include "TestIntsRef.h"

namespace org::apache::lucene::util
{

void TestIntsRef::testEmpty()
{
  shared_ptr<IntsRef> i = make_shared<IntsRef>();
  assertEquals(IntsRef::EMPTY_INTS, i->ints);
  assertEquals(0, i->offset);
  assertEquals(0, i->length);
}

void TestIntsRef::testFromInts()
{
  std::deque<int> ints = {1, 2, 3, 4};
  shared_ptr<IntsRef> i = make_shared<IntsRef>(ints, 0, 4);
  assertEquals(ints, i->ints);
  assertEquals(0, i->offset);
  assertEquals(4, i->length);

  shared_ptr<IntsRef> i2 = make_shared<IntsRef>(ints, 1, 3);
  assertEquals(make_shared<IntsRef>(std::deque<int>{2, 3, 4}, 0, 3), i2);

  assertFalse(i->equals(i2));
}
} // namespace org::apache::lucene::util