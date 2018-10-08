using namespace std;

#include "TestBytesRefArray.h"

namespace org::apache::lucene::util
{
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using Counter = org::apache::lucene::util::Counter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestBytesRefArray::testAppend() 
{
  shared_ptr<Random> random = TestBytesRefArray::random();
  shared_ptr<BytesRefArray> deque =
      make_shared<BytesRefArray>(Counter::newCounter());
  deque<wstring> stringList = deque<wstring>();
  for (int j = 0; j < 2; j++) {
    if (j > 0 && random->nextBoolean()) {
      deque->clear();
      stringList.clear();
    }
    int entries = atLeast(500);
    shared_ptr<BytesRefBuilder> spare = make_shared<BytesRefBuilder>();
    int initSize = deque->size();
    for (int i = 0; i < entries; i++) {
      wstring randomRealisticUnicodeString =
          TestUtil::randomRealisticUnicodeString(random);
      spare->copyChars(randomRealisticUnicodeString);
      TestUtil::assertEquals(i + initSize, deque->append(spare->get()));
      stringList.push_back(randomRealisticUnicodeString);
    }
    for (int i = 0; i < entries; i++) {
      assertNotNull(deque->get(spare, i));
      assertEquals(L"entry " + to_wstring(i) + L" doesn't match", stringList[i],
                   spare->get().utf8ToString());
    }

    // check random
    for (int i = 0; i < entries; i++) {
      int e = random->nextInt(entries);
      assertNotNull(deque->get(spare, e));
      assertEquals(L"entry " + to_wstring(i) + L" doesn't match", stringList[e],
                   spare->get().utf8ToString());
    }
    for (int i = 0; i < 2; i++) {

      shared_ptr<BytesRefIterator> iterator = deque->begin();
      for (auto string : stringList) {
        TestUtil::assertEquals(string, iterator->next()->utf8ToString());
      }
    }
  }
}

void TestBytesRefArray::testSort() 
{
  shared_ptr<Random> random = TestBytesRefArray::random();
  shared_ptr<BytesRefArray> deque =
      make_shared<BytesRefArray>(Counter::newCounter());
  deque<wstring> stringList = deque<wstring>();

  for (int j = 0; j < 2; j++) {
    if (j > 0 && random->nextBoolean()) {
      deque->clear();
      stringList.clear();
    }
    int entries = atLeast(500);
    shared_ptr<BytesRefBuilder> spare = make_shared<BytesRefBuilder>();
    constexpr int initSize = deque->size();
    for (int i = 0; i < entries; i++) {
      wstring randomRealisticUnicodeString =
          TestUtil::randomRealisticUnicodeString(random);
      spare->copyChars(randomRealisticUnicodeString);
      TestUtil::assertEquals(initSize + i, deque->append(spare->get()));
      stringList.push_back(randomRealisticUnicodeString);
    }

    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: Collections.sort(stringList,
    // org.apache.lucene.util.TestUtil.STRING_CODEPOINT_COMPARATOR);
    sort(stringList.begin(), stringList.end(),
         TestUtil::STRING_CODEPOINT_COMPARATOR);
    shared_ptr<BytesRefIterator> iter =
        deque->iterator(Comparator::naturalOrder());
    int i = 0;
    shared_ptr<BytesRef> next;
    while ((next = iter->next()) != nullptr) {
      assertEquals(L"entry " + to_wstring(i) + L" doesn't match", stringList[i],
                   next->utf8ToString());
      i++;
    }
    assertNull(iter->next());
    TestUtil::assertEquals(i, stringList.size());
  }
}
} // namespace org::apache::lucene::util