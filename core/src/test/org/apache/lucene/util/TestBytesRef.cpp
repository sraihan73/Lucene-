using namespace std;

#include "TestBytesRef.h"

namespace org::apache::lucene::util
{

void TestBytesRef::testEmpty()
{
  shared_ptr<BytesRef> b = make_shared<BytesRef>();
  assertEquals(BytesRef::EMPTY_BYTES, b->bytes);
  assertEquals(0, b->offset);
  assertEquals(0, b->length);
}

void TestBytesRef::testFromBytes()
{
  std::deque<char> bytes = {static_cast<char>(L'a'), static_cast<char>(L'b'),
                             static_cast<char>(L'c'), static_cast<char>(L'd')};
  shared_ptr<BytesRef> b = make_shared<BytesRef>(bytes);
  assertEquals(bytes, b->bytes);
  assertEquals(0, b->offset);
  assertEquals(4, b->length);

  shared_ptr<BytesRef> b2 = make_shared<BytesRef>(bytes, 1, 3);
  assertEquals(L"bcd", b2->utf8ToString());

  assertFalse(b->equals(b2));
}

void TestBytesRef::testFromChars()
{
  for (int i = 0; i < 100; i++) {
    wstring s = TestUtil::randomUnicodeString(random());
    wstring s2 = (make_shared<BytesRef>(s))->utf8ToString();
    assertEquals(s, s2);
  }

  // only for 4.x
  assertEquals(L"\uFFFF", (make_shared<BytesRef>(L"\uFFFF"))->utf8ToString());
}
} // namespace org::apache::lucene::util