using namespace std;

#include "TestMergedIterator.h"

namespace org::apache::lucene::util
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"rawtypes", "unchecked"}) public void
// testMergeEmpty()
void TestMergedIterator::testMergeEmpty()
{
  Iterator<int> merged = make_shared<MergedIterator<int>>();
  assertFalse(merged->hasNext());

  merged = make_shared<MergedIterator<>>((deque<int>())->begin());
  assertFalse(merged->hasNext());

  std::deque<Iterator<int>> itrs =
      std::deque<std::shared_ptr<Iterator>>(random()->nextInt(100));
  for (int i = 0; i < itrs.size(); i++) {
    itrs[i] = (deque<int>())->begin();
  }
  merged = make_shared<MergedIterator<>>(itrs);
  assertFalse(merged->hasNext());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testNoDupsRemoveDups()
void TestMergedIterator::testNoDupsRemoveDups() { testCase(1, 1, true); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testOffItrDupsRemoveDups()
void TestMergedIterator::testOffItrDupsRemoveDups() { testCase(3, 1, true); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testOnItrDupsRemoveDups()
void TestMergedIterator::testOnItrDupsRemoveDups() { testCase(1, 3, true); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testOnItrRandomDupsRemoveDups()
void TestMergedIterator::testOnItrRandomDupsRemoveDups()
{
  testCase(1, -3, true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testBothDupsRemoveDups()
void TestMergedIterator::testBothDupsRemoveDups() { testCase(3, 3, true); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testBothDupsWithRandomDupsRemoveDups()
void TestMergedIterator::testBothDupsWithRandomDupsRemoveDups()
{
  testCase(3, -3, true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void testNoDupsKeepDups()
void TestMergedIterator::testNoDupsKeepDups() { testCase(1, 1, false); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testOffItrDupsKeepDups()
void TestMergedIterator::testOffItrDupsKeepDups() { testCase(3, 1, false); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testOnItrDupsKeepDups()
void TestMergedIterator::testOnItrDupsKeepDups() { testCase(1, 3, false); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testOnItrRandomDupsKeepDups()
void TestMergedIterator::testOnItrRandomDupsKeepDups()
{
  testCase(1, -3, false);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testBothDupsKeepDups()
void TestMergedIterator::testBothDupsKeepDups() { testCase(3, 3, false); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
// testBothDupsWithRandomDupsKeepDups()
void TestMergedIterator::testBothDupsWithRandomDupsKeepDups()
{
  testCase(3, -3, false);
}

void TestMergedIterator::testCase(int itrsWithVal, int specifiedValsOnItr,
                                  bool removeDups)
{
  // Build a random number of lists
  deque<int> expected = deque<int>();
  shared_ptr<Random> random =
      make_shared<Random>(TestMergedIterator::random()->nextLong());
  int numLists = itrsWithVal + random->nextInt(1000 - itrsWithVal);
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes", "unchecked"})
  // java.util.List<int>[] lists = new java.util.List[numLists];
  std::deque<deque<int>> lists = std::deque<deque>(numLists);
  for (int i = 0; i < numLists; i++) {
    lists[i] = deque<>();
  }
  int start = random->nextInt(1000000);
  int end = start + VALS_TO_MERGE / itrsWithVal / abs(specifiedValsOnItr);
  for (int i = start; i < end; i++) {
    int maxList = lists.size();
    int maxValsOnItr = 0;
    int sumValsOnItr = 0;
    for (int itrWithVal = 0; itrWithVal < itrsWithVal; itrWithVal++) {
      int deque = random->nextInt(maxList);
      int valsOnItr = specifiedValsOnItr < 0
                          ? (1 + random->nextInt(-specifiedValsOnItr))
                          : specifiedValsOnItr;
      maxValsOnItr = max(maxValsOnItr, valsOnItr);
      sumValsOnItr += valsOnItr;
      for (int valOnItr = 0; valOnItr < valsOnItr; valOnItr++) {
        lists[deque].push_back(i);
      }
      maxList = maxList - 1;
      ArrayUtil::swap(lists, deque, maxList);
    }
    int maxCount = removeDups ? maxValsOnItr : sumValsOnItr;
    for (int count = 0; count < maxCount; count++) {
      expected.push_back(i);
    }
  }
  // Now check that they get merged cleanly
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes", "unchecked"})
  // java.util.Iterator<int>[] itrs = new java.util.Iterator[numLists];
  std::deque<Iterator<int>> itrs =
      std::deque<std::shared_ptr<Iterator>>(numLists);
  for (int i = 0; i < numLists; i++) {
    itrs[i] = lists[i].begin();
  }

  shared_ptr<MergedIterator<int>> mergedItr =
      make_shared<MergedIterator<int>>(removeDups, itrs);
  deque<int>::const_iterator expectedItr = expected.begin();
  while (expectedItr != expected.end()) {
    assertTrue(mergedItr->hasNext());
    assertEquals(*expectedItr, mergedItr->next());
    expectedItr++;
  }
  assertFalse(mergedItr->hasNext());
}
} // namespace org::apache::lucene::util