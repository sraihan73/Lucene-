using namespace std;

#include "BaseSortTestCase.h"

namespace org::apache::lucene::util
{

BaseSortTestCase::Entry::Entry(int value, int ord) : value(value), ord(ord) {}

int BaseSortTestCase::Entry::compareTo(shared_ptr<Entry> other)
{
  return value < other->value ? -1 : value == other->value ? 0 : 1;
}

BaseSortTestCase::BaseSortTestCase(bool stable) : stable(stable) {}

void BaseSortTestCase::assertSorted(
    std::deque<std::shared_ptr<Entry>> &original,
    std::deque<std::shared_ptr<Entry>> &sorted)
{
  assertEquals(original.size(), sorted.size());
  std::deque<std::shared_ptr<Entry>> actuallySorted =
      Arrays::copyOf(original, original.size());
  Arrays::sort(actuallySorted);
  for (int i = 0; i < original.size(); ++i) {
    assertEquals(actuallySorted[i]->value, sorted[i]->value);
    if (stable) {
      assertEquals(actuallySorted[i]->ord, sorted[i]->ord);
    }
  }
}

void BaseSortTestCase::test(std::deque<std::shared_ptr<Entry>> &arr)
{
  constexpr int o = random()->nextInt(1000);
  std::deque<std::shared_ptr<Entry>> toSort(o + arr.size() +
                                             random()->nextInt(3));
  System::arraycopy(arr, 0, toSort, o, arr.size());
  shared_ptr<Sorter> *const sorter = newSorter(toSort);
  sorter->sort(o, o + arr.size());
  assertSorted(arr, Arrays::copyOfRange(toSort, o, o + arr.size()));
}

deque<Strategy> Strategy::valueList;

Strategy::StaticConstructor::StaticConstructor() {}

Strategy::StaticConstructor Strategy::staticConstructor;
int Strategy::nextOrdinal = 0;
Strategy::Strategy(const wstring &name, InnerEnum innerEnum)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
}

bool Strategy::operator==(const Strategy &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Strategy::operator!=(const Strategy &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Strategy> Strategy::values() { return valueList; }

int Strategy::ordinal() { return ordinalValue; }

wstring Strategy::toString() { return nameValue; }

Strategy Strategy::valueOf(const wstring &name)
{
  for (auto enumInstance : Strategy::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}

void BaseSortTestCase::test(Strategy strategy, int length)
{
  std::deque<std::shared_ptr<Entry>> arr(length);
  for (int i = 0; i < arr.size(); ++i) {
    strategy.set(arr, i);
  }
  test(arr);
}

void BaseSortTestCase::test(Strategy strategy)
{
  test(strategy, random()->nextInt(20000));
}

void BaseSortTestCase::testEmpty()
{
  test(std::deque<std::shared_ptr<Entry>>(0));
}

void BaseSortTestCase::testOne() { test(Strategy::RANDOM, 1); }

void BaseSortTestCase::testTwo() { test(Strategy::RANDOM_LOW_CARDINALITY, 2); }

void BaseSortTestCase::testRandom() { test(Strategy::RANDOM); }

void BaseSortTestCase::testRandomLowCardinality()
{
  test(Strategy::RANDOM_LOW_CARDINALITY);
}

void BaseSortTestCase::testAscending() { test(Strategy::ASCENDING); }

void BaseSortTestCase::testAscendingSequences()
{
  test(Strategy::ASCENDING_SEQUENCES);
}

void BaseSortTestCase::testDescending() { test(Strategy::DESCENDING); }

void BaseSortTestCase::testStrictlyDescending()
{
  test(Strategy::STRICTLY_DESCENDING);
}
} // namespace org::apache::lucene::util