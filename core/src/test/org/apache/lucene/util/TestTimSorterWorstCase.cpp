using namespace std;

#include "TestTimSorterWorstCase.h"

namespace org::apache::lucene::util
{
using org::apache::lucene::util::LuceneTestCase::Slow;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;

void TestTimSorterWorstCase::testWorstCaseStackSize()
{
  // we need large arrays to be able to reproduce this bug
  // but not so big we blow up available heap.
  constexpr int length;
  if (TEST_NIGHTLY) {
    length = RandomNumbers::randomIntBetween(random(), 140000000, 400000000);
  } else {
    length = RandomNumbers::randomIntBetween(random(), 140000000, 200000000);
  }
  shared_ptr<PackedInts::Mutable> *const arr = generateWorstCaseArray(length);
  make_shared<TimSorterAnonymousInnerClass>(shared_from_this(), arr)
      .sort(0, length);
}

TestTimSorterWorstCase::TimSorterAnonymousInnerClass::
    TimSorterAnonymousInnerClass(
        shared_ptr<TestTimSorterWorstCase> outerInstance,
        shared_ptr<PackedInts::Mutable> arr)
    : TimSorter(0)
{
  this->outerInstance = outerInstance;
  this->arr = arr;
}

void TestTimSorterWorstCase::TimSorterAnonymousInnerClass::swap(int i, int j)
{
  constexpr int64_t tmp = arr->get(i);
  arr->set(i, arr->get(j));
  arr->set(j, tmp);
}

int TestTimSorterWorstCase::TimSorterAnonymousInnerClass::compare(int i, int j)
{
  return Long::compare(arr->get(i), arr->get(j));
}

void TestTimSorterWorstCase::TimSorterAnonymousInnerClass::save(int i, int len)
{
  throw make_shared<UnsupportedOperationException>();
}

void TestTimSorterWorstCase::TimSorterAnonymousInnerClass::restore(int i, int j)
{
  throw make_shared<UnsupportedOperationException>();
}

void TestTimSorterWorstCase::TimSorterAnonymousInnerClass::copy(int src,
                                                                int dest)
{
  arr->set(dest, arr->get(src));
}

int TestTimSorterWorstCase::TimSorterAnonymousInnerClass::compareSaved(int i,
                                                                       int j)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<PackedInts::Mutable>
TestTimSorterWorstCase::createArray(int length, deque<int> &runs)
{
  shared_ptr<PackedInts::Mutable> array_ = PackedInts::getMutable(length, 1, 0);
  int endRun = -1;
  for (auto len : runs) {
    array_->set(endRun += len, 1);
  }
  array_->set(length - 1, 0);
  return array_;
}

shared_ptr<PackedInts::Mutable>
TestTimSorterWorstCase::generateWorstCaseArray(int length)
{
  constexpr int minRun = TimSorter::minRun(length);
  const deque<int> runs = runsWorstCase(length, minRun);
  return createArray(length, runs);
}

deque<int> TestTimSorterWorstCase::runsWorstCase(int length, int minRun)
{
  deque<int> runs = deque<int>();

  int runningTotal = 0, Y = minRun + 4, X = minRun;

  while (static_cast<int64_t>(runningTotal) + Y + X <= length) {
    runningTotal += X + Y;
    generateWrongElem(X, minRun, runs);
    runs.push_back(0, Y);

    // X_{i+1} = Y_i + x_{i,1} + 1, since runs.get(1) = x_{i,1}
    X = Y + runs[1] + 1;

    // Y_{i+1} = X_{i+1} + Y_i + 1
    Y += X + 1;
  }

  if (static_cast<int64_t>(runningTotal) + X <= length) {
    runningTotal += X;
    generateWrongElem(X, minRun, runs);
  }

  runs.push_back(length - runningTotal);
  return runs;
}

void TestTimSorterWorstCase::generateWrongElem(int X, int minRun,
                                               deque<int> &runs)
{
  for (int newTotal; X >= 2 * minRun + 1; X = newTotal) {
    // Default strategy
    newTotal = X / 2 + 1;

    // Specialized strategies
    if (3 * minRun + 3 <= X && X <= 4 * minRun + 1) {
      // add x_1=MIN+1, x_2=MIN, x_3=X-newTotal  to runs
      newTotal = 2 * minRun + 1;
    } else if (5 * minRun + 5 <= X && X <= 6 * minRun + 5) {
      // add x_1=MIN+1, x_2=MIN, x_3=MIN+2, x_4=X-newTotal  to runs
      newTotal = 3 * minRun + 3;
    } else if (8 * minRun + 9 <= X && X <= 10 * minRun + 9) {
      // add x_1=MIN+1, x_2=MIN, x_3=MIN+2, x_4=2MIN+2, x_5=X-newTotal  to runs
      newTotal = 5 * minRun + 5;
    } else if (13 * minRun + 15 <= X && X <= 16 * minRun + 17) {
      // add x_1=MIN+1, x_2=MIN, x_3=MIN+2, x_4=2MIN+2, x_5=3MIN+4,
      // x_6=X-newTotal  to runs
      newTotal = 8 * minRun + 9;
    }
    runs.push_back(0, X - newTotal);
  }
  runs.push_back(0, X);
}
} // namespace org::apache::lucene::util