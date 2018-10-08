using namespace std;

#include "TestFrequencyTrackingRingBuffer.h"

namespace org::apache::lucene::util
{

void TestFrequencyTrackingRingBuffer::assertBuffer(
    shared_ptr<FrequencyTrackingRingBuffer> buffer, int maxSize, int sentinel,
    deque<int> &items)
{
  const deque<int> recentItems;
  if (items.size() <= maxSize) {
    recentItems = deque<>();
    for (int i = items.size(); i < maxSize; ++i) {
      recentItems.push_back(sentinel);
    }
    recentItems.insert(recentItems.end(), items.begin(), items.end());
  } else {
    recentItems = items.subList(items.size() - maxSize, items.size());
  }
  const unordered_map<int, int> expectedFrequencies = unordered_map<int, int>();
  for (shared_ptr<> : : optional<int> item : recentItems) {
    const optional<int> freq = expectedFrequencies[item];
    if (!freq) {
      expectedFrequencies.emplace(item, 1);
    } else {
      expectedFrequencies.emplace(item, freq + 1);
    }
  }
  assertEquals(expectedFrequencies, buffer->asFrequencyMap());
}

void TestFrequencyTrackingRingBuffer::test()
{
  constexpr int iterations = atLeast(100);
  for (int i = 0; i < iterations; ++i) {
    constexpr int maxSize = 2 + random()->nextInt(100);
    constexpr int numitems = random()->nextInt(5000);
    constexpr int maxitem = 1 + random()->nextInt(100);
    deque<int> items = deque<int>();
    constexpr int sentinel = random()->nextInt(200);
    shared_ptr<FrequencyTrackingRingBuffer> buffer =
        make_shared<FrequencyTrackingRingBuffer>(maxSize, sentinel);
    for (int j = 0; j < numitems; ++j) {
      const optional<int> item = random()->nextInt(maxitem);
      items.push_back(item);
      buffer->add(item);
    }
    assertBuffer(buffer, maxSize, sentinel, items);
  }
}

void TestFrequencyTrackingRingBuffer::testRamBytesUsed()
{
  constexpr int maxSize = 2 + random()->nextInt(10000);
  constexpr int sentinel = random()->nextInt();
  shared_ptr<FrequencyTrackingRingBuffer> buffer =
      make_shared<FrequencyTrackingRingBuffer>(maxSize, sentinel);
  for (int i = 0; i < 10000; ++i) {
    buffer->add(random()->nextInt());
  }
  assertEquals(RamUsageTester::sizeOf(buffer), buffer->ramBytesUsed());
}
} // namespace org::apache::lucene::util