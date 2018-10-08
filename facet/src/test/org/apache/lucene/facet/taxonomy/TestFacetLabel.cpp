using namespace std;

#include "TestFacetLabel.h"

namespace org::apache::lucene::facet::taxonomy
{
using FacetField = org::apache::lucene::facet::FacetField;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using SortedSetDocValuesFacetField =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetField;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasic()
void TestFacetLabel::testBasic()
{
  TestUtil::assertEquals(0, (make_shared<FacetLabel>())->length);
  TestUtil::assertEquals(1, (make_shared<FacetLabel>(L"hello"))->length);
  TestUtil::assertEquals(2,
                         (make_shared<FacetLabel>(L"hello", L"world"))->length);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testToString()
void TestFacetLabel::testToString()
{
  // When the category is empty, we expect an empty string
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"FacetLabel: []",
                         (make_shared<FacetLabel>())->toString());
  // one category
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"FacetLabel: [hello]",
                         (make_shared<FacetLabel>(L"hello"))->toString());
  // more than one category
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"FacetLabel: [hello, world]",
      (make_shared<FacetLabel>(L"hello", L"world"))->toString());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetComponent()
void TestFacetLabel::testGetComponent()
{
  std::deque<wstring> components(atLeast(10));
  for (int i = 0; i < components.size(); i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    components[i] = Integer::toString(i);
  }
  shared_ptr<FacetLabel> cp = make_shared<FacetLabel>(components);
  for (int i = 0; i < components.size(); i++) {
    TestUtil::assertEquals(i, stoi(cp->components[i]));
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDefaultConstructor()
void TestFacetLabel::testDefaultConstructor()
{
  // test that the default constructor (no parameters) currently
  // defaults to creating an object with a 0 initial capacity.
  // If we change this default later, we also need to change this
  // test.
  shared_ptr<FacetLabel> p = make_shared<FacetLabel>();
  TestUtil::assertEquals(0, p->length);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"FacetLabel: []", p->toString());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSubPath()
void TestFacetLabel::testSubPath()
{
  shared_ptr<FacetLabel> *const p =
      make_shared<FacetLabel>(L"hi", L"there", L"man");
  TestUtil::assertEquals(p->length, 3);

  shared_ptr<FacetLabel> p1 = p->subpath(2);
  TestUtil::assertEquals(2, p1->length);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"FacetLabel: [hi, there]", p1->toString());

  p1 = p->subpath(1);
  TestUtil::assertEquals(1, p1->length);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"FacetLabel: [hi]", p1->toString());

  p1 = p->subpath(0);
  TestUtil::assertEquals(0, p1->length);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"FacetLabel: []", p1->toString());

  // with all the following lengths, the prefix should be the whole path
  std::deque<int> lengths = {3, -1, 4};
  for (int i = 0; i < lengths.size(); i++) {
    p1 = p->subpath(lengths[i]);
    TestUtil::assertEquals(3, p1->length);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"FacetLabel: [hi, there, man]", p1->toString());
    TestUtil::assertEquals(p, p1);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEquals()
