using namespace std;

#include "TestFixedLengthBytesRefArray.h"

namespace org::apache::lucene::util
{

void TestFixedLengthBytesRefArray::testBasic() 
{
  shared_ptr<FixedLengthBytesRefArray> a =
      make_shared<FixedLengthBytesRefArray>(Integer::BYTES);
  int numValues = 100;
  for (int i = 0; i < numValues; i++) {
    std::deque<char> bytes = {0, 0, 0, static_cast<char>(10 - i)};
    a->append(make_shared<BytesRef>(bytes));
  }

  shared_ptr<BytesRefIterator> iterator = a->iterator(
      make_shared<ComparatorAnonymousInnerClass>(shared_from_this(), a));

  shared_ptr<BytesRef> last = nullptr;

  int count = 0;
  while (true) {
    shared_ptr<BytesRef> bytes = iterator->next();
    if (bytes == nullptr) {
      break;
    }
    if (last != nullptr) {
      assertTrue(L"count=" + to_wstring(count) + L" last=" + last + L" bytes=" +
                     bytes,
                 last->compareTo(bytes) < 0);
    }
    last = BytesRef::deepCopyOf(bytes);
    count++;
  }

  assertEquals(numValues, count);
}

TestFixedLengthBytesRefArray::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<TestFixedLengthBytesRefArray> outerInstance,
        shared_ptr<org::apache::lucene::util::FixedLengthBytesRefArray> a)
{
  this->outerInstance = outerInstance;
  this->a = a;
}

int TestFixedLengthBytesRefArray::ComparatorAnonymousInnerClass::compare(
    shared_ptr<BytesRef> a, shared_ptr<BytesRef> b)
{
  return a->compareTo(b);
}

void TestFixedLengthBytesRefArray::testRandom() 
{
  int length = TestUtil::nextInt(random(), 4, 10);
  int count = atLeast(10000);
  std::deque<std::shared_ptr<BytesRef>> values(count);

  shared_ptr<FixedLengthBytesRefArray> a =
      make_shared<FixedLengthBytesRefArray>(length);
  for (int i = 0; i < count; i++) {
    shared_ptr<BytesRef> value =
        make_shared<BytesRef>(std::deque<char>(length));
    random()->nextBytes(value->bytes);
    values[i] = value;
    a->append(value);
  }

  Arrays::sort(values);
  shared_ptr<BytesRefIterator> iterator = a->iterator(
      make_shared<ComparatorAnonymousInnerClass2>(shared_from_this(), a));
  for (int i = 0; i < count; i++) {
    shared_ptr<BytesRef> next = iterator->next();
    assertNotNull(next);
    assertEquals(values[i], next);
  }
}

TestFixedLengthBytesRefArray::ComparatorAnonymousInnerClass2::
    ComparatorAnonymousInnerClass2(
        shared_ptr<TestFixedLengthBytesRefArray> outerInstance,
        shared_ptr<org::apache::lucene::util::FixedLengthBytesRefArray> a)
{
  this->outerInstance = outerInstance;
  this->a = a;
}

int TestFixedLengthBytesRefArray::ComparatorAnonymousInnerClass2::compare(
    shared_ptr<BytesRef> a, shared_ptr<BytesRef> b)
{
  return a->compareTo(b);
}
} // namespace org::apache::lucene::util