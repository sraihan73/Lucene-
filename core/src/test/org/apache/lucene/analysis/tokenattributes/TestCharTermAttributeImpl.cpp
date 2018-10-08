using namespace std;

#include "TestCharTermAttributeImpl.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestCharTermAttributeImpl::testResize()
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  std::deque<wchar_t> content = (wstring(L"hello")).toCharArray();
  t->copyBuffer(content, 0, content.size());
  for (int i = 0; i < 2000; i++) {
    t->resizeBuffer(i);
    assertTrue(i <= t->buffer().size());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"hello", t->toString());
  }
}

void TestCharTermAttributeImpl::testSetLength()
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  std::deque<wchar_t> content = (wstring(L"hello")).toCharArray();
  t->copyBuffer(content, 0, content.size());
  expectThrows(out_of_range::typeid, [&]() { t->setLength(-1); });
}

void TestCharTermAttributeImpl::testGrow()
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  shared_ptr<StringBuilder> buf = make_shared<StringBuilder>(L"ab");
  for (int i = 0; i < 20; i++) {
    std::deque<wchar_t> content = buf->toString().toCharArray();
    t->copyBuffer(content, 0, content.size());
    TestUtil::assertEquals(buf->length(), t->length());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(buf->toString(), t->toString());
    buf->append(buf->toString());
  }
  TestUtil::assertEquals(1048576, t->length());

  // now as a StringBuilder, first variant
  t = make_shared<CharTermAttributeImpl>();
  buf = make_shared<StringBuilder>(L"ab");
  for (int i = 0; i < 20; i++) {
    t->setEmpty()->append(buf);
    TestUtil::assertEquals(buf->length(), t->length());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(buf->toString(), t->toString());
    buf->append(t);
  }
  TestUtil::assertEquals(1048576, t->length());

  // Test for slow growth to a long term
  t = make_shared<CharTermAttributeImpl>();
  buf = make_shared<StringBuilder>(L"a");
  for (int i = 0; i < 20000; i++) {
    t->setEmpty()->append(buf);
    TestUtil::assertEquals(buf->length(), t->length());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(buf->toString(), t->toString());
    buf->append(L"a");
  }
  TestUtil::assertEquals(20000, t->length());
}

void TestCharTermAttributeImpl::testToString() 
{
  std::deque<wchar_t> b = {L'a', L'l', L'o', L'h', L'a'};
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  t->copyBuffer(b, 0, 5);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"aloha", t->toString());

  t->setEmpty()->append(L"hi there");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"hi there", t->toString());
}

void TestCharTermAttributeImpl::testClone() 
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  std::deque<wchar_t> content = (wstring(L"hello")).toCharArray();
  t->copyBuffer(content, 0, 5);
  std::deque<wchar_t> buf = t->buffer();
  shared_ptr<CharTermAttributeImpl> copy = assertCloneIsEqual(t);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(t->toString(), copy->toString());
  assertNotSame(buf, copy->buffer());
}

void TestCharTermAttributeImpl::testEquals() 
{
  shared_ptr<CharTermAttributeImpl> t1a = make_shared<CharTermAttributeImpl>();
  std::deque<wchar_t> content1a = (wstring(L"hello")).toCharArray();
  t1a->copyBuffer(content1a, 0, 5);
  shared_ptr<CharTermAttributeImpl> t1b = make_shared<CharTermAttributeImpl>();
  std::deque<wchar_t> content1b = (wstring(L"hello")).toCharArray();
  t1b->copyBuffer(content1b, 0, 5);
  shared_ptr<CharTermAttributeImpl> t2 = make_shared<CharTermAttributeImpl>();
  std::deque<wchar_t> content2 = (wstring(L"hello2")).toCharArray();
  t2->copyBuffer(content2, 0, 6);
  assertTrue(t1a->equals(t1b));
  assertFalse(t1a->equals(t2));
  assertFalse(t2->equals(t1b));
}

void TestCharTermAttributeImpl::testCopyTo() 
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  shared_ptr<CharTermAttributeImpl> copy = assertCopyIsEqual(t);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"", t->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"", copy->toString());

  t = make_shared<CharTermAttributeImpl>();
  std::deque<wchar_t> content = (wstring(L"hello")).toCharArray();
  t->copyBuffer(content, 0, 5);
  std::deque<wchar_t> buf = t->buffer();
  copy = assertCopyIsEqual(t);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(t->toString(), copy->toString());
  assertNotSame(buf, copy->buffer());
}

