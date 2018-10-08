using namespace std;

#include "TestByteBlockPool.h"

namespace org::apache::lucene::util
{

void TestByteBlockPool::testReadAndWrite() 
{
  shared_ptr<Counter> bytesUsed = Counter::newCounter();
  shared_ptr<ByteBlockPool> pool = make_shared<ByteBlockPool>(
      make_shared<ByteBlockPool::DirectTrackingAllocator>(bytesUsed));
  pool->nextBuffer();
  bool reuseFirst = random()->nextBoolean();
  for (int j = 0; j < 2; j++) {

    deque<std::shared_ptr<BytesRef>> deque =
        deque<std::shared_ptr<BytesRef>>();
    int maxLength = atLeast(500);
    constexpr int numValues = atLeast(100);
    shared_ptr<BytesRefBuilder> ref = make_shared<BytesRefBuilder>();
    for (int i = 0; i < numValues; i++) {
      const wstring value =
          TestUtil::randomRealisticUnicodeString(random(), maxLength);
      deque.push_back(make_shared<BytesRef>(value));
      ref->copyChars(value);
      pool->append(ref->get());
    }
    // verify
    int64_t position = 0;
    for (auto expected : deque) {
      ref->grow(expected->length);
      ref->setLength(expected->length);
      switch (random()->nextInt(3)) {
      case 0:
        // copy bytes
        pool->readBytes(position, ref->bytes(), 0, ref->length());
        break;
      case 1:
        // copy bytes one by one
        for (int i = 0; i < ref->length(); ++i) {
          ref->setByteAt(i, pool->readByte(position + i));
        }
        break;
      case 2: {
        shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
        scratch->length = ref->length();
        pool->setRawBytesRef(scratch, position);
        System::arraycopy(scratch->bytes, scratch->offset, ref->bytes(), 0,
                          ref->length());
        break;
      }
      default:
        fail();
      }
      assertEquals(expected, ref->get());
      position += ref->length();
    }
    pool->reset(random()->nextBoolean(), reuseFirst);
    if (reuseFirst) {
      assertEquals(ByteBlockPool::BYTE_BLOCK_SIZE, bytesUsed->get());
    } else {
      assertEquals(0, bytesUsed->get());
      pool->nextBuffer(); // prepare for next iter
    }
  }
}

void TestByteBlockPool::testLargeRandomBlocks() 
{
  shared_ptr<Counter> bytesUsed = Counter::newCounter();
  shared_ptr<ByteBlockPool> pool = make_shared<ByteBlockPool>(
      make_shared<ByteBlockPool::DirectTrackingAllocator>(bytesUsed));
  pool->nextBuffer();

  deque<std::deque<char>> items = deque<std::deque<char>>();
  for (int i = 0; i < 100; i++) {
    int size;
    if (random()->nextBoolean()) {
      size = TestUtil::nextInt(random(), 100, 1000);
    } else {
      size = TestUtil::nextInt(random(), 50000, 100000);
    }
    std::deque<char> bytes(size);
    random()->nextBytes(bytes);
    items.push_back(bytes);
    pool->append(make_shared<BytesRef>(bytes));
  }

  int64_t position = 0;
  for (auto expected : items) {
    std::deque<char> actual(expected.size());
    pool->readBytes(position, actual, 0, actual.size());
    assertTrue(Arrays::equals(expected, actual));
    position += expected.size();
  }
}
} // namespace org::apache::lucene::util