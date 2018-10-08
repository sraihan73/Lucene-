using namespace std;

#include "TestFilterIterator.h"

namespace org::apache::lucene::util
{

const shared_ptr<java::util::Set<wstring>> TestFilterIterator::set =
    set<wstring>(java::util::Arrays::asList(L"a", L"b", L"c"));

template <typename T1>
void TestFilterIterator::assertNoMore(shared_ptr<Iterator<T1>> it)
{
  assertFalse(it->hasNext());
  expectThrows(NoSuchElementException::typeid, [&]() { it->next(); });
  assertFalse(it->hasNext());
}

void TestFilterIterator::testEmpty()
{
  constexpr Set<wstring>::const_iterator it =
      make_shared<FilterIteratorAnonymousInnerClass>(shared_from_this(),
                                                     set->begin());
  assertNoMore(it);
}

TestFilterIterator::FilterIteratorAnonymousInnerClass::
    FilterIteratorAnonymousInnerClass(
        shared_ptr<TestFilterIterator> outerInstance,
        shared_ptr<UnknownType> iterator)
    : FilterIterator<std::wstring, std::wstring>(iterator)
{
  this->outerInstance = outerInstance;
}

bool TestFilterIterator::FilterIteratorAnonymousInnerClass::predicateFunction(
    const wstring &s)
{
  return false;
}

void TestFilterIterator::testA1()
{
  constexpr Set<wstring>::const_iterator it =
      make_shared<FilterIteratorAnonymousInnerClass2>(shared_from_this(),
                                                      set->begin());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertTrue(it->hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"a", it->next());
  assertNoMore(it);
}

TestFilterIterator::FilterIteratorAnonymousInnerClass2::
    FilterIteratorAnonymousInnerClass2(
        shared_ptr<TestFilterIterator> outerInstance,
        shared_ptr<UnknownType> iterator)
    : FilterIterator<std::wstring, std::wstring>(iterator)
{
  this->outerInstance = outerInstance;
}

bool TestFilterIterator::FilterIteratorAnonymousInnerClass2::predicateFunction(
    const wstring &s)
{
  return L"a" == s;
}

void TestFilterIterator::testA2()
{
  constexpr Set<wstring>::const_iterator it =
      make_shared<FilterIteratorAnonymousInnerClass3>(shared_from_this(),
                                                      set->begin());
  // this time without check: assertTrue(it.hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"a", it->next());
  assertNoMore(it);
}

TestFilterIterator::FilterIteratorAnonymousInnerClass3::
    FilterIteratorAnonymousInnerClass3(
        shared_ptr<TestFilterIterator> outerInstance,
        shared_ptr<UnknownType> iterator)
    : FilterIterator<std::wstring, std::wstring>(iterator)
{
  this->outerInstance = outerInstance;
}

bool TestFilterIterator::FilterIteratorAnonymousInnerClass3::predicateFunction(
    const wstring &s)
{
  return L"a" == s;
}

void TestFilterIterator::testB1()
{
  constexpr Set<wstring>::const_iterator it =
      make_shared<FilterIteratorAnonymousInnerClass4>(shared_from_this(),
                                                      set->begin());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertTrue(it->hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"b", it->next());
  assertNoMore(it);
}

TestFilterIterator::FilterIteratorAnonymousInnerClass4::
    FilterIteratorAnonymousInnerClass4(
        shared_ptr<TestFilterIterator> outerInstance,
        shared_ptr<UnknownType> iterator)
    : FilterIterator<std::wstring, std::wstring>(iterator)
{
  this->outerInstance = outerInstance;
}

bool TestFilterIterator::FilterIteratorAnonymousInnerClass4::predicateFunction(
    const wstring &s)
{
  return L"b" == s;
}

void TestFilterIterator::testB2()
{
  constexpr Set<wstring>::const_iterator it =
      make_shared<FilterIteratorAnonymousInnerClass5>(shared_from_this(),
                                                      set->begin());
  // this time without check: assertTrue(it.hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"b", it->next());
  assertNoMore(it);
}

TestFilterIterator::FilterIteratorAnonymousInnerClass5::
    FilterIteratorAnonymousInnerClass5(
        shared_ptr<TestFilterIterator> outerInstance,
        shared_ptr<UnknownType> iterator)
    : FilterIterator<std::wstring, std::wstring>(iterator)
{
  this->outerInstance = outerInstance;
}

bool TestFilterIterator::FilterIteratorAnonymousInnerClass5::predicateFunction(
    const wstring &s)
{
  return L"b" == s;
}

void TestFilterIterator::testAll1()
{
  constexpr Set<wstring>::const_iterator it =
      make_shared<FilterIteratorAnonymousInnerClass6>(shared_from_this(),
                                                      set->begin());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertTrue(it->hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"a", it->next());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertTrue(it->hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"b", it->next());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertTrue(it->hasNext());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"c", it->next());
  assertNoMore(it);
}

TestFilterIterator::FilterIteratorAnonymousInnerClass6::
    FilterIteratorAnonymousInnerClass6(
        shared_ptr<TestFilterIterator> outerInstance,
        shared_ptr<UnknownType> iterator)
    : FilterIterator<std::wstring, std::wstring>(iterator)
{
  this->outerInstance = outerInstance;
}

bool TestFilterIterator::FilterIteratorAnonymousInnerClass6::predicateFunction(
    const wstring &s)
{
  return true;
}

void TestFilterIterator::testAll2()
{
  constexpr Set<wstring>::const_iterator it =
      make_shared<FilterIteratorAnonymousInnerClass7>(shared_from_this(),
                                                      set->begin());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"a", it->next());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"b", it->next());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"c", it->next());
  assertNoMore(it);
}

TestFilterIterator::FilterIteratorAnonymousInnerClass7::
    FilterIteratorAnonymousInnerClass7(
        shared_ptr<TestFilterIterator> outerInstance,
        shared_ptr<UnknownType> iterator)
    : FilterIterator<std::wstring, std::wstring>(iterator)
{
  this->outerInstance = outerInstance;
}

bool TestFilterIterator::FilterIteratorAnonymousInnerClass7::predicateFunction(
    const wstring &s)
{
  return true;
}

void TestFilterIterator::testUnmodifiable()
{
  constexpr Set<wstring>::const_iterator it =
      make_shared<FilterIteratorAnonymousInnerClass8>(shared_from_this(),
                                                      set->begin());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertEquals(L"a", it->next());
  expectThrows(UnsupportedOperationException::typeid, [&]() { it->remove(); });
}

TestFilterIterator::FilterIteratorAnonymousInnerClass8::
    FilterIteratorAnonymousInnerClass8(
        shared_ptr<TestFilterIterator> outerInstance,
        shared_ptr<UnknownType> iterator)
    : FilterIterator<std::wstring, std::wstring>(iterator)
{
  this->outerInstance = outerInstance;
}

bool TestFilterIterator::FilterIteratorAnonymousInnerClass8::predicateFunction(
    const wstring &s)
{
  return true;
}
} // namespace org::apache::lucene::util