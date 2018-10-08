using namespace std;

#include "TestMutableValues.h"

namespace org::apache::lucene::util::mutable_
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using BytesRef = org::apache::lucene::util::BytesRef;

void TestMutableValues::testStr()
{
  shared_ptr<MutableValueStr> xxx = make_shared<MutableValueStr>();
  assert((xxx->value->get().equals(make_shared<BytesRef>()),
          L"defaults have changed, test utility may not longer be as high"));
  assert((xxx->exists_,
          L"defaults have changed, test utility may not longer be as high"));
  assertSanity(xxx);
  shared_ptr<MutableValueStr> yyy = make_shared<MutableValueStr>();
  assertSanity(yyy);

  assertEquality(xxx, yyy);

  xxx->exists_ = false;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->exists_ = false;
  assertEquality(xxx, yyy);

  xxx->value->clear();
  xxx->value->copyChars(L"zzz");
  xxx->exists_ = true;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->value->clear();
  yyy->value->copyChars(L"aaa");
  yyy->exists_ = true;
  assertSanity(yyy);

  assertInEquality(xxx, yyy);
  assertTrue(0 < xxx->compareTo(yyy));
  assertTrue(yyy->compareTo(xxx) < 0);

  xxx->copy(yyy);
  assertSanity(xxx);
  assertEquality(xxx, yyy);

  // special BytesRef considerations...

  xxx->exists_ = false;
  xxx->value->clear(); // but leave bytes alone
  assertInEquality(xxx, yyy);

  yyy->exists_ = false;
  yyy->value->clear(); // but leave bytes alone
  assertEquality(xxx, yyy);
}

void TestMutableValues::testDouble()
{
  shared_ptr<MutableValueDouble> xxx = make_shared<MutableValueDouble>();
  assert((xxx->value == 0.0,
          L"defaults have changed, test utility may not longer be as high"));
  assert((xxx->exists_,
          L"defaults have changed, test utility may not longer be as high"));
  assertSanity(xxx);
  shared_ptr<MutableValueDouble> yyy = make_shared<MutableValueDouble>();
  assertSanity(yyy);

  assertEquality(xxx, yyy);

  xxx->exists_ = false;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->exists_ = false;
  assertEquality(xxx, yyy);

  xxx->value = 42.0;
  xxx->exists_ = true;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->value = -99.0;
  yyy->exists_ = true;
  assertSanity(yyy);

  assertInEquality(xxx, yyy);
  assertTrue(0 < xxx->compareTo(yyy));
  assertTrue(yyy->compareTo(xxx) < 0);

  xxx->copy(yyy);
  assertSanity(xxx);
  assertEquality(xxx, yyy);
}

void TestMutableValues::testInt()
{
  shared_ptr<MutableValueInt> xxx = make_shared<MutableValueInt>();
  assert((xxx->value == 0,
          L"defaults have changed, test utility may not longer be as high"));
  assert((xxx->exists_,
          L"defaults have changed, test utility may not longer be as high"));
  assertSanity(xxx);
  shared_ptr<MutableValueInt> yyy = make_shared<MutableValueInt>();
  assertSanity(yyy);

  assertEquality(xxx, yyy);

  xxx->exists_ = false;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->exists_ = false;
  assertEquality(xxx, yyy);

  xxx->value = 42;
  xxx->exists_ = true;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->value = -99;
  yyy->exists_ = true;
  assertSanity(yyy);

  assertInEquality(xxx, yyy);
  assertTrue(0 < xxx->compareTo(yyy));
  assertTrue(yyy->compareTo(xxx) < 0);

  xxx->copy(yyy);
  assertSanity(xxx);
  assertEquality(xxx, yyy);
}

