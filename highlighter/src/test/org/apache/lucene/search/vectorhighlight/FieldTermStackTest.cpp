using namespace std;

#include "FieldTermStackTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using Term = org::apache::lucene::index::Term;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using TermInfo =
    org::apache::lucene::search::vectorhighlight::FieldTermStack::TermInfo;
using TestUtil = org::apache::lucene::util::TestUtil;

void FieldTermStackTest::test1Term() 
{
  makeIndex();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"a"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(6, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(0,1,0)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(2,3,1)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(4,5,2)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(12,13,6)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(28,29,14)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(32,33,16)", stack->pop()->toString());
}

void FieldTermStackTest::test2Terms() 
{
  makeIndex();

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(tq(L"b"), Occur::SHOULD);
  query->add(tq(L"c"), Occur::SHOULD);
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(8, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"b(6,7,3)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"b(8,9,4)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"c(10,11,5)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"b(14,15,7)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"b(16,17,8)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"c(18,19,9)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"b(26,27,13)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"b(30,31,15)", stack->pop()->toString());
}

void FieldTermStackTest::test1Phrase() 
{
  makeIndex();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"c", L"d"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(3, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"c(10,11,5)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"c(18,19,9)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"d(20,21,10)", stack->pop()->toString());
}

void FieldTermStackTest::makeIndex() 
{
  //           111111111122222
  // 0123456789012345678901234 (offsets)
  // a a a b b c a b b c d e f
  // 0 1 2 3 4 5 6 7 8 9101112 (position)
  wstring value1 = L"a a a b b c a b b c d e f";
  // 222233333
  // 678901234 (offsets)
  // b a b a f
  // 1314151617 (position)
  wstring value2 = L"b a b a f";

  make1dmfIndex({value1, value2});
}

void FieldTermStackTest::test1TermB() 
{
  makeIndexB();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"ab"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(2, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(2,4,2)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(6,8,6)", stack->pop()->toString());
}

void FieldTermStackTest::test2TermsB() 
{
  makeIndexB();

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(tq(L"bc"), Occur::SHOULD);
  query->add(tq(L"ef"), Occur::SHOULD);
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(3, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bc(4,6,4)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bc(8,10,8)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ef(11,13,11)", stack->pop()->toString());
}

void FieldTermStackTest::test1PhraseB() 
{
  makeIndexB();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"ab", L"bb"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(4, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(2,4,2)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bb(3,5,3)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(6,8,6)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"bb(7,9,7)", stack->pop()->toString());
}

void FieldTermStackTest::makeIndexB() 
{
  //                             1 11 11
  // 01 12 23 34 45 56 67 78 89 90 01 12 (offsets)
  // aa|aa|ab|bb|bc|ca|ab|bb|bc|cd|de|ef
  //  0  1  2  3  4  5  6  7  8  9 10 11 (position)
  wstring value = L"aaabbcabbcdef";

  make1dmfIndexB({value});
}

void FieldTermStackTest::test1PhraseShortMV() 
{
  makeIndexShortMV();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"d"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(1, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"d(9,10,3)", stack->pop()->toString());
}

void FieldTermStackTest::test1PhraseLongMV() 
{
  makeIndexLongMV();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"search", L"engines"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(4, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"search(102,108,14)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"engines(109,116,15)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"search(157,163,24)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"engines(164,171,25)", stack->pop()->toString());
}

void FieldTermStackTest::test1PhraseMVB() 
{
  makeIndexLongMVB();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"sp", L"pe", L"ee", L"ed"}), true,
                              true); // "speed" -(2gram)-> "sp","pe","ee","ed"
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(4, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"sp(88,90,61)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"pe(89,91,62)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ee(90,92,63)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ed(91,93,64)", stack->pop()->toString());
}

void FieldTermStackTest::testWildcard() 
{
  makeIndexLongMV();
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(
      make_shared<WildcardQuery>(make_shared<Term>(F, L"th*e")), reader, true,
      true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  TestUtil::assertEquals(4, stack->termList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"the(15,18,2)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"these(133,138,20)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"the(153,156,23)", stack->pop()->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"the(195,198,31)", stack->pop()->toString());
}

void FieldTermStackTest::testTermInfoComparisonConsistency()
{
  shared_ptr<TermInfo> a = make_shared<TermInfo>(
      TestUtil::randomUnicodeString(random()), 0, 0, 0, 1);
  shared_ptr<TermInfo> b = make_shared<TermInfo>(
      TestUtil::randomUnicodeString(random()), 0, 0, 1, 1);
  shared_ptr<TermInfo> c = make_shared<TermInfo>(
      TestUtil::randomUnicodeString(random()), 0, 0, 2, 1);
  shared_ptr<TermInfo> d = make_shared<TermInfo>(
      TestUtil::randomUnicodeString(random()), 0, 0, 0, 1);

  assertConsistentEquals(a, a);
  assertConsistentEquals(b, b);
  assertConsistentEquals(c, c);
  assertConsistentEquals(d, d);
  assertConsistentEquals(a, d);
  assertConsistentLessThan(a, b);
  assertConsistentLessThan(b, c);
  assertConsistentLessThan(a, c);
  assertConsistentLessThan(d, b);
  assertConsistentLessThan(d, c);
}

template <typename T>
void FieldTermStackTest::assertConsistentEquals(T a, T b)
{
  static_assert(is_base_of<Comparable<T>, T>::value,
                L"T must inherit from Comparable< T >");

  TestUtil::assertEquals(a, b);
  TestUtil::assertEquals(b, a);
  TestUtil::assertEquals(a.hashCode(), b.hashCode());
  TestUtil::assertEquals(0, a.compareTo(b));
  TestUtil::assertEquals(0, b.compareTo(a));
}

template <typename T>
void FieldTermStackTest::assertConsistentLessThan(T a, T b)
{
  static_assert(is_base_of<Comparable<T>, T>::value,
                L"T must inherit from Comparable< T >");

  assertFalse(a.equals(b));
  assertFalse(b.equals(a));
  assertFalse(a.hashCode() == b.hashCode());
  assertTrue(a.compareTo(b) < 0);
  assertTrue(b.compareTo(a) > 0);
}
} // namespace org::apache::lucene::search::vectorhighlight