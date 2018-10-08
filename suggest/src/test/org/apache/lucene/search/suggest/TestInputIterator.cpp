using namespace std;

#include "TestInputIterator.h"

namespace org::apache::lucene::search::suggest
{
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestInputIterator::testEmpty() 
{
  shared_ptr<InputArrayIterator> iterator =
      make_shared<InputArrayIterator>(std::deque<std::shared_ptr<Input>>(0));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // getDirectory())
  {
    org::apache::lucene::store::Directory dir = getDirectory();
    shared_ptr<InputIterator> wrapper =
        make_shared<SortedInputIterator>(dir, L"sorted", iterator);
    assertNull(wrapper->next());
    wrapper = make_shared<UnsortedInputIterator>(iterator);
    assertNull(wrapper->next());
  }
}

void TestInputIterator::testTerms() 
{
  shared_ptr<Random> random = TestInputIterator::random();
  int num = atLeast(10000);

  map_obj<std::shared_ptr<BytesRef>,
      SimpleEntry<int64_t, std::shared_ptr<BytesRef>>>
      sorted = map_obj<std::shared_ptr<BytesRef>,
                   SimpleEntry<int64_t, std::shared_ptr<BytesRef>>>();
  map_obj<std::shared_ptr<BytesRef>, int64_t> sortedWithoutPayload =
      map_obj<std::shared_ptr<BytesRef>, int64_t>();
  map_obj<std::shared_ptr<BytesRef>,
      SimpleEntry<int64_t, Set<std::shared_ptr<BytesRef>>>>
      sortedWithContext =
          map_obj<std::shared_ptr<BytesRef>,
              SimpleEntry<int64_t, Set<std::shared_ptr<BytesRef>>>>();
  map_obj<std::shared_ptr<BytesRef>,
      SimpleEntry<int64_t, SimpleEntry<std::shared_ptr<BytesRef>,
                                         Set<std::shared_ptr<BytesRef>>>>>
      sortedWithPayloadAndContext =
          map_obj<std::shared_ptr<BytesRef>,
              SimpleEntry<int64_t,
                          SimpleEntry<std::shared_ptr<BytesRef>,
                                      Set<std::shared_ptr<BytesRef>>>>>();
  std::deque<std::shared_ptr<Input>> unsorted(num);
  std::deque<std::shared_ptr<Input>> unsortedWithoutPayload(num);
  std::deque<std::shared_ptr<Input>> unsortedWithContexts(num);
  std::deque<std::shared_ptr<Input>> unsortedWithPayloadAndContext(num);
  shared_ptr<Set<std::shared_ptr<BytesRef>>> ctxs;
  for (int i = 0; i < num; i++) {
    shared_ptr<BytesRef> key;
    shared_ptr<BytesRef> payload;
    ctxs = unordered_set<>();
    do {
      key = make_shared<BytesRef>(TestUtil::randomUnicodeString(random));
      payload = make_shared<BytesRef>(TestUtil::randomUnicodeString(random));
      for (int j = 0; j < atLeast(2); j++) {
        ctxs->add(make_shared<BytesRef>(TestUtil::randomUnicodeString(random)));
      }
    } while (sorted.find(key) != sorted.end());
    int64_t value = random->nextLong();
    sortedWithoutPayload.emplace(key, value);
    sorted.emplace(key, make_shared<SimpleEntry<>>(value, payload));
    sortedWithContext.emplace(key, make_shared<SimpleEntry<>>(value, ctxs));
    sortedWithPayloadAndContext.emplace(
        key, make_shared<SimpleEntry<>>(
                 value, make_shared<SimpleEntry<>>(payload, ctxs)));
    unsorted[i] = make_shared<Input>(key, value, payload);
    unsortedWithoutPayload[i] = make_shared<Input>(key, value);
    unsortedWithContexts[i] = make_shared<Input>(key, value, ctxs);
    unsortedWithPayloadAndContext[i] =
        make_shared<Input>(key, value, payload, ctxs);
  }

  // test the sorted iterator wrapper with payloads
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory tempDir =
  // getDirectory())
  {
    org::apache::lucene::store::Directory tempDir = getDirectory();
    shared_ptr<InputIterator> wrapper = make_shared<SortedInputIterator>(
        tempDir, L"sorted", make_shared<InputArrayIterator>(unsorted));
    map_obj<std::shared_ptr<BytesRef>,
        SimpleEntry<int64_t, std::shared_ptr<BytesRef>>>::const_iterator
        expected = sorted.begin();
    while (expected != sorted.end()) {
      unordered_map::Entry<std::shared_ptr<BytesRef>,
                           SimpleEntry<int64_t, std::shared_ptr<BytesRef>>>
          entry = *expected;

      TestUtil::assertEquals(entry.getKey(), wrapper->next());
      TestUtil::assertEquals(entry.getValue().getKey().longValue(),
                             wrapper->weight());
      TestUtil::assertEquals(entry.getValue().getValue(), wrapper->payload());
      expected++;
    }
    assertNull(wrapper->next());
  }

  // test the sorted iterator wrapper with contexts
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory tempDir =
  // getDirectory())
  {
    org::apache::lucene::store::Directory tempDir = getDirectory();
    shared_ptr<InputIterator> wrapper = make_shared<SortedInputIterator>(
        tempDir, L"sorted",
        make_shared<InputArrayIterator>(unsortedWithContexts));
    map_obj<std::shared_ptr<BytesRef>,
        SimpleEntry<int64_t, Set<std::shared_ptr<BytesRef>>>>::const_iterator
        actualEntries = sortedWithContext.begin();
    while (actualEntries != sortedWithContext.end()) {
      unordered_map::Entry<
          std::shared_ptr<BytesRef>,
          SimpleEntry<int64_t, Set<std::shared_ptr<BytesRef>>>>
          entry = *actualEntries;
      TestUtil::assertEquals(entry.getKey(), wrapper->next());
      TestUtil::assertEquals(entry.getValue().getKey().longValue(),
                             wrapper->weight());
      shared_ptr<Set<std::shared_ptr<BytesRef>>> actualCtxs =
          entry.getValue().getValue();
      TestUtil::assertEquals(actualCtxs, wrapper->contexts());
      actualEntries++;
    }
    assertNull(wrapper->next());
  }

  // test the sorted iterator wrapper with contexts and payload
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory tempDir =
  // getDirectory())
  {
    org::apache::lucene::store::Directory tempDir = getDirectory();
    shared_ptr<InputIterator> wrapper = make_shared<SortedInputIterator>(
        tempDir, L"sorter",
        make_shared<InputArrayIterator>(unsortedWithPayloadAndContext));
    map_obj<std::shared_ptr<BytesRef>,
        SimpleEntry<int64_t, SimpleEntry<std::shared_ptr<BytesRef>,
                                           Set<std::shared_ptr<BytesRef>>>>>::
        const_iterator expectedPayloadContextEntries =
            sortedWithPayloadAndContext.begin();
    while (expectedPayloadContextEntries != sortedWithPayloadAndContext.end()) {
      unordered_map::Entry<
          std::shared_ptr<BytesRef>,
          SimpleEntry<int64_t, SimpleEntry<std::shared_ptr<BytesRef>,
                                             Set<std::shared_ptr<BytesRef>>>>>
          entry = *expectedPayloadContextEntries;
      TestUtil::assertEquals(entry.getKey(), wrapper->next());
      TestUtil::assertEquals(entry.getValue().getKey().longValue(),
                             wrapper->weight());
      shared_ptr<Set<std::shared_ptr<BytesRef>>> actualCtxs =
          entry.getValue().getValue().getValue();
      TestUtil::assertEquals(actualCtxs, wrapper->contexts());
      shared_ptr<BytesRef> actualPayload = entry.getValue().getValue().getKey();
      TestUtil::assertEquals(actualPayload, wrapper->payload());
      expectedPayloadContextEntries++;
    }
    assertNull(wrapper->next());
  }

  // test the unsorted iterator wrapper with payloads
  shared_ptr<InputIterator> wrapper = make_shared<UnsortedInputIterator>(
      make_shared<InputArrayIterator>(unsorted));
  map_obj<std::shared_ptr<BytesRef>,
      SimpleEntry<int64_t, std::shared_ptr<BytesRef>>>
      actual = map_obj<std::shared_ptr<BytesRef>,
                   SimpleEntry<int64_t, std::shared_ptr<BytesRef>>>();
  shared_ptr<BytesRef> key;
  while ((key = wrapper->next()) != nullptr) {
    int64_t value = wrapper->weight();
    shared_ptr<BytesRef> payload = wrapper->payload();
    actual.emplace(
        BytesRef::deepCopyOf(key),
        make_shared<SimpleEntry<>>(value, BytesRef::deepCopyOf(payload)));
  }
  TestUtil::assertEquals(sorted, actual);

  // test the sorted iterator wrapper without payloads
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory tempDir =
  // getDirectory())
  {
    org::apache::lucene::store::Directory tempDir = getDirectory();
    shared_ptr<InputIterator> wrapperWithoutPayload =
        make_shared<SortedInputIterator>(
            tempDir, L"sorted",
            make_shared<InputArrayIterator>(unsortedWithoutPayload));
    map_obj<std::shared_ptr<BytesRef>, int64_t>::const_iterator
        expectedWithoutPayload = sortedWithoutPayload.begin();
    while (expectedWithoutPayload != sortedWithoutPayload.end()) {
      unordered_map::Entry<std::shared_ptr<BytesRef>, int64_t> entry =
          *expectedWithoutPayload;

      TestUtil::assertEquals(entry.getKey(), wrapperWithoutPayload->next());
      TestUtil::assertEquals(entry.getValue().longValue(),
                             wrapperWithoutPayload->weight());
      assertNull(wrapperWithoutPayload->payload());
      expectedWithoutPayload++;
    }
    assertNull(wrapperWithoutPayload->next());
  }

  // test the unsorted iterator wrapper without payloads
  shared_ptr<InputIterator> wrapperWithoutPayload =
      make_shared<UnsortedInputIterator>(
          make_shared<InputArrayIterator>(unsortedWithoutPayload));
  map_obj<std::shared_ptr<BytesRef>, int64_t> actualWithoutPayload =
      map_obj<std::shared_ptr<BytesRef>, int64_t>();
  while ((key = wrapperWithoutPayload->next()) != nullptr) {
    int64_t value = wrapperWithoutPayload->weight();
    assertNull(wrapperWithoutPayload->payload());
    actualWithoutPayload.emplace(BytesRef::deepCopyOf(key), value);
  }
  TestUtil::assertEquals(sortedWithoutPayload, actualWithoutPayload);
}

int64_t TestInputIterator::asLong(shared_ptr<BytesRef> b)
{
  return ((static_cast<int64_t>(asIntInternal(b, b->offset)) << 32) |
          asIntInternal(b, b->offset + 4) & 0xFFFFFFFFLL);
}

int TestInputIterator::asIntInternal(shared_ptr<BytesRef> b, int pos)
{
  return ((b->bytes[pos++] & 0xFF) << 24) | ((b->bytes[pos++] & 0xFF) << 16) |
         ((b->bytes[pos++] & 0xFF) << 8) | (b->bytes[pos] & 0xFF);
}

shared_ptr<Directory> TestInputIterator::getDirectory()
{
  return newDirectory();
}
} // namespace org::apache::lucene::search::suggest