void TestMutableValues::testFloat()
{
  shared_ptr<MutableValueFloat> xxx = make_shared<MutableValueFloat>();
  assert((xxx->value == 0.0F,
          L"defaults have changed, test utility may not longer be as high"));
  assert((xxx->exists_,
          L"defaults have changed, test utility may not longer be as high"));
  assertSanity(xxx);
  shared_ptr<MutableValueFloat> yyy = make_shared<MutableValueFloat>();
  assertSanity(yyy);

  assertEquality(xxx, yyy);

  xxx->exists_ = false;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->exists_ = false;
  assertEquality(xxx, yyy);

  xxx->value = 42.0F;
  xxx->exists_ = true;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->value = -99.0F;
  yyy->exists_ = true;
  assertSanity(yyy);

  assertInEquality(xxx, yyy);
  assertTrue(0 < xxx->compareTo(yyy));
  assertTrue(yyy->compareTo(xxx) < 0);

  xxx->copy(yyy);
  assertSanity(xxx);
  assertEquality(xxx, yyy);
}

void TestMutableValues::testLong()
{
  shared_ptr<MutableValueLong> xxx = make_shared<MutableValueLong>();
  assert((xxx->value == 0LL,
          L"defaults have changed, test utility may not longer be as high"));
  assert((xxx->exists_,
          L"defaults have changed, test utility may not longer be as high"));
  assertSanity(xxx);
  shared_ptr<MutableValueLong> yyy = make_shared<MutableValueLong>();
  assertSanity(yyy);

  assertEquality(xxx, yyy);

  xxx->exists_ = false;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->exists_ = false;
  assertEquality(xxx, yyy);

  xxx->value = 42LL;
  xxx->exists_ = true;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->value = -99LL;
  yyy->exists_ = true;
  assertSanity(yyy);

  assertInEquality(xxx, yyy);
  assertTrue(0 < xxx->compareTo(yyy));
  assertTrue(yyy->compareTo(xxx) < 0);

  xxx->copy(yyy);
  assertSanity(xxx);
  assertEquality(xxx, yyy);
}

void TestMutableValues::testBool()
{
  shared_ptr<MutableValueBool> xxx = make_shared<MutableValueBool>();
  assert((xxx->value == false,
          L"defaults have changed, test utility may not longer be as high"));
  assert((xxx->exists_,
          L"defaults have changed, test utility may not longer be as high"));
  assertSanity(xxx);
  shared_ptr<MutableValueBool> yyy = make_shared<MutableValueBool>();
  assertSanity(yyy);

  assertEquality(xxx, yyy);

  xxx->exists_ = false;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->exists_ = false;
  assertEquality(xxx, yyy);

  xxx->value = true;
  xxx->exists_ = true;
  assertSanity(xxx);

  assertInEquality(xxx, yyy);

  yyy->value = false;
  yyy->exists_ = true;
  assertSanity(yyy);

  assertInEquality(xxx, yyy);
  assertTrue(0 < xxx->compareTo(yyy));
  assertTrue(yyy->compareTo(xxx) < 0);

  xxx->copy(yyy);
  assertSanity(xxx);
  assertEquality(xxx, yyy);
}

void TestMutableValues::assertSanity(shared_ptr<MutableValue> x)
{
  assertEquality(x, x);
  shared_ptr<MutableValue> y = x->duplicate();
  assertEquality(x, y);
}

void TestMutableValues::assertEquality(shared_ptr<MutableValue> x,
                                       shared_ptr<MutableValue> y)
{
  assertEquals(x->hashCode(), y->hashCode());

  assertEquals(x, y);
  assertEquals(y, x);

  assertTrue(x->equalsSameType(y));
  assertTrue(y->equalsSameType(x));

  assertEquals(0, x->compareTo(y));
  assertEquals(0, y->compareTo(x));

  assertEquals(0, x->compareSameType(y));
  assertEquals(0, y->compareSameType(x));
}

void TestMutableValues::assertInEquality(shared_ptr<MutableValue> x,
                                         shared_ptr<MutableValue> y)
{
  assertFalse(x->equals(y));
  assertFalse(y->equals(x));

  assertFalse(x->equalsSameType(y));
  assertFalse(y->equalsSameType(x));

  assertFalse(0 == x->compareTo(y));
  assertFalse(0 == y->compareTo(x));
}
} // namespace org::apache::lucene::util::mutable_