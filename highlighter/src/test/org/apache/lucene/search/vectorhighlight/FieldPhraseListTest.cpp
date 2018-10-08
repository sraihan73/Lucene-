using namespace std;

#include "FieldPhraseListTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;
using Toffs = org::apache::lucene::search::vectorhighlight::FieldPhraseList::
    WeightedPhraseInfo::Toffs;
using TermInfo =
    org::apache::lucene::search::vectorhighlight::FieldTermStack::TermInfo;
using TestUtil = org::apache::lucene::util::TestUtil;

void FieldPhraseListTest::test1TermIndex() 
{
  make1d1fIndex(L"a");

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"a"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(1.0)((0,1))", fpl->phraseList.get(0)->toString());

  fq = make_shared<FieldQuery>(tq(L"b"), true, true);
  stack = make_shared<FieldTermStack>(reader, 0, F, fq);
  fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(0, fpl->phraseList.size());
}

void FieldPhraseListTest::test2TermsIndex() 
{
  make1d1fIndex(L"a a");

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"a"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(2, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(1.0)((0,1))", fpl->phraseList.get(0)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(1.0)((2,3))", fpl->phraseList.get(1)->toString());
}

void FieldPhraseListTest::test1PhraseIndex() 
{
  make1d1fIndex(L"a b");

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"a", L"b"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(1.0)((0,3))", fpl->phraseList.get(0)->toString());

  fq = make_shared<FieldQuery>(tq(L"b"), true, true);
  stack = make_shared<FieldTermStack>(reader, 0, F, fq);
  fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"b(1.0)((2,3))", fpl->phraseList.get(0)->toString());
}

void FieldPhraseListTest::test1PhraseIndexB() 
{
  // 01 12 23 34 45 56 67 78 (offsets)
  // bb|bb|ba|ac|cb|ba|ab|bc
  //  0  1  2  3  4  5  6  7 (positions)
  make1d1fIndexB(L"bbbacbabc");

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"ba", L"ac"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"baac(1.0)((2,5))",
                         fpl->phraseList.get(0)->toString());
}

void FieldPhraseListTest::test2ConcatTermsIndexB() 
{
  // 01 12 23 (offsets)
  // ab|ba|ab
  //  0  1  2 (positions)
  make1d1fIndexB(L"abab");

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"ab"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(2, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(1.0)((0,2))", fpl->phraseList.get(0)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(1.0)((2,4))", fpl->phraseList.get(1)->toString());
}

void FieldPhraseListTest::test2Terms1PhraseIndex() 
{
  make1d1fIndex(L"c a a b");

  // phraseHighlight = true
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"a", L"b"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(1.0)((4,7))", fpl->phraseList.get(0)->toString());

  // phraseHighlight = false
  fq = make_shared<FieldQuery>(pqF({L"a", L"b"}), false, true);
  stack = make_shared<FieldTermStack>(reader, 0, F, fq);
  fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(2, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a(1.0)((2,3))", fpl->phraseList.get(0)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(1.0)((4,7))", fpl->phraseList.get(1)->toString());
}

void FieldPhraseListTest::testPhraseSlop() 
{
  make1d1fIndex(L"c a a b c");

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF(2.0F, 1, {L"a", L"c"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ac(2.0)((4,5)(8,9))",
                         fpl->phraseList.get(0)->toString());
  TestUtil::assertEquals(4, fpl->phraseList.get(0).getStartOffset());
  TestUtil::assertEquals(9, fpl->phraseList.get(0).getEndOffset());
}

void FieldPhraseListTest::test2PhrasesOverlap() 
{
  make1d1fIndex(L"d a b c d");

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(pqF({L"a", L"b"}), Occur::SHOULD);
  query->add(pqF({L"b", L"c"}), Occur::SHOULD);
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"abc(1.0)((2,7))",
                         fpl->phraseList.get(0)->toString());
}

