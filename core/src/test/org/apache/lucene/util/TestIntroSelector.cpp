using namespace std;

#include "TestIntroSelector.h"

namespace org::apache::lucene::util
{

void TestIntroSelector::testSelect()
{
  for (int iter = 0; iter < 100; ++iter) {
    doTestSelect(false);
  }
}

void TestIntroSelector::testSlowSelect()
{
  for (int iter = 0; iter < 100; ++iter) {
    doTestSelect(true);
  }
}

void TestIntroSelector::doTestSelect(bool slow)
{
  constexpr int from = random()->nextInt(5);
  constexpr int to = from + TestUtil::nextInt(random(), 1, 10000);
  constexpr int max = random()->nextBoolean() ? random()->nextInt(100)
                                              : random()->nextInt(100000);
  std::deque<optional<int>> arr(from + to + random()->nextInt(5));
  for (int i = 0; i < arr.size(); ++i) {
    arr[i] = TestUtil::nextInt(random(), 0, max);
  }
  constexpr int k = TestUtil::nextInt(random(), from, to - 1);

  std::deque<optional<int>> expected = arr.clone();
  Arrays::sort(expected, from, to);

  std::deque<optional<int>> actual = arr.clone();
  shared_ptr<IntroSelector> selector =
      make_shared<IntroSelectorAnonymousInnerClass>(shared_from_this(), actual);
  if (slow) {
    selector->slowSelect(from, to, k);
  } else {
    selector->select(from, to, k);
  }

  assertEquals(expected[k], actual[k]);
  for (int i = 0; i < actual.size(); ++i) {
    if (i < from || i >= to) {
      assertSame(arr[i], actual[i]);
    } else if (i <= k) {
      assertTrue(actual[i] <= actual[k]);
    } else {
      assertTrue(actual[i] >= actual[k]);
    }
  }
}

TestIntroSelector::IntroSelectorAnonymousInnerClass::
    IntroSelectorAnonymousInnerClass(
        shared_ptr<TestIntroSelector> outerInstance,
        deque<optional<int>> &actual)
{
  this->outerInstance = outerInstance;
  this->actual = actual;
}

void TestIntroSelector::IntroSelectorAnonymousInnerClass::swap(int i, int j)
{
  ArrayUtil::swap(actual, i, j);
}

void TestIntroSelector::IntroSelectorAnonymousInnerClass::setPivot(int i)
{
  pivot = actual[i];
}

int TestIntroSelector::IntroSelectorAnonymousInnerClass::comparePivot(int j)
{
  return pivot->compareTo(actual[j]);
}
} // namespace org::apache::lucene::util