void TestCharTermAttributeImpl::testAttributeReflection() 
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  t->append(L"foobar");
  TestUtil::assertAttributeReflection(
      t, make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
}

TestCharTermAttributeImpl::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestCharTermAttributeImpl> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(CharTermAttribute::typeid->getName() + L"#term", L"foobar");
  this->put(TermToBytesRefAttribute::typeid->getName() + L"#bytes",
            make_shared<BytesRef>(L"foobar"));
}

void TestCharTermAttributeImpl::testCharSequenceInterface()
{
  const wstring s = L"0123456789";
  shared_ptr<CharTermAttributeImpl> *const t =
      make_shared<CharTermAttributeImpl>();
  t->append(s);

  TestUtil::assertEquals(s.length(), t->length());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"12", t->substr(1, 2)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(s, t->substr(0, s.length())->toString());

  assertTrue(Pattern::matches(L"01\\d+", t));
  assertTrue(Pattern::matches(L"34", t->substr(3, 2)));

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(s.substr(3, 4)->toString(),
                         t->substr(3, 4)->toString());

  for (int i = 0; i < s.length(); i++) {
    assertTrue(t->charAt(i) == s[i]);
  }
}

void TestCharTermAttributeImpl::testAppendableInterface()
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  shared_ptr<Formatter> formatter = make_shared<Formatter>(t, Locale::ROOT);
  formatter->format(L"%d", 1234);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"1234", t->toString());
  formatter->format(L"%d", 5678);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"12345678", t->toString());
  t->append(L'9');
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"123456789", t->toString());
  t->append(std::static_pointer_cast<std::wstring>(L"0"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"1234567890", t->toString());
  t->append(std::static_pointer_cast<std::wstring>(L"0123456789"), 1, 3);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"123456789012", t->toString());
  t->append(std::static_pointer_cast<std::wstring>(
                CharBuffer::wrap((wstring(L"0123456789")).toCharArray())),
            3, 5);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"12345678901234", t->toString());
  t->append(std::static_pointer_cast<std::wstring>(t));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"1234567890123412345678901234", t->toString());
  t->append(std::static_pointer_cast<std::wstring>(
                make_shared<StringBuilder>(L"0123456789")),
            5, 7);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"123456789012341234567890123456", t->toString());
  t->append(
      std::static_pointer_cast<std::wstring>(make_shared<StringBuilder>(t)));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"123456789012341234567890123456123456789012341234567890123456",
      t->toString());
  // very wierd, to test if a subSlice is wrapped correct :)
  shared_ptr<CharBuffer> buf =
      CharBuffer::wrap((wstring(L"0123456789")).toCharArray(), 3, 5);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"34567", buf->toString());
  t->setEmpty()->append(std::static_pointer_cast<std::wstring>(buf), 1, 2);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"4", t->toString());
  shared_ptr<CharTermAttribute> t2 = make_shared<CharTermAttributeImpl>();
  t2->append(L"test");
  t->append(std::static_pointer_cast<std::wstring>(t2));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"4test", t->toString());
  t->append(std::static_pointer_cast<std::wstring>(t2), 1, 2);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"4teste", t->toString());

  expectThrows(out_of_range::typeid, [&]() {
    t->append(std::static_pointer_cast<std::wstring>(t2), 1, 5);
  });

  expectThrows(out_of_range::typeid, [&]() {
    t->append(std::static_pointer_cast<std::wstring>(t2), 1, 0);
  });

  t->append(std::static_pointer_cast<std::wstring>(nullptr));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"4testenull", t->toString());
}