void FieldPhraseListTest::test3TermsPhrase() 
{
  make1d1fIndex(L"d a b a b c d");

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"a", L"b", L"c"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"abc(1.0)((6,11))",
                         fpl->phraseList.get(0)->toString());
}

void FieldPhraseListTest::testSearchLongestPhrase() 
{
  make1d1fIndex(L"d a b d c a b c");

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(pqF({L"a", L"b"}), Occur::SHOULD);
  query->add(pqF({L"a", L"b", L"c"}), Occur::SHOULD);
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(2, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"ab(1.0)((2,5))", fpl->phraseList.get(0)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"abc(1.0)((10,15))",
                         fpl->phraseList.get(1)->toString());
}

void FieldPhraseListTest::test1PhraseShortMV() 
{
  makeIndexShortMV();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"d"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"d(1.0)((9,10))", fpl->phraseList.get(0)->toString());
}

void FieldPhraseListTest::test1PhraseLongMV() 
{
  makeIndexLongMV();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"search", L"engines"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(2, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"searchengines(1.0)((102,116))",
                         fpl->phraseList.get(0)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"searchengines(1.0)((157,171))",
                         fpl->phraseList.get(1)->toString());
}

void FieldPhraseListTest::test1PhraseLongMVB() 
{
  makeIndexLongMVB();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"sp", L"pe", L"ee", L"ed"}), true,
                              true); // "speed" -(2gram)-> "sp","pe","ee","ed"
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  TestUtil::assertEquals(1, fpl->phraseList.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"sppeeeed(1.0)((88,93))",
                         fpl->phraseList.get(0)->toString());
}

void FieldPhraseListTest::testWeightedPhraseInfoComparisonConsistency()
{
  shared_ptr<WeightedPhraseInfo> a = newInfo(0, 0, 1);
  shared_ptr<WeightedPhraseInfo> b = newInfo(1, 2, 1);
  shared_ptr<WeightedPhraseInfo> c = newInfo(2, 3, 1);
  shared_ptr<WeightedPhraseInfo> d = newInfo(0, 0, 1);
  shared_ptr<WeightedPhraseInfo> e = newInfo(0, 0, 2);

  assertConsistentEquals(a, a);
  assertConsistentEquals(b, b);
  assertConsistentEquals(c, c);
  assertConsistentEquals(d, d);
  assertConsistentEquals(e, e);
  assertConsistentEquals(a, d);
  assertConsistentLessThan(a, b);
  assertConsistentLessThan(b, c);
  assertConsistentLessThan(a, c);
  assertConsistentLessThan(a, e);
  assertConsistentLessThan(e, b);
  assertConsistentLessThan(e, c);
  assertConsistentLessThan(d, b);
  assertConsistentLessThan(d, c);
  assertConsistentLessThan(d, e);
}

void FieldPhraseListTest::testToffsComparisonConsistency()
{
  shared_ptr<WeightedPhraseInfo::Toffs> a =
      make_shared<WeightedPhraseInfo::Toffs>(0, 0);
  shared_ptr<WeightedPhraseInfo::Toffs> b =
      make_shared<WeightedPhraseInfo::Toffs>(1, 2);
  shared_ptr<WeightedPhraseInfo::Toffs> c =
      make_shared<WeightedPhraseInfo::Toffs>(2, 3);
  shared_ptr<WeightedPhraseInfo::Toffs> d =
      make_shared<WeightedPhraseInfo::Toffs>(0, 0);

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

shared_ptr<WeightedPhraseInfo>
FieldPhraseListTest::newInfo(int startOffset, int endOffset, float boost)
{
  deque<std::shared_ptr<TermInfo>> infos = deque<std::shared_ptr<TermInfo>>();
  infos.push_back(make_shared<TermInfo>(TestUtil::randomUnicodeString(random()),
                                        startOffset, endOffset, 0, 0));
  return make_shared<WeightedPhraseInfo>(infos, boost);
}

template <typename T>
void FieldPhraseListTest::assertConsistentEquals(T a, T b)
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
void FieldPhraseListTest::assertConsistentLessThan(T a, T b)
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