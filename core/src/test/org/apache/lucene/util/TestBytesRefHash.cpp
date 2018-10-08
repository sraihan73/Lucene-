using namespace std;

#include "TestBytesRefHash.h"

namespace org::apache::lucene::util
{
using MaxBytesLengthExceededException =
    org::apache::lucene::util::BytesRefHash::MaxBytesLengthExceededException;
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void TestBytesRefHash::setUp() 
{
  LuceneTestCase::setUp();
  pool = newPool();
  hash = newHash(pool);
}

shared_ptr<ByteBlockPool> TestBytesRefHash::newPool()
{
  return random()->nextBoolean() && pool != nullptr
             ? pool
             : make_shared<ByteBlockPool>(
                   make_shared<RecyclingByteBlockAllocator>(
                       ByteBlockPool::BYTE_BLOCK_SIZE, random()->nextInt(25)));
}

shared_ptr<BytesRefHash>
TestBytesRefHash::newHash(shared_ptr<ByteBlockPool> blockPool)
{
  constexpr int initSize = 2 << 1 + random()->nextInt(5);
  return random()->nextBoolean()
             ? make_shared<BytesRefHash>(blockPool)
             : make_shared<BytesRefHash>(
                   blockPool, initSize,
                   make_shared<BytesRefHash::DirectBytesStartArray>(initSize));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSize()
void TestBytesRefHash::testSize()
{
  shared_ptr<BytesRefBuilder> ref = make_shared<BytesRefBuilder>();
  int num = atLeast(2);
  for (int j = 0; j < num; j++) {
    constexpr int mod = 1 + random()->nextInt(39);
    for (int i = 0; i < 797; i++) {
      wstring str;
      do {
        str = TestUtil::randomRealisticUnicodeString(random(), 1000);
      } while (str.length() == 0);
      ref->copyChars(str);
      int count = hash->size();
      int key = hash->add(ref->get());
      if (key < 0) {
        assertEquals(hash->size(), count);
      } else {
        assertEquals(hash->size(), count + 1);
      }
      if (i % mod == 0) {
        hash->clear();
        assertEquals(0, hash->size());
        hash->reinit();
      }
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGet()
void TestBytesRefHash::testGet()
{
  shared_ptr<BytesRefBuilder> ref = make_shared<BytesRefBuilder>();
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  int num = atLeast(2);
  for (int j = 0; j < num; j++) {
    unordered_map<wstring, int> strings = unordered_map<wstring, int>();
    int uniqueCount = 0;
    for (int i = 0; i < 797; i++) {
      wstring str;
      do {
        str = TestUtil::randomRealisticUnicodeString(random(), 1000);
      } while (str.length() == 0);
      ref->copyChars(str);
      int count = hash->size();
      int key = hash->add(ref->get());
      if (key >= 0) {
        assertNull(strings.emplace(str, static_cast<Integer>(key)));
        assertEquals(uniqueCount, key);
        uniqueCount++;
        assertEquals(hash->size(), count + 1);
      } else {
        assertTrue((-key) - 1 < count);
        assertEquals(hash->size(), count);
      }
    }
    for (auto entry : strings) {
      ref->copyChars(entry.first);
      assertEquals(ref->get(), hash->get(entry.second::intValue(), scratch));
    }
    hash->clear();
    assertEquals(0, hash->size());
    hash->reinit();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCompact()
void TestBytesRefHash::testCompact()
{
  shared_ptr<BytesRefBuilder> ref = make_shared<BytesRefBuilder>();
  int num = atLeast(2);
  for (int j = 0; j < num; j++) {
    int numEntries = 0;
    constexpr int size = 797;
    shared_ptr<BitSet> bits = make_shared<BitSet>(size);
    for (int i = 0; i < size; i++) {
      wstring str;
      do {
        str = TestUtil::randomRealisticUnicodeString(random(), 1000);
      } while (str.length() == 0);
      ref->copyChars(str);
      constexpr int key = hash->add(ref->get());
      if (key < 0) {
        assertTrue(bits->get((-key) - 1));
      } else {
        assertFalse(bits->get(key));
        bits->set(key);
        numEntries++;
      }
    }
    assertEquals(hash->size(), bits->cardinality());
    assertEquals(numEntries, bits->cardinality());
    assertEquals(numEntries, hash->size());
    std::deque<int> compact = hash->compact();
    assertTrue(numEntries < compact.size());
    for (int i = 0; i < numEntries; i++) {
      bits->set(compact[i], false);
    }
    assertEquals(0, bits->cardinality());
    hash->clear();
    assertEquals(0, hash->size());
    hash->reinit();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSort()
void TestBytesRefHash::testSort()
{
  shared_ptr<BytesRefBuilder> ref = make_shared<BytesRefBuilder>();
  int num = atLeast(2);
  for (int j = 0; j < num; j++) {

    // Sorts by unicode code point order (is there a simple way, e.g. a
    // Collator?)
    shared_ptr<SortedSet<wstring>> strings =
        set<wstring>(TestUtil::STRING_CODEPOINT_COMPARATOR);
    for (int i = 0; i < 797; i++) {
      wstring str;
      do {
        str = TestUtil::randomRealisticUnicodeString(random(), 1000);
      } while (str.length() == 0);
      ref->copyChars(str);
      hash->add(ref->get());
      strings->add(str);
    }
    std::deque<int> sort = hash->sort();
    assertTrue(strings->size() < sort.size());
    int i = 0;
    shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
    for (auto string : strings) {
      ref->copyChars(string);
      assertEquals(ref->get(), hash->get(sort[i++], scratch));
    }
    hash->clear();
    assertEquals(0, hash->size());
    hash->reinit();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAdd()
void TestBytesRefHash::testAdd()
{
  shared_ptr<BytesRefBuilder> ref = make_shared<BytesRefBuilder>();
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  int num = atLeast(2);
  for (int j = 0; j < num; j++) {
    shared_ptr<Set<wstring>> strings = unordered_set<wstring>();
    int uniqueCount = 0;
    for (int i = 0; i < 797; i++) {
      wstring str;
      do {
        str = TestUtil::randomRealisticUnicodeString(random(), 1000);
      } while (str.length() == 0);
      ref->copyChars(str);
      int count = hash->size();
      int key = hash->add(ref->get());

      if (key >= 0) {
        assertTrue(strings->add(str));
        assertEquals(uniqueCount, key);
        assertEquals(hash->size(), count + 1);
        uniqueCount++;
      } else {
        assertFalse(strings->add(str));
        assertTrue((-key) - 1 < count);
        assertEquals(str, hash->get((-key) - 1, scratch).utf8ToString());
        assertEquals(count, hash->size());
      }
    }

    assertAllIn(strings, hash);
    hash->clear();
    assertEquals(0, hash->size());
    hash->reinit();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFind() throws Exception
void TestBytesRefHash::testFind() 
{
  shared_ptr<BytesRefBuilder> ref = make_shared<BytesRefBuilder>();
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  int num = atLeast(2);
  for (int j = 0; j < num; j++) {
    shared_ptr<Set<wstring>> strings = unordered_set<wstring>();
    int uniqueCount = 0;
    for (int i = 0; i < 797; i++) {
      wstring str;
      do {
        str = TestUtil::randomRealisticUnicodeString(random(), 1000);
      } while (str.length() == 0);
      ref->copyChars(str);
      int count = hash->size();
      int key = hash->find(ref->get()); // hash.add(ref);
      if (key >= 0) {                   // string found in hash
        assertFalse(strings->add(str));
        assertTrue(key < count);
        assertEquals(str, hash->get(key, scratch).utf8ToString());
        assertEquals(count, hash->size());
      } else {
        key = hash->add(ref->get());
        assertTrue(strings->add(str));
        assertEquals(uniqueCount, key);
        assertEquals(hash->size(), count + 1);
        uniqueCount++;
      }
    }

    assertAllIn(strings, hash);
    hash->clear();
    assertEquals(0, hash->size());
    hash->reinit();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected =
// org.apache.lucene.util.BytesRefHash.MaxBytesLengthExceededException.class)
// public void testLargeValue()
void TestBytesRefHash::testLargeValue()
{
  std::deque<int> sizes = {
      random()->nextInt(5),
      ByteBlockPool::BYTE_BLOCK_SIZE - 33 + random()->nextInt(31),
      ByteBlockPool::BYTE_BLOCK_SIZE - 1 + random()->nextInt(37)};
  shared_ptr<BytesRef> ref = make_shared<BytesRef>();
  for (int i = 0; i < sizes.size(); i++) {
    ref->bytes = std::deque<char>(sizes[i]);
    ref->offset = 0;
    ref->length = sizes[i];
    try {
      assertEquals(i, hash->add(ref));
    } catch (const MaxBytesLengthExceededException &e) {
      if (i < sizes.size() - 1) {
        fail(L"unexpected exception at size: " + to_wstring(sizes[i]));
      }
      throw e;
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAddByPoolOffset()
void TestBytesRefHash::testAddByPoolOffset()
{
  shared_ptr<BytesRefBuilder> ref = make_shared<BytesRefBuilder>();
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  shared_ptr<BytesRefHash> offsetHash = newHash(pool);
  int num = atLeast(2);
  for (int j = 0; j < num; j++) {
    shared_ptr<Set<wstring>> strings = unordered_set<wstring>();
    int uniqueCount = 0;
    for (int i = 0; i < 797; i++) {
      wstring str;
      do {
        str = TestUtil::randomRealisticUnicodeString(random(), 1000);
      } while (str.length() == 0);
      ref->copyChars(str);
      int count = hash->size();
      int key = hash->add(ref->get());

      if (key >= 0) {
        assertTrue(strings->add(str));
        assertEquals(uniqueCount, key);
        assertEquals(hash->size(), count + 1);
        int offsetKey = offsetHash->addByPoolOffset(hash->byteStart(key));
        assertEquals(uniqueCount, offsetKey);
        assertEquals(offsetHash->size(), count + 1);
        uniqueCount++;
      } else {
        assertFalse(strings->add(str));
        assertTrue((-key) - 1 < count);
        assertEquals(str, hash->get((-key) - 1, scratch).utf8ToString());
        assertEquals(count, hash->size());
        int offsetKey =
            offsetHash->addByPoolOffset(hash->byteStart((-key) - 1));
        assertTrue((-offsetKey) - 1 < count);
        assertEquals(str, hash->get((-offsetKey) - 1, scratch).utf8ToString());
        assertEquals(count, hash->size());
      }
    }

    assertAllIn(strings, hash);
    for (auto string : strings) {
      ref->copyChars(string);
      int key = hash->add(ref->get());
      shared_ptr<BytesRef> bytesRef = offsetHash->get((-key) - 1, scratch);
      assertEquals(ref->get(), bytesRef);
    }

    hash->clear();
    assertEquals(0, hash->size());
    offsetHash->clear();
    assertEquals(0, offsetHash->size());
    hash->reinit(); // init for the next round
    offsetHash->reinit();
  }
}

void TestBytesRefHash::assertAllIn(shared_ptr<Set<wstring>> strings,
                                   shared_ptr<BytesRefHash> hash)
{
  shared_ptr<BytesRefBuilder> ref = make_shared<BytesRefBuilder>();
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  int count = hash->size();
  for (auto string : strings) {
    ref->copyChars(string);
    int key = hash->add(ref->get()); // add again to check duplicates
    assertEquals(string, hash->get((-key) - 1, scratch).utf8ToString());
    assertEquals(count, hash->size());
    assertTrue(L"key: " + to_wstring(key) + L" count: " + to_wstring(count) +
                   L" string: " + string,
               key < count);
  }
}
} // namespace org::apache::lucene::util