void TestCharTermAttributeImpl::testAppendableInterfaceWithLongSequences()
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  t->append(std::static_pointer_cast<std::wstring>(
      L"01234567890123456789012345678901234567890123456789"));
  t->append(std::static_pointer_cast<std::wstring>(CharBuffer::wrap(
                (wstring(L"01234567890123456789012345678901234567890123456789"))
                    .toCharArray())),
            3, 50);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"0123456789012345678901234567890123456789012345678934"
                         L"567890123456789012345678901234567890123456789",
                         t->toString());
  t->setEmpty()->append(
      std::static_pointer_cast<std::wstring>(
          make_shared<StringBuilder>(L"01234567890123456789")),
      5, 17);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      std::static_pointer_cast<std::wstring>(L"567890123456"), t->toString());
  t->append(make_shared<StringBuilder>(t));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      std::static_pointer_cast<std::wstring>(L"567890123456567890123456"),
      t->toString());
  // very wierd, to test if a subSlice is wrapped correct :)
  shared_ptr<CharBuffer> buf = CharBuffer::wrap(
      (wstring(L"012345678901234567890123456789")).toCharArray(), 3, 15);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"345678901234567", buf->toString());
  t->setEmpty()->append(buf, 1, 14);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"4567890123456", t->toString());

  // finally use a completely custom std::wstring that is not catched by
  // instanceof checks
  const wstring longTestString = L"012345678901234567890123456789";
  t->append(make_shared<CharSequenceAnonymousInnerClass>(shared_from_this(),
                                                         longTestString));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"4567890123456" + longTestString, t->toString());
}

TestCharTermAttributeImpl::CharSequenceAnonymousInnerClass::
    CharSequenceAnonymousInnerClass(
        shared_ptr<TestCharTermAttributeImpl> outerInstance,
        const wstring &longTestString)
{
  this->outerInstance = outerInstance;
  this->longTestString = longTestString;
}

wchar_t
TestCharTermAttributeImpl::CharSequenceAnonymousInnerClass::charAt(int i)
{
  return longTestString[i];
}

int TestCharTermAttributeImpl::CharSequenceAnonymousInnerClass::length()
{
  return longTestString.length();
}

shared_ptr<std::wstring>
TestCharTermAttributeImpl::CharSequenceAnonymousInnerClass::subSequence(
    int start, int end)
{
  return longTestString.substr(start, end - start);
}

wstring TestCharTermAttributeImpl::CharSequenceAnonymousInnerClass::toString()
{
  return longTestString;
}

void TestCharTermAttributeImpl::testNonCharSequenceAppend()
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  t->append(L"0123456789");
  t->append(L"0123456789");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"01234567890123456789", t->toString());
  t->append(make_shared<StringBuilder>(L"0123456789"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"012345678901234567890123456789", t->toString());
  shared_ptr<CharTermAttribute> t2 = make_shared<CharTermAttributeImpl>();
  t2->append(L"test");
  t->append(t2);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"012345678901234567890123456789test", t->toString());
  t->append(static_cast<wstring>(nullptr));
  t->append(std::static_pointer_cast<StringBuilder>(nullptr));
  t->append(std::static_pointer_cast<CharTermAttribute>(nullptr));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"012345678901234567890123456789testnullnullnull",
                         t->toString());
}

void TestCharTermAttributeImpl::testExceptions()
{
  shared_ptr<CharTermAttributeImpl> t = make_shared<CharTermAttributeImpl>();
  t->append(L"test");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"test", t->toString());

  expectThrows(out_of_range::typeid, [&]() { t->charAt(-1); });

  expectThrows(out_of_range::typeid, [&]() { t->charAt(4); });

  expectThrows(out_of_range::typeid, [&]() { t->substr(0, 5); });

  expectThrows(out_of_range::typeid, [&]() { t->substr(5, -5); });
}

template <typename T>
T TestCharTermAttributeImpl::assertCloneIsEqual(T att)
{
  static_assert(is_base_of<org.apache.lucene.util.AttributeImpl, T>::value,
                L"T must inherit from org.apache.lucene.util.AttributeImpl");

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") T clone = (T) att.clone();
  T clone = static_cast<T>(att->clone());
  assertEquals(L"Clone must be equal", att, clone);
  assertEquals(L"Clone's hashcode must be equal", att->hashCode(),
               clone->hashCode());
  return clone;
}

template <typename T>
T TestCharTermAttributeImpl::assertCopyIsEqual(T att) 
{
  static_assert(is_base_of<org.apache.lucene.util.AttributeImpl, T>::value,
                L"T must inherit from org.apache.lucene.util.AttributeImpl");

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") T copy = (T)
  // att.getClass().newInstance();
  T copy = static_cast<T>(att->getClass().newInstance());
  att->copyTo(copy);
  assertEquals(L"Copied instance must be equal", att, copy);
  assertEquals(L"Copied instance's hashcode must be equal", att->hashCode(),
               copy->hashCode());
  return copy;
}
} // namespace org::apache::lucene::analysis::tokenattributes