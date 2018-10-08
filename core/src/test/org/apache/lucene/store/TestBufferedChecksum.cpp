using namespace std;

#include "TestBufferedChecksum.h"

namespace org::apache::lucene::store
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBufferedChecksum::testSimple()
{
  shared_ptr<Checksum> c = make_shared<BufferedChecksum>(make_shared<CRC32>());
  c->update(1);
  c->update(2);
  c->update(3);
  assertEquals(1438416925LL, c->getValue());
}

void TestBufferedChecksum::testRandom()
{
  shared_ptr<Checksum> c1 = make_shared<CRC32>();
  shared_ptr<Checksum> c2 = make_shared<BufferedChecksum>(make_shared<CRC32>());
  int iterations = atLeast(10000);
  for (int i = 0; i < iterations; i++) {
    switch (random()->nextInt(4)) {
    case 0: {
      // update(byte[], int, int)
      int length = random()->nextInt(1024);
      std::deque<char> bytes(length);
      random()->nextBytes(bytes);
      c1->update(bytes, 0, bytes.size());
      c2->update(bytes, 0, bytes.size());
      break;
    }
    case 1: {
      // update(int)
      int b = random()->nextInt(256);
      c1->update(b);
      c2->update(b);
      break;
    }
    case 2:
      // reset()
      c1->reset();
      c2->reset();
      break;
    case 3:
      // getValue()
      assertEquals(c1->getValue(), c2->getValue());
      break;
    }
  }
  assertEquals(c1->getValue(), c2->getValue());
}
} // namespace org::apache::lucene::store