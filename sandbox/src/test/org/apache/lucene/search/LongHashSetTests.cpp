using namespace std;

#include "LongHashSetTests.h"

namespace org::apache::lucene::search
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void LongHashSetTests::assertEquals(shared_ptr<Set<int64_t>> set1,
                                    shared_ptr<LongHashSet> set2)
{
  LuceneTestCase::assertEquals(set1, set2);
  LuceneTestCase::assertEquals(set2, set1);
  LuceneTestCase::assertEquals(set2, set2);
  assertEquals(set1->hashCode(), set2->hashCode());

  if (set1->isEmpty() == false) {
    shared_ptr<Set<int64_t>> set3 = unordered_set<int64_t>(set1);
    int64_t removed = set3->begin()->next();
    while (true) {
      int64_t next = random()->nextLong();
      if (next != removed && set3->add(next)) {
        break;
      }
    }
    assertNotEquals(set3, set2);
  }
}

void LongHashSetTests::assertNotEquals(shared_ptr<Set<int64_t>> set1,
                                       shared_ptr<LongHashSet> set2)
{
  assertFalse(set1->equals(set2));
  assertFalse(set2->equals(set1));
  shared_ptr<LongHashSet> set3 = make_shared<LongHashSet>(
      set1->stream().mapToLong(optional<int64_t>::longValue).toArray());
  assertFalse(set2->equals(set3));
}

void LongHashSetTests::testEmpty()
{
  shared_ptr<Set<int64_t>> set1 = unordered_set<int64_t>();
  shared_ptr<LongHashSet> set2 = make_shared<LongHashSet>();
  assertEquals(set1, set2);
}

void LongHashSetTests::testOneValue()
{
  shared_ptr<Set<int64_t>> set1 =
      unordered_set<int64_t>(Arrays::asList(42LL));
  shared_ptr<LongHashSet> set2 = make_shared<LongHashSet>(42);
  assertEquals(set1, set2);

  set1 = unordered_set<>(Arrays::asList(numeric_limits<int64_t>::min()));
  set2 = make_shared<LongHashSet>(numeric_limits<int64_t>::min());
  assertEquals(set1, set2);
}

void LongHashSetTests::testTwoValues()
{
  shared_ptr<Set<int64_t>> set1 = unordered_set<int64_t>(
      Arrays::asList(42LL, numeric_limits<int64_t>::max()));
  shared_ptr<LongHashSet> set2 =
      make_shared<LongHashSet>(42, numeric_limits<int64_t>::max());
  assertEquals(set1, set2);

  set1 =
      unordered_set<>(Arrays::asList(numeric_limits<int64_t>::min(), 42LL));
  set2 = make_shared<LongHashSet>(numeric_limits<int64_t>::min(), 42LL);
  assertEquals(set1, set2);
}

void LongHashSetTests::testRandom()
{
  constexpr int iters = atLeast(10);
  for (int iter = 0; iter < iters; ++iter) {
    std::deque<int64_t> values(
        random()->nextInt(1 << random()->nextInt(16)));
    for (int i = 0; i < values.size(); ++i) {
      if (i == 0 || random()->nextInt(10) < 9) {
        values[i] = random()->nextLong();
      } else {
        values[i] = values[random()->nextInt(i)];
      }
    }
    if (values.size() > 0 && random()->nextBoolean()) {
      values[values.size() / 2] = numeric_limits<int64_t>::min();
    }
    shared_ptr<Set<int64_t>> set1 =
        LongStream::of(values)
            .mapToObj(optional<int64_t>::valueOf)
            .collect(Collectors::toCollection(unordered_set::new));
    shared_ptr<LongHashSet> set2 = make_shared<LongHashSet>(values);
    assertEquals(set1, set2);
  }
}
} // namespace org::apache::lucene::search