void TestFacetLabel::testEquals()
{
  TestUtil::assertEquals(make_shared<FacetLabel>(), make_shared<FacetLabel>());
  assertFalse(
      (make_shared<FacetLabel>())->equals(make_shared<FacetLabel>(L"hi")));
  assertFalse((make_shared<FacetLabel>())->equals(static_cast<Integer>(3)));
  TestUtil::assertEquals(make_shared<FacetLabel>(L"hello", L"world"),
                         make_shared<FacetLabel>(L"hello", L"world"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testHashCode()
void TestFacetLabel::testHashCode()
{
  TestUtil::assertEquals((make_shared<FacetLabel>())->hashCode(),
                         (make_shared<FacetLabel>())->hashCode());
  assertFalse((make_shared<FacetLabel>())->hashCode() ==
              (make_shared<FacetLabel>(L"hi"))->hashCode());
  TestUtil::assertEquals(
      (make_shared<FacetLabel>(L"hello", L"world"))->hashCode(),
      (make_shared<FacetLabel>(L"hello", L"world"))->hashCode());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLongHashCode()
void TestFacetLabel::testLongHashCode()
{
  TestUtil::assertEquals((make_shared<FacetLabel>())->longHashCode(),
                         (make_shared<FacetLabel>())->longHashCode());
  assertFalse((make_shared<FacetLabel>())->longHashCode() ==
              (make_shared<FacetLabel>(L"hi"))->longHashCode());
  TestUtil::assertEquals(
      (make_shared<FacetLabel>(L"hello", L"world"))->longHashCode(),
      (make_shared<FacetLabel>(L"hello", L"world"))->longHashCode());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testArrayConstructor()
void TestFacetLabel::testArrayConstructor()
{
  shared_ptr<FacetLabel> p = make_shared<FacetLabel>(L"hello", L"world", L"yo");
  TestUtil::assertEquals(3, p->length);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"FacetLabel: [hello, world, yo]", p->toString());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCompareTo()
void TestFacetLabel::testCompareTo()
{
  shared_ptr<FacetLabel> p = make_shared<FacetLabel>(L"a", L"b", L"c", L"d");
  shared_ptr<FacetLabel> pother =
      make_shared<FacetLabel>(L"a", L"b", L"c", L"d");
  TestUtil::assertEquals(0, pother->compareTo(p));
  TestUtil::assertEquals(0, p->compareTo(pother));
  pother = make_shared<FacetLabel>();
  assertTrue(pother->compareTo(p) < 0);
  assertTrue(p->compareTo(pother) > 0);
  pother = make_shared<FacetLabel>(L"a", L"b_", L"c", L"d");
  assertTrue(pother->compareTo(p) > 0);
  assertTrue(p->compareTo(pother) < 0);
  pother = make_shared<FacetLabel>(L"a", L"b", L"c");
  assertTrue(pother->compareTo(p) < 0);
  assertTrue(p->compareTo(pother) > 0);
  pother = make_shared<FacetLabel>(L"a", L"b", L"c", L"e");
  assertTrue(pother->compareTo(p) > 0);
  assertTrue(p->compareTo(pother) < 0);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptyNullComponents() throws Exception
void TestFacetLabel::testEmptyNullComponents() 
{
  // LUCENE-4724: CategoryPath should not allow empty or null components
  std::deque<std::deque<wstring>> components_tests = {
      std::deque<wstring>{L"", L"test"},
      std::deque<wstring>{L"test", L""},
      std::deque<wstring>{L"test", L"", L"foo"},
      std::deque<wstring>{L"", L"test"},
      std::deque<wstring>{L"test", L""},
      std::deque<wstring>{L"test", L"", L"foo"}};

  // empty or null components should not be allowed.
  for (auto components : components_tests) {
    expectThrows(invalid_argument::typeid,
                 [&]() { make_shared<FacetLabel>(components); });
    expectThrows(invalid_argument::typeid,
                 [&]() { make_shared<FacetField>(L"dim", components); });
    expectThrows(invalid_argument::typeid, [&]() {
      make_shared<AssociationFacetField>(make_shared<BytesRef>(), L"dim",
                                         components);
    });
    expectThrows(invalid_argument::typeid, [&]() {
      make_shared<IntAssociationFacetField>(17, L"dim", components);
    });
    expectThrows(invalid_argument::typeid, [&]() {
      make_shared<FloatAssociationFacetField>(17.0f, L"dim", components);
    });
  }

  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FacetField>(nullptr, std::deque<wstring>{L"abc"});
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FacetField>(L"", std::deque<wstring>{L"abc"});
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<IntAssociationFacetField>(17, nullptr,
                                          std::deque<wstring>{L"abc"});
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<IntAssociationFacetField>(17, L"",
                                          std::deque<wstring>{L"abc"});
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FloatAssociationFacetField>(17.0f, nullptr,
                                            std::deque<wstring>{L"abc"});
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FloatAssociationFacetField>(17.0f, L"",
                                            std::deque<wstring>{L"abc"});
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<AssociationFacetField>(make_shared<BytesRef>(), nullptr,
                                       std::deque<wstring>{L"abc"});
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<AssociationFacetField>(make_shared<BytesRef>(), L"",
                                       std::deque<wstring>{L"abc"});
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SortedSetDocValuesFacetField>(nullptr, L"abc");
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SortedSetDocValuesFacetField>(L"", L"abc");
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SortedSetDocValuesFacetField>(L"dim", nullptr);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<SortedSetDocValuesFacetField>(L"dim", L"");
  });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLongPath() throws Exception
void TestFacetLabel::testLongPath() 
{
  wstring bigComp = L"";
  while (true) {
    int len = FacetLabel::MAX_CATEGORY_PATH_LENGTH;
    bigComp = TestUtil::randomSimpleString(random(), len, len);
    if (bigComp.find(L'\u001f') != wstring::npos) {
      continue;
    }
    break;
  }

  // long paths should not be allowed
  const wstring longPath = bigComp;
  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<FacetLabel>(L"dim", longPath); });
}
} // namespace org::apache::lucene::facet::taxonomy