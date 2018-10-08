using namespace std;

#include "TestByteSlices.h"

namespace org::apache::lucene::index
{
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using RecyclingByteBlockAllocator =
    org::apache::lucene::util::RecyclingByteBlockAllocator;

void TestByteSlices::testBasic() 
{
  shared_ptr<ByteBlockPool> pool =
      make_shared<ByteBlockPool>(make_shared<RecyclingByteBlockAllocator>(
          ByteBlockPool::BYTE_BLOCK_SIZE, random()->nextInt(100)));

  constexpr int NUM_STREAM = atLeast(100);

  shared_ptr<ByteSliceWriter> writer = make_shared<ByteSliceWriter>(pool);

  std::deque<int> starts(NUM_STREAM);
  std::deque<int> uptos(NUM_STREAM);
  std::deque<int> counters(NUM_STREAM);

  shared_ptr<ByteSliceReader> reader = make_shared<ByteSliceReader>();

  for (int ti = 0; ti < 100; ti++) {

    for (int stream = 0; stream < NUM_STREAM; stream++) {
      starts[stream] = -1;
      counters[stream] = 0;
    }

    int num = atLeast(3000);
    for (int iter = 0; iter < num; iter++) {
      int stream;
      if (random()->nextBoolean()) {
        stream = random()->nextInt(3);
      } else {
        stream = random()->nextInt(NUM_STREAM);
      }

      if (VERBOSE) {
        wcout << L"write stream=" << stream << endl;
      }

      if (starts[stream] == -1) {
        constexpr int spot = pool->newSlice(ByteBlockPool::FIRST_LEVEL_SIZE);
        starts[stream] = uptos[stream] = spot + pool->byteOffset;
        if (VERBOSE) {
          wcout << L"  init to " << starts[stream] << endl;
        }
      }

      writer->init(uptos[stream]);
      int numValue;
      if (random()->nextInt(10) == 3) {
        numValue = random()->nextInt(100);
      } else if (random()->nextInt(5) == 3) {
        numValue = random()->nextInt(3);
      } else {
        numValue = random()->nextInt(20);
      }

      for (int j = 0; j < numValue; j++) {
        if (VERBOSE) {
          wcout << L"    write " << (counters[stream] << j) << endl;
        }
        // write some large (incl. negative) ints:
        writer->writeVInt(random()->nextInt());
        writer->writeVInt(counters[stream] + j);
      }
      counters[stream] += numValue;
      uptos[stream] = writer->getAddress();
      if (VERBOSE) {
        wcout << L"    addr now " << uptos[stream] << endl;
      }
    }

    for (int stream = 0; stream < NUM_STREAM; stream++) {
      if (VERBOSE) {
        wcout << L"  stream=" << stream << L" count=" << counters[stream]
              << endl;
      }

      if (starts[stream] != -1 && starts[stream] != uptos[stream]) {
        reader->init(pool, starts[stream], uptos[stream]);
        for (int j = 0; j < counters[stream]; j++) {
          reader->readVInt();
          assertEquals(j, reader->readVInt());
        }
      }
    }

    pool->reset();
  }
}
} // namespace org::apache::lucene::index